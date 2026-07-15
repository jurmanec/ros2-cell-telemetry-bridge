# Edge (ROS 2 / C++)

Manufacturing cell simulator + HTTP bridge for P3. Jazzy + `rclcpp`, Docker on macOS.

## Prerequisites

- Docker Desktop running (`docker info` succeeds)

## Quick start — publisher only

```bash
cd edge
chmod +x scripts/*.sh docker/entrypoint.sh
./scripts/build.sh
./scripts/run_publisher.sh          # terminal A
./scripts/echo_topic.sh             # terminal B
```

## M2 stack — publisher + bridge + stub ingest

```bash
cd edge
chmod +x scripts/*.sh
./scripts/run_m2_stack.sh
```

Watch stub logs for `accepted cell_id=... samples=N`. Host curl check:

```bash
curl -sS -X POST http://localhost:8080/v1/telemetry/batch \
  -H 'Content-Type: application/json' \
  -d '{"schema_version":1,"cell_id":"cell-001","samples":[{"stamp_sec":1,"stamp_nanosec":0,"cycle":1,"temp_c":40.5,"vibration":0.12,"fault":false}]}'
```

Contracts: [telemetry-batch-http.md](../docs/contracts/telemetry-batch-http.md), [bridge-http-retry.md](../docs/bridge-http-retry.md), [ADR 0002](../docs/decisions/0002-bridge-transport.md).

## Dev Container (IDE / IntelliSense)

1. Install **Dev Containers** (+ **clangd**; do not also enable Microsoft C/C++ IntelliSense).
2. **File → Open Folder…** → open this `edge/` directory.
3. **Dev Containers: Reopen in Container**.
4. Build: `colcon build --packages-up-to cell_sim cell_bridge --cmake-args -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`

## Layout

```
edge/
  .devcontainer/       IDE container
  docker/              ROS runtime image
  docker-compose.yml   M2: publisher + bridge + ingest stub
  stub_ingest/         Python HTTP stub (M2)
  scripts/             build / run_publisher / echo_topic / run_m2_stack
  ws/src/cell_msgs/    CellTelemetry.msg
  ws/src/cell_sim/     cell publisher
  ws/src/cell_bridge/  ROS → HTTP JSON bridge
```
