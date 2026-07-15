"""
M3 ingest Lambda — validate batch, persist to DynamoDB, rule-based alert logs.

Contract: docs/contracts/telemetry-batch-http.md
ADR: docs/decisions/0003-cloud-floor.md
"""

from __future__ import annotations

import json
import logging
import os
import time
import uuid
from decimal import Decimal
from typing import Any

import boto3

LOG = logging.getLogger()
LOG.setLevel(logging.INFO)

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


def process_batch(payload: Any) -> tuple[int, dict[str, Any]]:
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
