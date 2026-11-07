# How-to: Two-cell weld demo

Start here to run both welding cells, the local historian, and the bridge with one command.

The container publishes `WELD_CELL_07` and `WELD_CELL_08` on `cell/weld_snapshot`. The historian writes every snapshot to InfluxDB. The bridge posts every 5th snapshot to the deployed API, and posts an alarm snapshot immediately.

**Assumption:** `p3-edge:jazzy` is already built and includes the historian. Compose uses that image and does not build it. Confirm the executable:

```bash
docker run --rm p3-edge:jazzy ros2 pkg executables cell_historian
```

Expect a line `cell_historian weld_historian_node`. If that line is missing, build from `edge/` with `./scripts/build.sh`, then run the demo. `--rm` deletes this check container when the command exits. The image stays. Details: [docker-fundamentals.md](../concepts/docker/docker-fundamentals.md).

**Prerequisites:** Docker Desktop; AWS `dev` stack deployed with the `schema_version` 2 handler; `cloud/.env` has `INGEST_URL` set to that stack's **IngestUrl** and `INGEST_API_KEY` matching the key Lambda was deployed with. Confirm the URL the same way as [edge-to-aws.md](edge-to-aws.md) step 1. Compose pulls `influxdb:2.7` if it is not already local.

---

## 1. Start

From `edge/`:

```bash
docker compose --profile weld up weld_demo
```

That starts InfluxDB, then one container. The container runs both publishers, `weld_historian_node`, and `weld_cell_bridge_node` (`reduce:=every_n`, `every_n:=5`). `INGEST_URL` and `INGEST_API_KEY` come from `cloud/.env`.

Leave `weld_cell_publisher`, `weld_historian`, and `weld_cell_bridge` stopped. Those are the single-process commands in [weld-publisher-bridge.md](weld-publisher-bridge.md).

**ROS 2 graph**

From `edge/`, open a shell in the demo container. `docker exec` does not run the image entrypoint, so source ROS in that shell before the `ros2` commands.

```bash
docker compose --profile weld exec weld_demo bash
source /opt/ros/jazzy/setup.bash
source /ws/install/setup.bash
```

Then list the graph:

```bash
ros2 node list
ros2 topic list
ros2 topic info /cell/weld_snapshot -v
ros2 topic hz /cell/weld_snapshot
ros2 topic echo /cell/weld_snapshot
```

For a live view on the host, see [Foxglove from a Docker container](../concepts/ros2/foxglove.md). Foxglove's ROS 2 connection steps are in the [Foxglove ROS 2 docs](https://docs.foxglove.dev/docs/getting-started/frameworks/ros2).

---

## 2. Check the three stores

| Check | Expect |
|-------|--------|
| Publisher logs | `WELD_CELL_07` and `WELD_CELL_08`, about one snapshot per second |
| Historian logs | `historian wrote` about once a second for each cell |
| Bridge logs | `ingest ok` with `status=202` about every fifth snapshot. An alarm snapshot is posted on that same snapshot, not on the next fifth |
| InfluxDB at [http://localhost:8086](http://localhost:8086) | User `p3`, password `p3-historian-dev`, bucket `weld`. About one point per second per `cell_id`, including snapshots the bridge skipped |
| DynamoDB table `p3-cell-telemetry-dev-samples` | Items for `pk` `WELD_CELL_07` and `WELD_CELL_08`, `schema_version` 2. About one item per five snapshots, plus any alarm snapshot between those posts |
| CloudWatch Logs (`/aws/lambda/p3-cell-telemetry-dev-ingest`) | `"event":"CELL_ALERT"` with `"reason":"alarm"` on the `GAS_FLOW_LOW` sample |

The first `GAS_FLOW_LOW` is the 15th completed cycle. A cycle is 42 snapshots, so at one snapshot per second that alarm is about 10.5 minutes after the publishers start. That snapshot is `FAULTED`, is in both stores, and is the one that logs `CELL_ALERT`.

Query one cell from the last hour:

```bash
docker compose --profile weld exec influxdb influx query --org p3 --token p3-historian-dev-token \
  'from(bucket:"weld") |> range(start:-1h) |> filter(fn:(r) => r.cell_id == "WELD_CELL_07")'
```

---

## 3. Stop

Ctrl+C stops the demo. From `edge/`:

```bash
docker compose --profile weld down
```

`down` removes the containers and the Compose network. It leaves the Influx volume and `p3-edge:jazzy` in place. Details: [cleaning-up.md](../concepts/docker/cleaning-up.md).

---

## Related

- Read the local history in the InfluxDB UI: [influxdb-weld.md](influxdb-weld.md)
- Live ROS graph from the host: [foxglove.md](../concepts/ros2/foxglove.md)
- Manual publisher, bridge, and historian commands: [weld-publisher-bridge.md](weld-publisher-bridge.md)
- Snapshot contract: [weld-cell-snapshot.md](../contracts/weld-cell-snapshot.md)
- Roadmap step this demo runs: [v2-architecture.md](../planning/v2-architecture.md)
