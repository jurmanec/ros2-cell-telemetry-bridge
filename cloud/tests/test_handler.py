import json
import os
import sys
from unittest.mock import MagicMock, patch

import pytest

# Allow importing src.handler without packaging
sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))

from src import handler


SAMPLE = {
    "stamp_sec": 10,
    "stamp_nanosec": 20,
    "cycle": 3,
    "temp_c": 41.5,
    "vibration": 0.16,
    "fault": False,
}


def test_validate_ok():
    payload = {"schema_version": 1, "cell_id": "cell-001", "samples": [SAMPLE]}
    cell_id, samples = handler.validate_and_normalize(payload)
    assert cell_id == "cell-001"
    assert samples[0]["cycle"] == 3


def test_validate_rejects_empty_samples():
    cell_id, err = handler.validate_and_normalize(
        {"schema_version": 1, "cell_id": "c", "samples": []}
    )
    assert cell_id is None
    assert "non-empty" in err


def test_fault_rule_emits_alert(caplog):
    os.environ["SAMPLES_TABLE"] = "unused"
    os.environ["INGEST_API_KEY"] = "secret"
    os.environ.pop("TEMP_C_THRESHOLD", None)

    samples = [{**SAMPLE, "fault": True, "cycle": 50}]
    with caplog.at_level("INFO"):
        alerts = handler.evaluate_rules("cell-001", samples)

    assert len(alerts) == 1
    assert alerts[0]["event"] == "CELL_ALERT"
    assert alerts[0]["reason"] == "fault"
    assert any("CELL_ALERT" in r.message for r in caplog.records)


def test_temp_threshold_rule():
    os.environ["TEMP_C_THRESHOLD"] = "40.0"
    samples = [{**SAMPLE, "fault": False, "temp_c": 45.0}]
    alerts = handler.evaluate_rules("cell-001", samples)
    assert len(alerts) == 1
    assert "temp_c_threshold" in alerts[0]["reason"]


def test_handle_ingest_unauthorized():
    os.environ["INGEST_API_KEY"] = "secret"
    event = {
        "headers": {"x-api-key": "wrong"},
        "body": json.dumps(
            {"schema_version": 1, "cell_id": "cell-001", "samples": [SAMPLE]}
        ),
    }
    resp = handler.handle_ingest(event, None)
    assert resp["statusCode"] == 401


WELD = {
    "schema_version": 2,
    "timestamp": "2026-11-06T17:05:22.011192632Z",
    "cell_id": "WELD_CELL_07",
    "cell_type": "robotic_welding",
    "status": "RUNNING",
    "program": "DOOR_FRAME_WELD_V4",
    "cycle_time_last_s": 42.1,
    "parts_total_shift": 156,
    "parts_scrap_shift": 3,
    "oee_availability": 0.91,
    "joints": [
        {"axis": 1, "temp_c": 49.2, "current_a": 4.1},
        {"axis": 2, "temp_c": 56.8, "current_a": 6.3},
        {"axis": 3, "temp_c": 51.4, "current_a": 4.7},
    ],
    "process": {
        "weld_current_a": 210,
        "weld_voltage_v": 23.1,
        "wire_feed_m_min": 7.4,
        "arc_on_s": 19.6,
        "gas_flow_l_min": 15.0,
    },
    "alarms": [],
}


def test_weld_snapshot_persists_without_alert():
    os.environ["INGEST_API_KEY"] = "secret"
    os.environ["SAMPLES_TABLE"] = "t"

    mock_table = MagicMock()
    event = {"headers": {"x-api-key": "secret"}, "body": json.dumps(WELD)}

    with patch.object(handler, "_table", return_value=mock_table):
        resp = handler.handle_ingest(event, None)

    assert resp["statusCode"] == 202
    body = json.loads(resp["body"])
    assert body == {"status": "ok", "accepted": 1, "alerts": 0}
    item = mock_table.put_item.call_args.kwargs["Item"]
    assert item["pk"] == "WELD_CELL_07"
    assert item["schema_version"] == 2
    assert item["sk"] == "2026-11-06T17:05:22.011192632Z"
    assert item["joints"][1]["axis"] == 2
    assert item["process"]["weld_current_a"] == handler.Decimal("210")
    assert item["alarms"] == []


def test_weld_alarm_emits_cell_alert(caplog):
    os.environ["INGEST_API_KEY"] = "secret"
    os.environ["SAMPLES_TABLE"] = "t"
    payload = {
        **WELD,
        "status": "FAULTED",
        "alarms": [
            {
                "code": "GAS_FLOW_LOW",
                "severity": "warning",
                "message": "Shielding gas flow below setpoint",
            }
        ],
    }
    event = {"headers": {"x-api-key": "secret"}, "body": json.dumps(payload)}

    with patch.object(handler, "_table", return_value=MagicMock()), caplog.at_level("INFO"):
        resp = handler.handle_ingest(event, None)

    assert resp["statusCode"] == 202
    assert json.loads(resp["body"])["alerts"] == 1
    assert any('"event":"CELL_ALERT"' in r.message and "GAS_FLOW_LOW" in r.message for r in caplog.records)


def test_weld_rejects_bad_shape():
    os.environ["INGEST_API_KEY"] = "secret"
    payload = {**WELD, "joints": []}
    event = {"headers": {"x-api-key": "secret"}, "body": json.dumps(payload)}
    resp = handler.handle_ingest(event, None)
    assert resp["statusCode"] == 400


FACILITY = {
    "schema_version": 3,
    "timestamp": "2026-11-12T21:00:00.111192632Z",
    "site_id": "SITE-01",
    "hall_id": "HALL-01",
    "asset_id": "CRAH-01",
    "asset_type": "crah",
    "status": "RUNNING",
    "reading": 16,
    "unit": "C",
    "alarms": [],
}


def test_facility_snapshot_persists_without_alert():
    os.environ["INGEST_API_KEY"] = "secret"
    os.environ["SAMPLES_TABLE"] = "t"

    mock_table = MagicMock()
    event = {"headers": {"x-api-key": "secret"}, "body": json.dumps(FACILITY)}

    with patch.object(handler, "_table", return_value=mock_table):
        resp = handler.handle_ingest(event, None)

    assert resp["statusCode"] == 202
    body = json.loads(resp["body"])
    assert body == {"status": "ok", "accepted": 1, "alerts": 0}
    item = mock_table.put_item.call_args.kwargs["Item"]
    assert item["pk"] == "CRAH-01"
    assert item["schema_version"] == 3
    assert item["sk"] == "2026-11-12T21:00:00.111192632Z"
    assert item["asset_type"] == "crah"
    assert item["reading"] == handler.Decimal("16")
    assert item["unit"] == "C"
    assert item["alarms"] == []


def test_facility_replay_keeps_the_same_sort_key():
    os.environ["INGEST_API_KEY"] = "secret"
    os.environ["SAMPLES_TABLE"] = "t"
    event = {"headers": {"x-api-key": "secret"}, "body": json.dumps(FACILITY)}
    mock_table = MagicMock()

    with patch.object(handler, "_table", return_value=mock_table):
        first = handler.handle_ingest(event, None)
        second = handler.handle_ingest(event, None)

    assert first["statusCode"] == 202
    assert second["statusCode"] == 202
    keys = [call.kwargs["Item"]["sk"] for call in mock_table.put_item.call_args_list]
    assert keys == [
        "2026-11-12T21:00:00.111192632Z",
        "2026-11-12T21:00:00.111192632Z",
    ]


def test_facility_rejects_a_short_fractional_second():
    os.environ["INGEST_API_KEY"] = "secret"
    payload = {**FACILITY, "timestamp": "2026-11-12T21:00:00.1Z"}
    event = {"headers": {"x-api-key": "secret"}, "body": json.dumps(payload)}
    resp = handler.handle_ingest(event, None)
    assert resp["statusCode"] == 400
    assert "nine fractional digits" in json.loads(resp["body"])["message"]


def test_facility_alarm_is_stored_and_does_not_emit_cell_alert(caplog):
    os.environ["INGEST_API_KEY"] = "secret"
    os.environ["SAMPLES_TABLE"] = "t"
    payload = {
        **FACILITY,
        "status": "FAULTED",
        "reading": 28,
        "alarms": [
            {
                "code": "COOLING_SUPPLY_HIGH",
                "severity": "warning",
                "message": "Supply air above setpoint",
            }
        ],
    }
    event = {"headers": {"x-api-key": "secret"}, "body": json.dumps(payload)}

    with patch.object(handler, "_table", return_value=MagicMock()) as mock_table, caplog.at_level("INFO"):
        resp = handler.handle_ingest(event, None)

    assert resp["statusCode"] == 202
    assert json.loads(resp["body"])["alerts"] == 0
    item = mock_table.return_value.put_item.call_args.kwargs["Item"]
    assert item["status"] == "FAULTED"
    assert item["alarms"][0]["code"] == "COOLING_SUPPLY_HIGH"
    assert not any("CELL_ALERT" in r.message for r in caplog.records)


def test_unknown_schema_version_rejected():
    os.environ["INGEST_API_KEY"] = "secret"
    event = {
        "headers": {"x-api-key": "secret"},
        "body": json.dumps({"schema_version": 4, "cell_id": "cell-001"}),
    }
    resp = handler.handle_ingest(event, None)
    assert resp["statusCode"] == 400
    assert "1, 2, or 3" in json.loads(resp["body"])["message"]


def test_handle_ingest_ok():
    os.environ["INGEST_API_KEY"] = "secret"
    os.environ["SAMPLES_TABLE"] = "t"
    os.environ.pop("TEMP_C_THRESHOLD", None)

    mock_table = MagicMock()
    mock_batch = MagicMock()
    mock_table.batch_writer.return_value.__enter__.return_value = mock_batch
    mock_table.batch_writer.return_value.__exit__.return_value = None

    event = {
        "headers": {"x-api-key": "secret"},
        "body": json.dumps(
            {
                "schema_version": 1,
                "cell_id": "cell-001",
                "samples": [{**SAMPLE, "fault": True}],
            }
        ),
    }

    with patch.object(handler, "_table", return_value=mock_table):
        resp = handler.handle_ingest(event, None)

    assert resp["statusCode"] == 202
    body = json.loads(resp["body"])
    assert body["accepted"] == 1
    assert body["alerts"] == 1
    assert mock_batch.put_item.called
