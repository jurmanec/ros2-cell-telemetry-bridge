# How-to: Edge ROS nodes → AWS ingest

Run the Jazzy `cell_sim` publisher and `cell_bridge` so batches POST to the **deployed** API Gateway URL (DynamoDB + `CELL_ALERT` logs), not the local M2 stub.

`./scripts/run_m2_stack.sh` always targets the stub (`http://ingest:8080/...`). Use this guide instead for cloud mode.

**Prerequisites:** Docker Desktop; AWS `dev` stack deployed; `INGEST_API_KEY` in `cloud/.env` matches the value Lambda was deployed with.

---

## 1. Confirm cloud ingest

```bash
cd cloud
npx serverless info --stage dev --region us-east-1
```

Copy **IngestUrl**. It must end with `/v1/telemetry/batch` (HTTP API — do **not** insert `/dev/` in the path).

Optional sanity check without ROS:

```bash
cd cloud
.venv/bin/python scripts/smoke_ingest.py   # expect SMOKE PASSED
```

Details: [cloud/README.md](../../cloud/README.md).

---

## 2. Build the edge image

```bash
cd edge
chmod +x scripts/*.sh docker/entrypoint.sh
./scripts/build.sh    # tags p3-edge:jazzy (or $EDGE_IMAGE)
```

Rebuild if your image predates bridge `api_key` → `x-api-key` support.

---

## 3. Export URL and API key

```bash
export INGEST_URL='https://xxxx.execute-api.us-east-1.amazonaws.com/v1/telemetry/batch'
export INGEST_API_KEY='…'   # same secret as Lambda / cloud/.env
```

---

## 4. Run publisher + bridge (Compose, no stub)

Default `docker-compose.yml` hardcodes the stub URL and omits `api_key`. Start the publisher alone, then override the bridge command so it shares the Compose network (DDS discovery).

**Terminal A — publisher**

```bash
cd edge
docker compose up cell_publisher
```

**Terminal B — bridge → AWS**

```bash
cd edge
docker compose run --rm --no-deps cell_bridge \
  ros2 run cell_bridge cell_bridge_node --ros-args \
  -p ingest_url:=$INGEST_URL \
  -p api_key:=$INGEST_API_KEY \
  -p cell_id:=cell-001 \
  -p batch_size:=5 \
  -p flush_period_sec:=1.0
```

Do not start the `ingest` stub service for this demo.

### Equivalent `ros2 run` (inside the edge image / Dev Container)

If you already have a sourced workspace (e.g. Dev Container):

```bash
ros2 run cell_bridge cell_bridge_node --ros-args \
  -p ingest_url:=$INGEST_URL \
  -p api_key:=$INGEST_API_KEY \
  -p cell_id:=cell-001
```

Run `cell_publisher` in another process on the same ROS domain.

---

## 5. Verify

| Check | Expect |
|-------|--------|
| Bridge logs | Successful POSTs (not connection errors or HTTP 401) |
| DynamoDB table `p3-cell-telemetry-dev-samples` | New items for `pk` / `cell_id` = `cell-001` |
| CloudWatch Logs (`/aws/lambda/p3-cell-telemetry-dev-ingest`) | On fault samples: `"event":"CELL_ALERT"` (JSON inside the Lambda log line) |

Event shapes: [cloud-ingest-events.md](../contracts/cloud-ingest-events.md).

---

## Troubleshooting

| Symptom | Likely cause |
|---------|----------------|
| HTTP 401 / `unauthorized` | `api_key` ≠ Lambda `INGEST_API_KEY`, or stack not redeployed after changing `.env` |
| `{"message":"Not Found"}` | Wrong URL — missing `/v1/telemetry/batch`, or REST-style `/dev/...` prefix on an HTTP API |
| Bridge never POSTs / empty topic | Publisher not running, or containers not on the same Compose network / ROS domain |
| Stub still receiving traffic | You used `run_m2_stack.sh` or left `ingest_url` pointing at `http://ingest:8080/...` |

---

## Related

- Local stub-only demo: [edge/README.md](../../edge/README.md) (`run_m2_stack.sh`)
- Cloud deploy / smoke: [cloud/README.md](../../cloud/README.md)
- Bridge transport ADR: [0002-bridge-transport.md](../decisions/0002-bridge-transport.md)
