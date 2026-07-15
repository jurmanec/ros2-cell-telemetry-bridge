#!/usr/bin/env bash
# Build the P3 edge Docker image (ROS 2 Jazzy + cell_sim).
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

IMAGE="${EDGE_IMAGE:-p3-edge:jazzy}"

echo "==> Building $IMAGE"
docker build -f docker/Dockerfile -t "$IMAGE" .
echo "==> Done: $IMAGE"
