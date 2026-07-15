# ROS 2 / rclcpp discovery spike

**Status:** Complete  
**Date:** 2026-07-13  
**Goal:** Prove a C++ (`rclcpp`) node can publish on this developer machine; record OS/distro constraints. No cloud.

Disposable code: [`spike/ros2/`](../../spike/ros2/).

---

## Environment (host)

| Item | Value |
|------|--------|
| Host OS | macOS 15.4.1 (Darwin 24.4.0) |
| Arch | arm64 (Apple Silicon) |
| Native `ros2` / `colcon` | Not installed |
| Docker | Desktop 29.4.0 (`aarch64` Linux VM) |
| Base image | `ros:humble-ros-base` (not `osrf/ros:…`; that tag 404’d) |
| Spike image | `p3-ros2-spike:humble` |

**Spike decision:** Edge path works via **Docker (Linux) + `rclcpp`**. Do not chase native macOS ROS2 for v1.

**v1 distro lock:** **Jazzy** (developer prior exploratory experience; LTS through May 2029). Humble was used only to prove the Docker path; disposable spike image stays on Humble unless re-verified on Jazzy.

---

## Success criteria

| Check | Result |
|-------|--------|
| Docker daemon | OK (must be started manually) |
| Image pull/build | OK — `colcon build` of `cell_telemetry_spike` |
| Publisher runs | OK — logs `cycle=… temp_c=… vibration=… fault=…` |
| Topic echo | OK — `/cell/telemetry` seen from second container on user-defined bridge network |
| Distro used in spike | Humble |
| Distro locked for v1 | **Jazzy** |

---

## How to re-run

```bash
# Docker Desktop must be running
cd spike/ros2
./run_publisher.sh          # foreground publisher
# other terminal:
./echo_topic.sh             # network + echo (starts publisher if needed)
```

Manual two-container check (what was verified):

```bash
docker network create p3-spike-net   # once
docker run -d --name p3-cell-publisher --network p3-spike-net p3-ros2-spike:humble
docker run --rm --network p3-spike-net p3-ros2-spike:humble \
  bash -c 'source /opt/ros/humble/setup.bash && source /ws/install/setup.bash && ros2 topic echo /cell/telemetry --once'
docker rm -f p3-cell-publisher
```

---

## Notes / caveats

- **DDS across containers** needs a shared Docker network (default bridge isolation otherwise).
- Host has no native ROS tooling; all ROS CLI runs inside containers.
- Spike uses `std_msgs/String` payload; custom messages are fine later for v1.
- `osrf/ros:humble-ros-base` was **not found** on Docker Hub at spike time; use `ros:humble-ros-base`.

---

## Implications for architecture / v1

- Dev loop: edit on Mac → build/run ROS nodes in Docker (**Jazzy** / arm64).
- Bridge and cloud can develop on the host; only the ROS graph requires Linux.
- Scaffold should use `ros:jazzy-ros-base` (or equivalent), not the Humble spike image.
- Spike unblocks: architecture one-pager + v1 milestone list (charter next steps 2–3).
