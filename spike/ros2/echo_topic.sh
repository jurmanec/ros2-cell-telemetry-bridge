#!/usr/bin/env bash
# Echo /cell/telemetry from a second container on the same Docker network.
# Usage: start publisher with network, then this script.
set -euo pipefail

IMAGE="${SPIKE_IMAGE:-p3-ros2-spike:humble}"
NET="${SPIKE_NET:-p3-spike-net}"

docker network inspect "$NET" >/dev/null 2>&1 || docker network create "$NET"

# Ensure publisher is running on the network (start if needed)
if ! docker ps --format '{{.Names}}' | grep -q '^p3-cell-publisher$'; then
  echo "==> Starting publisher on $NET"
  docker run -d --rm --name p3-cell-publisher --network "$NET" "$IMAGE"
  sleep 2
fi

echo "==> Echoing /cell/telemetry (10 messages)"
docker run --rm --network "$NET" "$IMAGE" \
  bash -c 'source /opt/ros/humble/setup.bash && source /ws/install/setup.bash && ros2 topic echo /cell/telemetry --once'
# Print a few more
docker run --rm --network "$NET" "$IMAGE" \
  bash -c 'source /opt/ros/humble/setup.bash && source /ws/install/setup.bash && timeout 5 ros2 topic echo /cell/telemetry || true'
