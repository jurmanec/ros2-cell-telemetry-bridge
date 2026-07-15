#!/usr/bin/env bash
# Echo /cell/telemetry via docker exec into the publisher container.
# A second container on a Docker network does not reliably discover DDS
# peers on Docker Desktop; same-container exec matches the manual Option A path.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
IMAGE="${EDGE_IMAGE:-p3-edge:jazzy}"
NET="${EDGE_NET:-p3-edge-net}"

if ! docker image inspect "$IMAGE" >/dev/null 2>&1; then
  "$ROOT/scripts/build.sh"
fi

docker network inspect "$NET" >/dev/null 2>&1 || docker network create "$NET"

if ! docker ps --format '{{.Names}}' | grep -q '^p3-cell-publisher$'; then
  echo "==> Starting publisher on $NET"
  docker rm -f p3-cell-publisher >/dev/null 2>&1 || true
  docker run -d --name p3-cell-publisher --network "$NET" "$IMAGE"
  sleep 2
fi

echo "==> Echoing /cell/telemetry (once) via docker exec"
docker exec p3-cell-publisher bash -c \
  'source /opt/ros/jazzy/setup.bash && source /ws/install/setup.bash && timeout 15 ros2 topic echo /cell/telemetry --once'
