#!/usr/bin/env python3
"""
Delete every item in the samples table. The table itself stays.

Requires AWS credentials that can Scan and BatchWriteItem the table.
Default table: p3-cell-telemetry-dev-samples (override with SAMPLES_TABLE, --stage, or --table).

From cloud/:

    .venv/bin/python scripts/clear_samples.py --yes
"""

from __future__ import annotations

import argparse
import os
import sys
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


def delete_all(table) -> int:
    deleted = 0
    scan_kwargs: dict = {"ProjectionExpression": "pk, sk"}
    while True:
        page = table.scan(**scan_kwargs)
        items = page.get("Items", [])
        if items:
            with table.batch_writer() as batch:
                for item in items:
                    batch.delete_item(Key={"pk": item["pk"], "sk": item["sk"]})
                    deleted += 1
        last_key = page.get("LastEvaluatedKey")
        if not last_key:
            return deleted
        scan_kwargs["ExclusiveStartKey"] = last_key


def main() -> int:
    load_dotenv(CLOUD / ".env")

    parser = argparse.ArgumentParser(description="Delete every item in the samples table")
    parser.add_argument("--stage", default=os.environ.get("STAGE", DEFAULT_STAGE))
    parser.add_argument("--region", default=os.environ.get("AWS_REGION", DEFAULT_REGION))
    parser.add_argument(
        "--table",
        default="",
        help="DynamoDB table name (default: SAMPLES_TABLE or p3-cell-telemetry-<stage>-samples)",
    )
    parser.add_argument(
        "--yes",
        action="store_true",
        help="Delete the items. Without this flag the script only prints the table name.",
    )
    args = parser.parse_args()

    table_name = args.table.strip() or os.environ.get("SAMPLES_TABLE", "").strip()
    if not table_name:
        table_name = f"{SERVICE}-{args.stage}-samples"

    try:
        import boto3
    except ImportError:
        fail("boto3 is not installed. From cloud/: .venv/bin/python scripts/clear_samples.py --yes")

    table = boto3.resource("dynamodb", region_name=args.region).Table(table_name)
    if not args.yes:
        print(f"Would delete every item in {table_name} ({args.region}). Re-run with --yes.")
        return 0

    try:
        deleted = delete_all(table)
    except Exception as exc:  # noqa: BLE001
        fail(f"{table_name}: {exc}")

    print(f"Deleted {deleted} item(s) from {table_name}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
