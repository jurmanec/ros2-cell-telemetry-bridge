#!/usr/bin/env bash
# Build and run the disposable ROS2 rclcpp spike in Docker.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
cd "$ROOT"

IMAGE="${SPIKE_IMAGE:-p3-ros2-spike:humble}"

echo "==> Building $IMAGE"
docker build -t "$IMAGE" .

echo "==> Running cell_publisher (Ctrl+C to stop)"
docker run --rm --name p3-cell-publisher "$IMAGE"
