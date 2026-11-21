#!/bin/bash
# Three facility assets, the local historian, and the facility bridge in one container.
# InfluxDB is a separate Compose service. INGEST_URL and INGEST_API_KEY come from the environment.
# SCENARIO selects /scenarios/<id>.yaml. Unset, it is 002.
set -u

if [[ -z "${INGEST_URL:-}" || -z "${INGEST_API_KEY:-}" ]]; then
  echo "facility_demo: INGEST_URL and INGEST_API_KEY must be set (cloud/.env)" >&2
  exit 1
fi

scenario="${SCENARIO:-002}"
params="/scenarios/${scenario}.yaml"
if [[ ! -f "${params}" ]]; then
  echo "facility_demo: scenario file not found: ${params}" >&2
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

start ros2 run cell_sim facility_asset_publisher --ros-args \
  -r __node:=facility_crah_01 \
  --params-file "${params}"

start ros2 run cell_sim facility_asset_publisher --ros-args \
  -r __node:=facility_room_01 \
  --params-file "${params}"

start ros2 run cell_sim facility_asset_publisher --ros-args \
  -r __node:=facility_ups_01 \
  --params-file "${params}"

start ros2 run cell_historian facility_historian_node --ros-args \
  -p influx_url:=http://influxdb:8086 \
  -p influx_org:=p3 \
  -p influx_bucket:=facility \
  -p influx_token:=p3-historian-dev-token

start ros2 run cell_bridge facility_bridge_node --ros-args \
  -p "ingest_url:=${INGEST_URL}" \
  -p "api_key:=${INGEST_API_KEY}" \
  -p reduce:=every_n \
  -p every_n:=5

wait -n
cleanup
