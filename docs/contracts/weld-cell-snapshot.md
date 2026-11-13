# Contract — Welding-cell snapshot

**Status:** Proposed  
**Date:** 2026-11-06  
**Plan:** [weld-cell-snapshot.md](../planning/weld-cell-snapshot.md)  
**ROS type:** `cell_msgs/msg/WeldCellSnapshot`  
**Topic:** `cell/weld_snapshot` (relative; fully qualified `/cell/weld_snapshot` in the default namespace)  
**Source shape:** [fixed_cell_welding_detail.md](../concepts/cell_details/fixed_cell_welding_detail.md)

This contract does not replace [cell-telemetry.md](cell-telemetry.md). `CellTelemetry` on `cell/telemetry` and HTTP `schema_version` 1 stay as they are.

---

## Purpose

Wire shape for a simulated robotic welding cell: one status snapshot per publish, from the edge publisher through the weld bridge to version-2 ingest.

`cell_id` is set by the publisher (ROS parameter, default `WELD_CELL_07`). The bridge copies it from the message and does not take a `cell_id` parameter.

---

## Topic

| Item | Value |
|------|--------|
| Name | `cell/weld_snapshot` |
| Type | `cell_msgs/msg/WeldCellSnapshot` |
| QoS | Reliable, depth 10 (publisher default) |
| Rate (sim) | About 1 Hz. A cell-status snapshot, not a high-rate joint stream |

---

## ROS types

Messages live under `edge/ws/src/cell_msgs/msg/`.

### `WeldJoint.msg`

| Field | Type | Meaning | Units / notes |
|-------|------|---------|---------------|
| `axis` | `uint8` | Joint index | 1, 2, or 3 in v1 of this snapshot. Axis 2 runs hottest |
| `temp_c` | `float64` | Joint temperature | °C |
| `current_a` | `float64` | Joint current | A. High while the arc is on; near zero between cycles |

### `WeldProcess.msg`

| Field | Type | Meaning | Units / notes |
|-------|------|---------|---------------|
| `weld_current_a` | `float64` | Arc current | A. Healthy samples stay near 210 |
| `weld_voltage_v` | `float64` | Arc voltage | V. Healthy samples stay near 23.1 |
| `wire_feed_m_min` | `float64` | Wire-feed speed | m/min. Healthy samples stay near 7.4 |
| `arc_on_s` | `float64` | Arc-on time in the current cycle | s. Climbs during the arc, then resets |
| `gas_flow_l_min` | `float64` | Shielding-gas flow | L/min. Healthy samples stay near 15 |

### `CellAlarm.msg`

| Field | Type | Meaning | Units / notes |
|-------|------|---------|---------------|
| `code` | `string` | Alarm identifier | Example: `GAS_FLOW_LOW` |
| `severity` | `string` | `info`, `warning`, or `error` | The occasional demo alarm is `warning` |
| `message` | `string` | Short human-readable text | |

### `WeldCellSnapshot.msg`

| Field | Type | Meaning | Units / notes |
|-------|------|---------|---------------|
| `cell_id` | `string` | Identity of the publishing cell | Publisher parameter; default `WELD_CELL_07` |
| `cell_type` | `string` | Kind of cell | `robotic_welding` |
| `status` | `string` | `RUNNING` or `FAULTED` | `FAULTED` only on a sample that carries an alarm |
| `program` | `string` | Active robot program | `DOOR_FRAME_WELD_V4` |
| `stamp` | `builtin_interfaces/Time` | Sample time (node clock when published) | ROS time (`sec`, `nanosec`) |
| `cycle_time_last_s` | `float64` | Duration of the last finished weld cycle | s |
| `parts_total_shift` | `uint32` | Parts completed this shift | Increments once per completed cycle |
| `parts_scrap_shift` | `uint32` | Scrapped parts this shift | Increments rarely |
| `oee_availability` | `float64` | Availability figure from the simulator | 0–1. A simple ratio, not a plant OEE model |
| `joints` | `WeldJoint[]` | Joint samples | Three elements |
| `process` | `WeldProcess` | Weld process block | |
| `alarms` | `CellAlarm[]` | Active alarms | Empty while `RUNNING` |

---

## Example (`ros2 topic echo /cell/weld_snapshot`)

```yaml
cell_id: WELD_CELL_07
cell_type: robotic_welding
status: RUNNING
program: DOOR_FRAME_WELD_V4
stamp:
  sec: 1783967375
  nanosec: 11192632
cycle_time_last_s: 42.1
parts_total_shift: 156
parts_scrap_shift: 3
oee_availability: 0.91
joints:
  - axis: 1
    temp_c: 49.2
    current_a: 4.1
  - axis: 2
    temp_c: 56.8
    current_a: 6.3
  - axis: 3
    temp_c: 51.4
    current_a: 4.7
process:
  weld_current_a: 210.0
  weld_voltage_v: 23.1
  wire_feed_m_min: 7.4
  arc_on_s: 19.6
  gas_flow_l_min: 15.0
alarms: []
---
```

---

## HTTP body

Same path as [telemetry-batch-http.md](telemetry-batch-http.md): `POST /v1/telemetry/batch`. One snapshot per request. `schema_version` **2** distinguishes it from a v1 batch. The bridge formats `stamp` as one UTC timestamp. Ingest stores one DynamoDB item (`pk` = `cell_id`) and emits `CELL_ALERT` once per alarm. A snapshot with an empty `alarms` array stores the item and emits no alert. Version-1 batches are unchanged.

```json
{
  "schema_version": 2,
  "timestamp": "2026-11-06T17:05:22.011192632Z",
  "cell_id": "WELD_CELL_07",
  "cell_type": "robotic_welding",
  "status": "RUNNING",
  "program": "DOOR_FRAME_WELD_V4",
  "cycle_time_last_s": 42.1,
  "parts_total_shift": 156,
  "parts_scrap_shift": 3,
  "oee_availability": 0.91,
  "joints": [
    {"axis": 1, "temp_c": 49.2, "current_a": 4.1},
    {"axis": 2, "temp_c": 56.8, "current_a": 6.3},
    {"axis": 3, "temp_c": 51.4, "current_a": 4.7}
  ],
  "process": {
    "weld_current_a": 210,
    "weld_voltage_v": 23.1,
    "wire_feed_m_min": 7.4,
    "arc_on_s": 19.6,
    "gas_flow_l_min": 15.0
  },
  "alarms": []
}
```

| Field | Type | Notes |
|-------|------|--------|
| `schema_version` | number | `2` for this snapshot |
| `timestamp` | string | UTC RFC 3339 from ROS `stamp`, with nine fractional digits. Ingest stores this string as the DynamoDB sort key, so a second post of the same cell and timestamp replaces the item. |
| `cell_id` | string | Copied from `WeldCellSnapshot.cell_id` |
| `joints` | array | One object per `WeldJoint` |
| `process` | object | The `WeldProcess` fields |
| `alarms` | array | Empty when `status` is `RUNNING`. Objects use `code`, `severity`, `message` |

A faulted sample sets `status` to `FAULTED` and includes one alarm, for example `{"code":"GAS_FLOW_LOW","severity":"warning","message":"Shielding gas flow below setpoint"}`.

---

## Compatibility

- Which snapshots the bridge POSTs is [weld-bridge-reduction.md](weld-bridge-reduction.md). The default policy forwards every snapshot.
- v1 `CellTelemetry` and `schema_version` 1 are unchanged.
- This snapshot is a cell-status document. It is not a 6-axis stream, a seam tracker, or a plant OEE model.
- AMR, AGV, and fleet-summary payloads are a later contract.
