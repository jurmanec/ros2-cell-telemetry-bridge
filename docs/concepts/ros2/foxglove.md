# Foxglove from a Docker container

How to open the ROS 2 graph in Foxglove Studio when the nodes run inside Docker on a Mac. This is a live view of the topics. InfluxDB, DynamoDB, and CloudWatch remain the record of what was stored.

Foxglove on the host does not join the container's DDS graph. Docker Desktop does not reliably discover ROS 2 peers across that boundary, so a native ROS 2 connection from the Mac app sees an empty graph. The path that works is `foxglove_bridge` inside the same container as the publishers, with its WebSocket published to the host.

---

## 1. Run the bridge in the node container

`foxglove_bridge` is a ROS 2 node. It subscribes to the topics in its own process and serves them on a WebSocket. It has to share DDS with the publishers.

For the weld demo that container is `weld_demo`. Both cells, the historian, and the weld bridge already run there, on `/cell/weld_snapshot`. Start `foxglove_bridge` in that same container. A second container for the bridge hits the same discovery limit as Foxglove on the host. The InfluxDB container has no ROS graph.

```bash
ros2 launch foxglove_bridge foxglove_bridge_launch.xml port:=8765
```

The server binds `0.0.0.0:8765` by default, which is what Docker port publishing needs. `127.0.0.1` inside the container would accept connections only from that container.

---

## 2. Install the package in the edge image

`p3-edge:jazzy` is `ros:jazzy-ros-base` plus this repo's packages. It does not include Foxglove. Add the Jazzy apt package in `edge/docker/Dockerfile`:

```dockerfile
ros-jazzy-foxglove-bridge
```

Then rebuild with `edge/scripts/build.sh`. Compose does not build `p3-edge:jazzy` when the demo starts. An image from before this package was added will fail at `ros2 launch` with a missing package.

The demo script is mounted at runtime, so adding the launch line to `edge/scripts/run_weld_demo.sh` does not by itself require a rebuild. The apt package does.

---

## 3. Publish port 8765

`weld_demo` currently publishes no ports. Map the bridge port on that service:

```yaml
ports:
  - "8765:8765"
```

The host then reaches the server at `ws://localhost:8765`. InfluxDB's `8086` mapping is the same idea for a different service. Publishing `8765` on `influxdb` would not reach the ROS node.

In Foxglove Studio, open a Foxglove WebSocket connection to `ws://localhost:8765`. The desktop app and the browser app both use that URL for a bridge on this machine.

---

## 4. What the weld topics look like

The bridge forwards the live ROS graph. For this demo that is:

| Panel | What it shows |
|-------|----------------|
| Connection graph | `/weld_cell_07`, `/weld_cell_08`, `/weld_historian`, `/weld_cell_bridge`, and the Foxglove bridge node |
| Raw messages | `cell/weld_snapshot`, alternating `cell_id` `WELD_CELL_07` and `WELD_CELL_08` |
| Plot | Numeric fields such as `process.weld_current_a`, `process.gas_flow_l_min`, and the joint temperatures. Split or filter by `cell_id` |

Two publishers at one snapshot per second make the topic about 2 Hz. `gas_flow_l_min` drops and `status` becomes `FAULTED` on a `GAS_FLOW_LOW` snapshot.

These messages are custom cell telemetry. There is no TF tree, URDF, or `sensor_msgs/JointState`, so the 3D panel has nothing to draw.

A shell in the same container can list the same graph with `ros2 node list` and `ros2 topic echo` after sourcing `/opt/ros/jazzy/setup.bash` and `/ws/install/setup.bash`. `docker exec` does not run the image entrypoint, so that shell does not have ROS on `PATH` until those two files are sourced. That CLI stays inside the container. Foxglove is the view on the host.

---

## Related

- Foxglove ROS 2 connection: [docs.foxglove.dev](https://docs.foxglove.dev/docs/getting-started/frameworks/ros2)
- Demo that owns the node container: [weld-demo.md](../../how-to/weld-demo.md)
- Why a second container misses DDS peers: [weld-cell-snapshot.md](../../planning/weld-cell-snapshot.md)
