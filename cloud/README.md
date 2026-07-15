# P3 cloud floor (M3)

AWS ingest for industrial cell telemetry batches. Locks: [ADR 0003](../docs/decisions/0003-cloud-floor.md).  
Wire contract: [telemetry-batch-http.md](../docs/contracts/telemetry-batch-http.md).

## Stack

- API Gateway **HTTP API** `POST /v1/telemetry/batch`
- **Python 3.12** Lambda (validate → DynamoDB → fault/threshold rules → `CELL_ALERT` logs)
- **DynamoDB** on-demand (`pk=cell_id`, `sk=time#cycle#…`)
- Auth: request header **`x-api-key`** must match `INGEST_API_KEY` (shared secret / demo API key)

## Prerequisites

- Node.js 18+ (Serverless CLI)
- AWS credentials configured (`aws sts get-caller-identity`)
- `npm install` in this directory

Lambda runtime is **python3.12** (ADR 0003). Serverless Framework v3’s bundled config schema stops at `python3.11`; `plugins/aws-lambda-runtime-schema-patch.js` adds `python3.12` so deploy/print do not warn.

## Configure

```bash
cd cloud
cp .env.example .env   # then set INGEST_API_KEY to a strong secret
# optional in .env: TEMP_C_THRESHOLD=50
```

`serverless.yml` has `useDotenv: true`, so deploy/print read `INGEST_API_KEY` from `.env` (or from a shell `export`, which still wins if set). Redeploy after changing `.env` so Lambda gets the new value.

## Test locally

**Do not** run `python3 -m venv .venv` on top of an existing folder built by a different Python (Conda `(base)` vs Homebrew). That fails with `ensurepip … non-zero exit status 1` and can leave `.venv` half-broken (arm64 Conda mixed with x86_64 Homebrew).

Create once with a single interpreter, then reuse:

```bash
cd cloud
# only if missing or broken:
rm -rf .venv
python3 -m venv .venv   # with Conda (base); or: /usr/local/bin/python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt pytest
pytest -q
```

Or call venv binaries without activating:

```bash
cd cloud
.venv/bin/pip install -r requirements.txt pytest
.venv/bin/pytest -q
```

Expect **6 passed**.

## Deploy (dev)

```bash
cd cloud
npm install
# INGEST_API_KEY comes from .env via useDotenv (or export to override)
npx serverless deploy --stage dev --region us-east-1
npx serverless info --stage dev --region us-east-1
```

Note the **IngestUrl** output.

## Curl check

```bash
# Use IngestUrl from: npx serverless info --stage dev --region us-east-1
URL='https://xxxx.execute-api.us-east-1.amazonaws.com/v1/telemetry/batch'
curl -sS -w '\nHTTP %{http_code}\n' -X POST "$URL" \
  -H "Content-Type: application/json" \
  -H "x-api-key: $INGEST_API_KEY" \
  -d '{"schema_version":1,"cell_id":"cell-001","samples":[{"stamp_sec":1,"stamp_nanosec":0,"cycle":50,"temp_c":40.5,"vibration":0.12,"fault":true}]}'
```

Expect `202` and `{"status":"ok","accepted":1,"alerts":1}`. In CloudWatch Logs for the ingest function, look for `"event":"CELL_ALERT"`.

Log event shapes (for smoke / Insights): [cloud-ingest-events.md](../docs/contracts/cloud-ingest-events.md).

If you see `{"message":"Not Found"}`, the request never hit Lambda. Common causes: missing `/v1/telemetry/batch`, using `/dev/...` (HTTP APIs do not put the stage in the path), or GET instead of POST. Auth failures look like `{"status":"error","message":"unauthorized"}` (401), not Gateway Not Found.

## M4 smoke (one command)

Requires deployed `dev` stack, AWS credentials, and `INGEST_API_KEY` in `.env` (same value Lambda has). Uses venv boto3 if present:

```bash
cd cloud
# optional: export INGEST_URL='https://….execute-api.us-east-1.amazonaws.com/v1/telemetry/batch'
.venv/bin/python scripts/smoke_ingest.py
# or: npm run smoke
```

Asserts HTTP `202` + `alerts≥1`, a DynamoDB row for a unique smoke `cell_id`, and a CloudWatch log line containing `"event":"CELL_ALERT"`. Events: [cloud-ingest-events.md](../docs/contracts/cloud-ingest-events.md).

Edge path (publisher → bridge → same URL/`api_key`) was verified in M3; see below. Smoke does not rebuild the ROS stack.

## Point the edge bridge at cloud

Rebuild the edge image after pulling bridge `api_key` support, then run bridge with:

```bash
ros2 run cell_bridge cell_bridge_node --ros-args \
  -p ingest_url:=https://xxxx.execute-api.us-east-1.amazonaws.com/v1/telemetry/batch \
  -p api_key:=$INGEST_API_KEY \
  -p cell_id:=cell-001
```

(or set the same params in Compose / `docker run`). Local stub demos can leave `api_key` empty.

Keep `./scripts/run_m2_stack.sh` for local stub-only demos.

## Remove

```bash
npx serverless remove --stage dev --region us-east-1
```
