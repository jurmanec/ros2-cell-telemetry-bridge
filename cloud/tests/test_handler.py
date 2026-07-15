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
