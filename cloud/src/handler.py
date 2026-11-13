"""
M3 ingest Lambda — validate batch, persist to DynamoDB, rule-based alert logs.

Contract: docs/contracts/telemetry-batch-http.md
ADR: docs/decisions/0003-cloud-floor.md, docs/decisions/0004-snapshot-sort-key.md
"""

from __future__ import annotations

import json
import logging
import os
import re
import time
import uuid
from decimal import Decimal
from typing import Any

import boto3

LOG = logging.getLogger()
LOG.setLevel(logging.INFO)

_UTC_TIMESTAMP = re.compile(r"^\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}\.\d{9}Z$")
_WELD_STATUSES = {"RUNNING", "FAULTED"}
_ALARM_SEVERITIES = {"info", "warning", "error"}
_PROCESS_FIELDS = (
    "weld_current_a",
    "weld_voltage_v",
    "wire_feed_m_min",
    "arc_on_s",
    "gas_flow_l_min",
)

_dynamodb = None


def _table():
    global _dynamodb
    if _dynamodb is None:
        _dynamodb = boto3.resource("dynamodb")
    name = os.environ["SAMPLES_TABLE"]
    return _dynamodb.Table(name)


def _expected_api_key() -> str:
    return os.environ.get("INGEST_API_KEY", "")


def _temp_threshold() -> float | None:
    raw = os.environ.get("TEMP_C_THRESHOLD", "").strip()
    if not raw:
        return None
    try:
        return float(raw)
    except ValueError:
        LOG.warning("invalid TEMP_C_THRESHOLD=%r ignored", raw)
        return None


def _response(status: int, body: dict[str, Any]) -> dict[str, Any]:
    return {
        "statusCode": status,
        "headers": {"Content-Type": "application/json"},
        "body": json.dumps(body),
    }


def _header(headers: dict[str, str] | None, name: str) -> str | None:
    if not headers:
        return None
    lower = {k.lower(): v for k, v in headers.items()}
    return lower.get(name.lower())


def validate_and_normalize(payload: Any) -> tuple[str, list[dict[str, Any]]] | tuple[None, str]:
    """Return (cell_id, samples) or (None, error_message)."""
    if not isinstance(payload, dict):
        return None, "body must be a JSON object"

    schema_version = payload.get("schema_version")
    if schema_version != 1:
        return None, "schema_version must be 1"

    cell_id = payload.get("cell_id")
    if not isinstance(cell_id, str) or not cell_id.strip():
        return None, "cell_id must be a non-empty string"

    samples = payload.get("samples")
    if not isinstance(samples, list) or len(samples) == 0:
        return None, "samples must be a non-empty array"

    normalized: list[dict[str, Any]] = []
    for i, sample in enumerate(samples):
        if not isinstance(sample, dict):
            return None, f"samples[{i}] must be an object"
        try:
            stamp_sec = int(sample["stamp_sec"])
            stamp_nanosec = int(sample["stamp_nanosec"])
            cycle = int(sample["cycle"])
            temp_c = float(sample["temp_c"])
            vibration = float(sample["vibration"])
            fault = bool(sample["fault"])
        except (KeyError, TypeError, ValueError) as exc:
            return None, f"samples[{i}] invalid fields: {exc}"

        normalized.append(
            {
                "stamp_sec": stamp_sec,
                "stamp_nanosec": stamp_nanosec,
                "cycle": cycle,
                "temp_c": temp_c,
                "vibration": vibration,
                "fault": fault,
            }
        )

    return cell_id.strip(), normalized


def sample_sort_key(sample: dict[str, Any]) -> str:
    return (
        f"{sample['stamp_sec']:010d}"
        f"#{sample['stamp_nanosec']:09d}"
        f"#{sample['cycle']:020d}"
        f"#{uuid.uuid4().hex[:8]}"
    )


def persist_samples(cell_id: str, samples: list[dict[str, Any]]) -> int:
    table = _table()
    ingested_at = int(time.time())
    with table.batch_writer() as batch:
        for sample in samples:
            item = {
                "pk": cell_id,
                "sk": sample_sort_key(sample),
                "cell_id": cell_id,
                "stamp_sec": sample["stamp_sec"],
                "stamp_nanosec": sample["stamp_nanosec"],
                "cycle": sample["cycle"],
                "temp_c": Decimal(str(sample["temp_c"])),
                "vibration": Decimal(str(sample["vibration"])),
                "fault": sample["fault"],
                "schema_version": 1,
                "ingested_at": ingested_at,
            }
            batch.put_item(Item=item)
    return len(samples)


def evaluate_rules(cell_id: str, samples: list[dict[str, Any]]) -> list[dict[str, Any]]:
    """Return alert event dicts (also logged as JSON lines)."""
    threshold = _temp_threshold()
    alerts: list[dict[str, Any]] = []
    for sample in samples:
        reasons: list[str] = []
        if sample["fault"]:
            reasons.append("fault")
        if threshold is not None and sample["temp_c"] > threshold:
            reasons.append("temp_c_threshold")
        if not reasons:
            continue
        event = {
            "event": "CELL_ALERT",
            "alert": True,
            "cell_id": cell_id,
            "cycle": sample["cycle"],
            "reason": ",".join(reasons),
            "temp_c": sample["temp_c"],
            "vibration": sample["vibration"],
            "fault": sample["fault"],
            "stamp_sec": sample["stamp_sec"],
            "stamp_nanosec": sample["stamp_nanosec"],
        }
        alerts.append(event)
        # Observable M3 side effect: structured CloudWatch log line
        LOG.info(json.dumps(event, separators=(",", ":")))
    return alerts


def _as_number(value: Any, label: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise TypeError(label)
    return float(value)


def _as_int(value: Any, label: str) -> int:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise TypeError(label)
    number = float(value)
    if not number.is_integer():
        raise ValueError(label)
    return int(number)


def _decimal(value: float) -> Decimal:
    return Decimal(str(value))


def validate_weld_snapshot(payload: Any) -> tuple[dict[str, Any], None] | tuple[None, str]:
    """Return (snapshot, None) or (None, error_message). schema_version 2 only."""
    if not isinstance(payload, dict):
        return None, "body must be a JSON object"
    if payload.get("schema_version") != 2:
        return None, "schema_version must be 2"

    timestamp = payload.get("timestamp")
    if not isinstance(timestamp, str) or _UTC_TIMESTAMP.fullmatch(timestamp) is None:
        return None, "timestamp must be a UTC RFC 3339 string with nine fractional digits"

    cell_id = payload.get("cell_id")
    if not isinstance(cell_id, str) or not cell_id.strip():
        return None, "cell_id must be a non-empty string"

    cell_type = payload.get("cell_type")
    if not isinstance(cell_type, str) or not cell_type.strip():
        return None, "cell_type must be a non-empty string"

    status = payload.get("status")
    if status not in _WELD_STATUSES:
        return None, "status must be RUNNING or FAULTED"

    program = payload.get("program")
    if not isinstance(program, str) or not program.strip():
        return None, "program must be a non-empty string"

    try:
        cycle_time_last_s = _as_number(payload.get("cycle_time_last_s"), "cycle_time_last_s")
        parts_total_shift = _as_int(payload.get("parts_total_shift"), "parts_total_shift")
        parts_scrap_shift = _as_int(payload.get("parts_scrap_shift"), "parts_scrap_shift")
        oee_availability = _as_number(payload.get("oee_availability"), "oee_availability")
    except (TypeError, ValueError) as exc:
        return None, f"invalid numeric field: {exc}"

    joints = payload.get("joints")
    if not isinstance(joints, list) or len(joints) != 3:
        return None, "joints must be an array of 3 objects"
    normalized_joints: list[dict[str, Any]] = []
    for i, joint in enumerate(joints):
        if not isinstance(joint, dict):
            return None, f"joints[{i}] must be an object"
        try:
            normalized_joints.append(
                {
                    "axis": _as_int(joint.get("axis"), "axis"),
                    "temp_c": _as_number(joint.get("temp_c"), "temp_c"),
                    "current_a": _as_number(joint.get("current_a"), "current_a"),
                }
            )
        except (TypeError, ValueError) as exc:
            return None, f"joints[{i}] invalid fields: {exc}"

    process = payload.get("process")
    if not isinstance(process, dict):
        return None, "process must be an object"
    normalized_process: dict[str, float] = {}
    for field in _PROCESS_FIELDS:
        try:
            normalized_process[field] = _as_number(process.get(field), field)
        except (TypeError, ValueError) as exc:
            return None, f"process invalid fields: {exc}"

    alarms = payload.get("alarms")
    if not isinstance(alarms, list):
        return None, "alarms must be an array"
    normalized_alarms: list[dict[str, str]] = []
    for i, alarm in enumerate(alarms):
        if not isinstance(alarm, dict):
            return None, f"alarms[{i}] must be an object"
        code = alarm.get("code")
        severity = alarm.get("severity")
        message = alarm.get("message")
        if not isinstance(code, str) or not code.strip():
            return None, f"alarms[{i}].code must be a non-empty string"
        if severity not in _ALARM_SEVERITIES:
            return None, f"alarms[{i}].severity must be info, warning, or error"
        if not isinstance(message, str) or not message.strip():
            return None, f"alarms[{i}].message must be a non-empty string"
        normalized_alarms.append(
            {"code": code.strip(), "severity": severity, "message": message.strip()}
        )

    return {
        "timestamp": timestamp,
        "cell_id": cell_id.strip(),
        "cell_type": cell_type.strip(),
        "status": status,
        "program": program.strip(),
        "cycle_time_last_s": cycle_time_last_s,
        "parts_total_shift": parts_total_shift,
        "parts_scrap_shift": parts_scrap_shift,
        "oee_availability": oee_availability,
        "joints": normalized_joints,
        "process": normalized_process,
        "alarms": normalized_alarms,
    }, None


def snapshot_sort_key(timestamp: str) -> str:
    return timestamp


def persist_weld_snapshot(snapshot: dict[str, Any]) -> int:
    table = _table()
    item = {
        "pk": snapshot["cell_id"],
        "sk": snapshot_sort_key(snapshot["timestamp"]),
        "cell_id": snapshot["cell_id"],
        "schema_version": 2,
        "timestamp": snapshot["timestamp"],
        "cell_type": snapshot["cell_type"],
        "status": snapshot["status"],
        "program": snapshot["program"],
        "cycle_time_last_s": _decimal(snapshot["cycle_time_last_s"]),
        "parts_total_shift": snapshot["parts_total_shift"],
        "parts_scrap_shift": snapshot["parts_scrap_shift"],
        "oee_availability": _decimal(snapshot["oee_availability"]),
        "joints": [
            {
                "axis": joint["axis"],
                "temp_c": _decimal(joint["temp_c"]),
                "current_a": _decimal(joint["current_a"]),
            }
            for joint in snapshot["joints"]
        ],
        "process": {field: _decimal(snapshot["process"][field]) for field in _PROCESS_FIELDS},
        "alarms": snapshot["alarms"],
        "ingested_at": int(time.time()),
    }
    table.put_item(Item=item)
    return 1


def evaluate_weld_rules(snapshot: dict[str, Any]) -> list[dict[str, Any]]:
    """One CELL_ALERT per alarm. A running snapshot with an empty alarms array emits none."""
    alerts: list[dict[str, Any]] = []
    for alarm in snapshot["alarms"]:
        event = {
            "event": "CELL_ALERT",
            "alert": True,
            "schema_version": 2,
            "cell_id": snapshot["cell_id"],
            "timestamp": snapshot["timestamp"],
            "status": snapshot["status"],
            "reason": "alarm",
            "code": alarm["code"],
            "severity": alarm["severity"],
            "message": alarm["message"],
        }
        alerts.append(event)
        LOG.info(json.dumps(event, separators=(",", ":")))
    return alerts


def validate_facility_snapshot(payload: Any) -> tuple[dict[str, Any], None] | tuple[None, str]:
    """Return (snapshot, None) or (None, error_message). schema_version 3 only."""
    if not isinstance(payload, dict):
        return None, "body must be a JSON object"
    if payload.get("schema_version") != 3:
        return None, "schema_version must be 3"

    timestamp = payload.get("timestamp")
    if not isinstance(timestamp, str) or _UTC_TIMESTAMP.fullmatch(timestamp) is None:
        return None, "timestamp must be a UTC RFC 3339 string with nine fractional digits"

    identity: dict[str, str] = {}
    for field in ("site_id", "hall_id", "asset_id", "asset_type", "unit"):
        value = payload.get(field)
        if not isinstance(value, str) or not value.strip():
            return None, f"{field} must be a non-empty string"
        identity[field] = value.strip()

    status = payload.get("status")
    if status not in _WELD_STATUSES:
        return None, "status must be RUNNING or FAULTED"

    try:
        reading = _as_number(payload.get("reading"), "reading")
    except (TypeError, ValueError) as exc:
        return None, f"invalid numeric field: {exc}"

    alarms = payload.get("alarms")
    if not isinstance(alarms, list):
        return None, "alarms must be an array"
    normalized_alarms: list[dict[str, str]] = []
    for i, alarm in enumerate(alarms):
        if not isinstance(alarm, dict):
            return None, f"alarms[{i}] must be an object"
        code = alarm.get("code")
        severity = alarm.get("severity")
        message = alarm.get("message")
        if not isinstance(code, str) or not code.strip():
            return None, f"alarms[{i}].code must be a non-empty string"
        if severity not in _ALARM_SEVERITIES:
            return None, f"alarms[{i}].severity must be info, warning, or error"
        if not isinstance(message, str) or not message.strip():
            return None, f"alarms[{i}].message must be a non-empty string"
        normalized_alarms.append(
            {"code": code.strip(), "severity": severity, "message": message.strip()}
        )

    return {
        "timestamp": timestamp,
        "status": status,
        "reading": reading,
        "alarms": normalized_alarms,
        **identity,
    }, None


def persist_facility_snapshot(snapshot: dict[str, Any]) -> int:
    table = _table()
    item = {
        "pk": snapshot["asset_id"],
        "sk": snapshot_sort_key(snapshot["timestamp"]),
        "schema_version": 3,
        "timestamp": snapshot["timestamp"],
        "site_id": snapshot["site_id"],
        "hall_id": snapshot["hall_id"],
        "asset_id": snapshot["asset_id"],
        "asset_type": snapshot["asset_type"],
        "status": snapshot["status"],
        "reading": _decimal(snapshot["reading"]),
        "unit": snapshot["unit"],
        "alarms": snapshot["alarms"],
        "ingested_at": int(time.time()),
    }
    table.put_item(Item=item)
    return 1


def process_facility_snapshot(payload: Any) -> tuple[int, dict[str, Any]]:
    snapshot, err = validate_facility_snapshot(payload)
    if snapshot is None:
        return 400, {"status": "error", "message": err}

    accepted = persist_facility_snapshot(snapshot)
    LOG.info(
        json.dumps(
            {
                "event": "INGEST_OK",
                "asset_id": snapshot["asset_id"],
                "accepted": accepted,
                "alerts": 0,
            },
            separators=(",", ":"),
        )
    )
    return 202, {"status": "ok", "accepted": accepted, "alerts": 0}


def process_weld_snapshot(payload: Any) -> tuple[int, dict[str, Any]]:
    snapshot, err = validate_weld_snapshot(payload)
    if snapshot is None:
        return 400, {"status": "error", "message": err}

    accepted = persist_weld_snapshot(snapshot)
    alerts = evaluate_weld_rules(snapshot)
    LOG.info(
        json.dumps(
            {
                "event": "INGEST_OK",
                "cell_id": snapshot["cell_id"],
                "accepted": accepted,
                "alerts": len(alerts),
            },
            separators=(",", ":"),
        )
    )
    return 202, {"status": "ok", "accepted": accepted, "alerts": len(alerts)}


def process_batch(payload: Any) -> tuple[int, dict[str, Any]]:
    if isinstance(payload, dict) and payload.get("schema_version") == 2:
        return process_weld_snapshot(payload)
    if isinstance(payload, dict) and payload.get("schema_version") == 3:
        return process_facility_snapshot(payload)
    if isinstance(payload, dict) and payload.get("schema_version") != 1:
        return 400, {"status": "error", "message": "schema_version must be 1, 2, or 3"}

    result = validate_and_normalize(payload)
    if result[0] is None:
        return 400, {"status": "error", "message": result[1]}

    cell_id, samples = result  # type: ignore[misc]
    accepted = persist_samples(cell_id, samples)
    alerts = evaluate_rules(cell_id, samples)
    LOG.info(
        json.dumps(
            {
                "event": "INGEST_OK",
                "cell_id": cell_id,
                "accepted": accepted,
                "alerts": len(alerts),
            },
            separators=(",", ":"),
        )
    )
    return 202, {"status": "ok", "accepted": accepted, "alerts": len(alerts)}


def handle_ingest(event: dict[str, Any], _context: Any) -> dict[str, Any]:
    expected = _expected_api_key()
    provided = _header(event.get("headers") or {}, "x-api-key")
    if not expected or provided != expected:
        LOG.warning(json.dumps({"event": "INGEST_UNAUTHORIZED"}))
        return _response(401, {"status": "error", "message": "unauthorized"})

    raw_body = event.get("body") or ""
    if event.get("isBase64Encoded"):
        import base64

        raw_body = base64.b64decode(raw_body).decode("utf-8")

    try:
        payload = json.loads(raw_body) if raw_body else None
    except json.JSONDecodeError:
        return _response(400, {"status": "error", "message": "invalid json"})

    status, body = process_batch(payload)
    return _response(status, body)
