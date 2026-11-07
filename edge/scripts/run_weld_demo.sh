#!/bin/bash
# Two weld cells, the local historian, and the weld bridge in one container.
# InfluxDB is a separate Compose service. INGEST_URL and INGEST_API_KEY come from the environment.
set -u

if [[ -z "${INGEST_URL:-}" || -z "${INGEST_API_KEY:-}" ]]; then
  echo "weld_demo: INGEST_URL and INGEST_API_KEY must be set (cloud/.env)" >&2
  exit 1
fi

pids=()
cleanup() {
  local pid
  for pid in "${pids[@]:-}"; do
    kill "$pid" 2>/dev/null || true
  done
}
trap cleanup EXIT INT TERM

start() {
  "$@" &
  pids+=("$!")
}

start ros2 run cell_sim weld_cell_publisher --ros-args \
  -r __node:=weld_cell_07 \
  -p cell_id:=WELD_CELL_07 \
  -p period_sec:=1.0

start ros2 run cell_sim weld_cell_publisher --ros-args \
  -r __node:=weld_cell_08 \
  -p cell_id:=WELD_CELL_08 \
  -p period_sec:=1.0

start ros2 run cell_historian weld_historian_node --ros-args \
  -p influx_url:=http://influxdb:8086 \
  -p influx_org:=p3 \
  -p influx_bucket:=weld \
  -p influx_token:=p3-historian-dev-token

start ros2 run cell_bridge weld_cell_bridge_node --ros-args \
  -p "ingest_url:=${INGEST_URL}" \
  -p "api_key:=${INGEST_API_KEY}" \
  -p reduce:=every_n \
  -p every_n:=5

wait -n
cleanup
