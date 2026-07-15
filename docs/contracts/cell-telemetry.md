# Contract — Cell telemetry

**Status:** Active (M1)  
**Date:** 2026-07-13  
**ROS type:** `cell_msgs/msg/CellTelemetry`  
**Topic:** `cell/telemetry` (relative; fully qualified `/cell/telemetry` in default namespace)

---

## Purpose

Wire shape for simulated manufacturing-cell health samples from the edge publisher to the future bridge (M2) and cloud rules (M3).

---

## Topic

| Item | Value |
|------|--------|
| Name | `cell/telemetry` |
| Type | `cell_msgs/msg/CellTelemetry` |
| QoS (v1) | Reliable, depth 10 (publisher default) |
| Rate (sim) | ~2 Hz (500 ms timer); not part of the contract |

---

## Message fields

| Field | Type | Meaning | Units / notes |
|-------|------|---------|---------------|
| `stamp` | `builtin_interfaces/Time` | Sample time (node clock when published) | ROS time |
| `cycle` | `uint64` | Monotonic simulated cycle count | Starts at 1 on first publish |
| `temp_c` | `float64` | Simulated cell temperature | °C |
| `vibration` | `float64` | Simulated vibration magnitude | Arbitrary sim units (dimensionless for v1) |
| `fault` | `bool` | Fault flag for rule demos | `true` = unhealthy |

Source: [`edge/ws/src/cell_msgs/msg/CellTelemetry.msg`](../../edge/ws/src/cell_msgs/msg/CellTelemetry.msg)

---

## Example (`ros2 topic echo /cell/telemetry`)

```yaml
stamp:
  sec: ...
  nanosec: ...
cycle: 12
temp_c: 46.0
vibration: 0.2
fault: false
---
```

---

## Compatibility notes

- M0 used `std_msgs/String` key=value text; **superseded** by this message for M1+.
- Field set is intentionally small; add fields via new msg version / ADR if needed—do not silently reinterpret existing fields.
- Bridge/cloud (M2+) should treat unknown future fields as non-breaking only if using a versioned envelope (not required in v1).

---

## Test expectations

- Unit tests construct/fill `CellTelemetry` via `cell_sim` helpers (see `cell_sim` tests).
- Smoke: publisher runs; `ros2 topic echo /cell/telemetry --once` shows structured fields (not a free-form string).
