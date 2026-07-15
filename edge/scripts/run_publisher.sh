#!/usr/bin/env bash
# Run cell_publisher (foreground). Docker Desktop must be running.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
IMAGE="${EDGE_IMAGE:-p3-edge:jazzy}"
NET="${EDGE_NET:-p3-edge-net}"

if ! docker image inspect "$IMAGE" >/dev/null 2>&1; then
  "$ROOT/scripts/build.sh"
fi

docker network inspect "$NET" >/dev/null 2>&1 || docker network create "$NET"

echo "==> Running cell_publisher on $NET (Ctrl+C to stop)"
docker run --rm --name p3-cell-publisher --network "$NET" "$IMAGE"
