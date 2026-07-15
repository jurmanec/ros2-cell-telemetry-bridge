#!/usr/bin/env python3
"""
M4 smoke: POST fault batch to AWS ingest → assert DynamoDB + CELL_ALERT log.

Requires: AWS credentials, deployed stage, INGEST_API_KEY (env or cloud/.env).
Optional: INGEST_URL, AWS_REGION, STAGE, SAMPLES_TABLE, INGEST_FUNCTION.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
import time
import urllib.error
import urllib.request
from pathlib import Path

CLOUD = Path(__file__).resolve().parents[1]
DEFAULT_STAGE = "dev"
DEFAULT_REGION = "us-east-1"
SERVICE = "p3-cell-telemetry"


def load_dotenv(path: Path) -> None:
    if not path.is_file():
        return
    for line in path.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line or line.startswith("#") or "=" not in line:
            continue
        key, _, val = line.partition("=")
        key = key.strip()
        val = val.strip().strip("'").strip('"')
        os.environ.setdefault(key, val)


def fail(msg: str, code: int = 1) -> None:
    print(f"FAIL: {msg}", file=sys.stderr)
    raise SystemExit(code)


def ok(msg: str) -> None:
    print(f"OK: {msg}")


def resolve_ingest_url(stage: str, region: str) -> str:
    if url := os.environ.get("INGEST_URL", "").strip():
        return url.rstrip("/")

    try:
        import boto3
    except ImportError:
        boto3 = None  # type: ignore[assignment]

    if boto3 is not None:
        cf = boto3.client("cloudformation", region_name=region)
        stack = f"{SERVICE}-{stage}"
        try:
            outs = cf.describe_stacks(StackName=stack)["Stacks"][0].get("Outputs") or []
            for o in outs:
                if o.get("OutputKey") == "IngestUrl" and o.get("OutputValue"):
                    return str(o["OutputValue"]).rstrip("/")
        except Exception as exc:  # noqa: BLE001
            print(f"note: CloudFormation IngestUrl lookup failed: {exc}", file=sys.stderr)

    proc = subprocess.run(
        ["npx", "serverless", "info", "--stage", stage, "--region", region],
        cwd=CLOUD,
        capture_output=True,
        text=True,
    )
    combined = (proc.stdout or "") + (proc.stderr or "")
    matches = re.findall(
        r"https://[a-z0-9]+\.execute-api\.[a-z0-9-]+\.amazonaws\.com/v1/telemetry/batch",
        combined,
    )
    if matches:
        return matches[0].rstrip("/")
    fail(
        "could not resolve IngestUrl; set INGEST_URL or deploy stack "
        f"{SERVICE}-{stage}"
    )


def post_batch(url: str, api_key: str, cell_id: str, cycle: int) -> tuple[int, dict]:
    stamp_sec = int(time.time())
    body = {
        "schema_version": 1,
        "cell_id": cell_id,
        "samples": [
            {
                "stamp_sec": stamp_sec,
                "stamp_nanosec": 0,
                "cycle": cycle,
                "temp_c": 42.0,
                "vibration": 0.2,
                "fault": True,
            }
        ],
    }
    raw = json.dumps(body).encode("utf-8")
    req = urllib.request.Request(
        url,
        data=raw,
        method="POST",
        headers={
            "Content-Type": "application/json",
            "x-api-key": api_key,
        },
    )
    try:
        with urllib.request.urlopen(req, timeout=30) as resp:
            payload = json.loads(resp.read().decode("utf-8"))
            return resp.status, payload
    except urllib.error.HTTPError as e:
        text = e.read().decode("utf-8", "replace")
        try:
            payload = json.loads(text)
        except json.JSONDecodeError:
            payload = {"raw": text}
        return e.code, payload


def assert_dynamo(table_name: str, cell_id: str, region: str) -> None:
    import boto3

    table = boto3.resource("dynamodb", region_name=region).Table(table_name)
    resp = table.query(
        KeyConditionExpression="pk = :pk",
        ExpressionAttributeValues={":pk": cell_id},
        Limit=5,
    )
    items = resp.get("Items") or []
    if not items:
        fail(f"DynamoDB table {table_name!r}: no items for pk={cell_id!r}")
    ok(f"DynamoDB has {len(items)} item(s) for pk={cell_id}")


def assert_cell_alert(
    function_name: str,
    cell_id: str,
    region: str,
    start_ms: int,
    timeout_sec: float = 45.0,
) -> None:
    import boto3

    logs = boto3.client("logs", region_name=region)
    log_group = f"/aws/lambda/{function_name}"
    deadline = time.time() + timeout_sec
    last_err: str | None = None

    while time.time() < deadline:
        try:
            resp = logs.filter_log_events(
                logGroupName=log_group,
                startTime=start_ms,
                filterPattern='"CELL_ALERT"',
                limit=50,
            )
            for ev in resp.get("events") or []:
                msg = ev.get("message") or ""
                compact = msg.replace(" ", "")
                if '"event":"CELL_ALERT"' in compact and cell_id in msg:
                    ok(f"CloudWatch CELL_ALERT for cell_id={cell_id} in {log_group}")
                    return
            last_err = "no matching CELL_ALERT yet"
        except logs.exceptions.ResourceNotFoundException:
            last_err = f"log group not found: {log_group}"
        except Exception as exc:  # noqa: BLE001
            last_err = str(exc)
        time.sleep(2.0)

    fail(
        f"timed out waiting for CELL_ALERT (cell_id={cell_id}) in {log_group}: {last_err}"
    )


def main() -> int:
    load_dotenv(CLOUD / ".env")

    parser = argparse.ArgumentParser(description="M4 AWS ingest smoke")
    parser.add_argument("--stage", default=os.environ.get("STAGE", DEFAULT_STAGE))
    parser.add_argument("--region", default=os.environ.get("AWS_REGION", DEFAULT_REGION))
    parser.add_argument(
        "--skip-logs",
        action="store_true",
        help="Skip CloudWatch CELL_ALERT poll (still checks HTTP + DynamoDB)",
    )
    args = parser.parse_args()
    stage = args.stage
    region = args.region

    api_key = os.environ.get("INGEST_API_KEY", "").strip()
    if not api_key:
        fail("INGEST_API_KEY missing (set in environment or cloud/.env)")

    url = resolve_ingest_url(stage, region)
    table = os.environ.get("SAMPLES_TABLE", f"{SERVICE}-{stage}-samples")
    function_name = os.environ.get("INGEST_FUNCTION", f"{SERVICE}-{stage}-ingest")

    cell_id = f"smoke-{int(time.time())}"
    cycle = int(time.time()) % 1_000_000_000
    start_ms = int((time.time() - 5) * 1000)

    print(f"POST {url}")
    print(f"cell_id={cell_id} table={table} function={function_name}")

    status, body = post_batch(url, api_key, cell_id, cycle)
    if status != 202:
        fail(f"HTTP {status} body={body}")
    if body.get("status") != "ok":
        fail(f"unexpected body status: {body}")
    alerts = int(body.get("alerts") or 0)
    if alerts < 1:
        fail(f"expected alerts>=1, got {body}")
    ok(f"HTTP 202 accepted={body.get('accepted')} alerts={alerts}")

    assert_dynamo(table, cell_id, region)

    if args.skip_logs:
        print("SKIP: CloudWatch log assert (--skip-logs)")
    else:
        assert_cell_alert(function_name, cell_id, region, start_ms)

    print("SMOKE PASSED")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
