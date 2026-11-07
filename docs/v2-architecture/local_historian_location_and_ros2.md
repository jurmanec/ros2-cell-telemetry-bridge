# Local Historian: Physical Location and ROS 2 Integration

**Yes — that model is realistic.**

## Physical Location

The local historian is an **on-site machine** (or a small cluster of machines) located in the plant, typically:

- In a local server room or edge rack near the cells
- On an industrial PC or server on the same network as the cells
- Sometimes as a dedicated appliance or virtual machine running on plant infrastructure

It is **not** in the cloud. Its job is to stay available even if the WAN or cloud link is down.

## How It Fits with ROS 2

In a ROS 2-based cell, the historian commonly works like this:

```
Welding cell ROS 2 node
    │
    ├── publishes /cell/weld_snapshot
    ├── publishes /cell/telemetry
    └── publishes /cell/status, /cell/alarms, etc.
            │
            ▼
Local Historian (ROS 2 node or bridge)
    │
    ├── subscribes to the relevant topics
    └── writes every message into a local time-series database
            (InfluxDB, TimescaleDB, PI, Canary, etc.)
```

## Typical Behavior

- The historian **subscribes** to the cell topics (`cell/telemetry`, `cell/weld_snapshot`, joint data, alarms, etc.).
- It records **all** (or nearly all) messages at the local rate (e.g., 1 Hz) for a retention period of days to months.
- It does **not** normally forward everything upstream; that reduction happens later at the bridge/edge gateway.
- Operators and engineers can query or trend the data locally even when the cloud is unreachable.

## Summary

Yes — a local historian is usually a dedicated on-site machine (or service) that subscribes to the cell’s ROS 2 topics (such as `cell/weld_snapshot` and `cell/telemetry`) and keeps a full local record of the telemetry. The bridge then reads from the cell topics or from the historian and decides what reduced stream goes to the cloud.
