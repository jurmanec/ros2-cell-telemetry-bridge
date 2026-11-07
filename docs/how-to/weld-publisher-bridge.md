# How-to: Welding-cell publisher and bridge

Run `weld_cell_publisher` and `weld_cell_bridge` so each snapshot POSTs to the **deployed** API Gateway URL, not the local stub.

The publisher writes `cell/weld_snapshot` and does not take `ingest_url` or `api_key`. The bridge copies `cell_id` from each snapshot and POSTs one `schema_version` 2 body.

**Assumption:** the edge image is already built (`p3-edge:jazzy`, or `$EDGE_IMAGE`). This guide does not run `./scripts/build.sh`. Rebuild that image only if it predates `weld_cell_publisher` and `weld_cell_bridge_node`.

**Prerequisites:** Docker Desktop; AWS `dev` stack deployed with the `schema_version` 2 handler; `cloud/.env` has `INGEST_API_KEY` matching the value Lambda was deployed with, and `INGEST_URL` set to that stack's **IngestUrl**. Confirm the URL the same way as [edge-to-aws.md](edge-to-aws.md) step 1. A Lambda deployed before the version-2 handler rejects these bodies with HTTP 400.

---

## Two-cell demo

The one-command demo (both cells, historian, and reduced bridge) is in [weld-demo.md](weld-demo.md). The steps below start the publisher, bridge, and historian one process at a time.

---

## 1. Load URL and API key from `cloud/.env`

The bridge reads ROS parameters, not the environment. From `edge/`, load `cloud/.env` so `$INGEST_URL` and `$INGEST_API_KEY` expand in the bridge command.

```bash
cd edge
set -a
source ../cloud/.env
set +a
```

`INGEST_URL` must be the deployed **IngestUrl** (uncomment it in `cloud/.env`). `INGEST_API_KEY` must match the key Lambda was deployed with.

---

## 2. Run publisher + bridge (Compose, no stub)

`weld_cell_bridge` in `docker-compose.yml` hardcodes the stub URL and omits `api_key`. Start the publisher alone, then override the bridge command so it shares the Compose network.

First Change directories to `edge`.
```bash
cd edge
```
### Publisher

**Start with the parameters in `docker-compose.yml`.** The file sets `cell_id:=WELD_CELL_07`. `period_sec` stays at the node default, `1.0`. This service does not start the stub.

```bash
docker compose --profile weld up weld_cell_publisher
```

**Set parameters on the command line.** Anything after the service name replaces the compose command for this container, so `cell_id` and `period_sec` can change without editing the file. `--rm` removes the container when the process exits.

```bash
cd edge
docker compose --profile weld run --rm weld_cell_publisher \
  ros2 run cell_sim weld_cell_publisher --ros-args \
  -p cell_id:=WELD_CELL_07 \
  -p period_sec:=1.0
```

**Simulate many cells.** If weld cell 07 (above) was simulated in terminal A, run the following in terminal B. We use a distinct node name `-r __node:=weld_cell_08` (remap the node name), and cell_id `-p cell_id:=WELD_CELL_08`.
```bash
cd edge
docker compose --profile weld run --rm weld_cell_publisher \
ros2 run cell_sim weld_cell_publisher --ros-args \
-r __node:=weld_cell_08 \
-p cell_id:=WELD_CELL_08 \
-p period_sec:=1.0
```

### Bridge → AWS

Docker compose. `reduce` is one policy for every welding cell:

- `all` posts every snapshot. This is the default.
- `every_n` posts every Nth snapshot from each cell (`every_n`, default 5). A snapshot with an alarm is always posted.
- `heartbeat` posts when `heartbeat_sec` (default 5.0) has passed since that cell's last post. A snapshot with an alarm is always posted.

Any other value is invalid.

```bash
cd edge
docker compose --profile weld run --rm --no-deps weld_cell_bridge \
  ros2 run cell_bridge weld_cell_bridge_node --ros-args \
  -p ingest_url:=$INGEST_URL \
  -p api_key:=$INGEST_API_KEY \
  -p reduce:=every_n \
  -p every_n:=5
```

`--no-deps` skips the stub and a second publisher. Do not start the `ingest` stub service for this demo.

Ros2
```bash
ros2 run cell_bridge weld_cell_bridge_node --ros-args \
-p ingest_url:=$INGEST_URL \
-p api_key:=$INGEST_API_KEY \
-p reduce:=every_n \
-p every_n:=5
```

### Local historian

Start this beside the publisher. It records every snapshot in InfluxDB and does not post to AWS. The bucket keeps 7 days.

```bash
docker compose --profile weld up weld_historian
```

Query one cell from the last hour:

```bash
docker compose --profile weld exec influxdb influx query --org p3 --token p3-historian-dev-token \
  'from(bucket:"weld") |> range(start:-1h) |> filter(fn:(r) => r.cell_id == "WELD_CELL_07")'
```

UI at localhost 8086. See `docker-compose.yml` for details.

### Equivalent `ros2 run` (shell inside a container)

`docker exec` does not run the image entrypoint, so ROS is not on `PATH` until you source it. That shell also does not have `cloud/.env`. Export the two values there before starting the bridge (same URL and key as on the host).

```bash
source /opt/ros/jazzy/setup.bash
source /ws/install/setup.bash
export INGEST_URL="https://…"
export INGEST_API_KEY="…"
```

Publisher, if this container is not already running it:

```bash
ros2 run cell_sim weld_cell_publisher --ros-args \
  -p cell_id:=WELD_CELL_07
```

Bridge. Run this in the same container as the publisher when a second container does not see the topic:

```bash
ros2 run cell_bridge weld_cell_bridge_node --ros-args \
  -p ingest_url:=$INGEST_URL \
  -p api_key:=$INGEST_API_KEY
```

---

## 3. Verify

| Check | Expect |
|-------|--------|
| Publisher logs | `cell_id` `WELD_CELL_07`, `status` `RUNNING`, `arc_on_s` climbing while the arc is on |
| Bridge logs | `ingest ok` with `status=202` (not connection errors, HTTP 401, or HTTP 400) |
| DynamoDB table `p3-cell-telemetry-dev-samples` | New items for `pk` / `cell_id` = `WELD_CELL_07`, `schema_version` 2, nested `joints`, `process`, and `alarms` |
| CloudWatch Logs (`/aws/lambda/p3-cell-telemetry-dev-ingest`) | No `CELL_ALERT` on a running snapshot. On the `GAS_FLOW_LOW` sample: `"event":"CELL_ALERT"` with `"reason":"alarm"` |

Every 15th completed cycle is `FAULTED` with one `GAS_FLOW_LOW` alarm. That snapshot is the one that logs `CELL_ALERT`. Event shapes: [cloud-ingest-events.md](../contracts/cloud-ingest-events.md).

---

## 4. Stop

Ctrl+C in terminal A stops the publisher. The bridge container exits with `--rm`. From `edge/`, remove leftover containers:

```bash
docker compose --profile weld down
```

`down` removes the containers and the Compose network. It leaves `p3-edge:jazzy` in place. Details: [cleaning-up.md](../concepts/docker/cleaning-up.md).

---

## Troubleshooting

| Symptom | Likely cause |
|---------|----------------|
| HTTP 401 / `unauthorized` | `api_key` ≠ Lambda `INGEST_API_KEY`. Redeploy after changing `INGEST_API_KEY` in `.env`. `INGEST_URL` is not a Lambda setting |
| HTTP 400 / `schema_version must be 1` | The deployed Lambda predates the version-2 handler. Redeploy from `cloud/` |
| Bridge posts to an empty URL | `INGEST_URL` still commented in `cloud/.env`, or this shell was not loaded with `set -a` and `source` |
| Bridge never POSTs | Publisher not running, or the two containers are not on the same Compose network |
| Stub still receiving traffic | The bridge was started with `docker compose --profile weld up weld_cell_bridge`, which keeps `ingest_url` on `http://ingest:8080/...` |

---

## Related

- Snapshot contract: [weld-cell-snapshot.md](../contracts/weld-cell-snapshot.md)
- Version-1 publisher and bridge to AWS: [edge-to-aws.md](edge-to-aws.md)
