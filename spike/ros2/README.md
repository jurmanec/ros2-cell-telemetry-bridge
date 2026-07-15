# Disposable ROS 2 spike (rclcpp)

Throwaway workspace to prove **C++ publisher in Docker** on this Mac (built on **Humble**). **v1 targets Jazzy** — see [docs/spikes/ros2-rclcpp.md](../../docs/spikes/ros2-rclcpp.md).

## Prerequisites

1. **Docker Desktop running** (whale icon in menu bar; `docker info` succeeds).
2. From this directory:

```bash
chmod +x *.sh
./run_publisher.sh
```

Topic check (separate terminal):

```bash
./echo_topic.sh
```

## Layout

```
src/cell_telemetry_spike/   # ament_cmake + rclcpp node
Dockerfile                  # ros:humble-ros-base + build
```
