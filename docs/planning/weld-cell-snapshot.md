# Plan — Welding-cell snapshot telemetry

**Status:** Done  
**Date:** 2026-11-06  
**Follows:** v1 cell telemetry (`CellTelemetry` on `cell/telemetry`, `schema_version` 1)

This plan adds one realistic fixed-cell publisher before any mobile-robot or fleet payload. The source shape is the robotic welding snapshot in [fixed_cell_welding_detail.md](../concepts/cell_details/fixed_cell_welding_detail.md) and section 1.1 of [cell_and_fleet_telemetry_samples.md](../concepts/cell_and_fleet_telemetry_samples.md).

v1 stays runnable until the new topic is visible and, later, until a version-2 ingest path stores and alerts. Do not change `schema_version` 1 in place.

---

## Goal

A simulator publishes a welding-cell status snapshot on a ROS topic. `weld_cell_bridge_node` turns that snapshot into one JSON document and POSTs it. Cloud ingest stores it and raises `CELL_ALERT` only when the snapshot carries an alarm.

Publisher, bridge, and ingest are in place. A version-2 snapshot is stored as its own DynamoDB item. `CELL_ALERT` is logged only when that snapshot carries an alarm. Version-1 batches still return 202. The deployed API accepts version 2 after the next `serverless deploy`.

---

## Snapshot

| Field | Notes |
|-------|--------|
| `cell_id` | Publisher parameter. Default `WELD_CELL_07` |
| `cell_type` | `robotic_welding` |
| `status` | `RUNNING`, or `FAULTED` on the occasional alarm sample |
| `program` | `DOOR_FRAME_WELD_V4` |
| `stamp` | ROS `builtin_interfaces/Time` (`sec`, `nanosec`). The bridge later formats one UTC string |
| `cycle_time_last_s` | Duration of the last finished weld cycle |
| `parts_total_shift` | Increments once per completed cycle |
| `parts_scrap_shift` | Increments rarely |
| `oee_availability` | Simple ratio from the simulator, not a real OEE calculation |
| `joints` | Three axes. Each has `axis`, `temp_c`, `current_a`. Axis 2 runs hottest |
| `process` | `weld_current_a`, `weld_voltage_v`, `wire_feed_m_min`, `arc_on_s`, `gas_flow_l_min` |
| `alarms` | Empty while running. Occasionally one alarm, for example `GAS_FLOW_LOW` / `warning` |

Topic: `cell/weld_snapshot`. One snapshot about once a second. That is a cell-status sample, not a high-rate joint stream.

Healthy process values stay near the example: 210 A, 23.1 V, 7.4 m/min wire, 15 L/min gas. Joint current is high while the arc is on and near zero between cycles. `arc_on_s` climbs during the arc and resets when the cycle completes.

---

## Steps

### 1. Contract — done

[weld-cell-snapshot.md](../contracts/weld-cell-snapshot.md) names topic `cell/weld_snapshot`, the ROS types, and the `schema_version` 2 JSON body. `cell_id` is a publisher parameter (default `WELD_CELL_07`).

### 2. ROS messages — done

Under `edge/ws/src/cell_msgs/msg/`:

| Message | Fields |
|---------|--------|
| Joint sample | `axis`, `temp_c`, `current_a` |
| Weld process | `weld_current_a`, `weld_voltage_v`, `wire_feed_m_min`, `arc_on_s`, `gas_flow_l_min` |
| Alarm | `code`, `severity`, `message` |
| `WeldCellSnapshot` | The snapshot table above, with `joints[]`, one process, and `alarms[]` |

An empty `alarms` array is the normal running sample.

### 3. Simulator — done

`weld_cell_publisher` is separate from `cell_publisher`. Defaults: `cell_id:=WELD_CELL_07`, `period_sec:=1.0`. Compose starts it only with `--profile weld` and passes `cell_id` on this service. A 42-tick cycle keeps the arc on for 20 ticks. Every 15th completed cycle publishes one `GAS_FLOW_LOW` warning and `status=FAULTED`.

### 4. Prove the publisher — done

- Unit test (`test_weld_snapshot`): running sample, arc-off current, completed cycle, and the gas-flow alarm.
- `ros2 topic echo /cell/weld_snapshot` inside the publisher container showed `cell_id: WELD_CELL_07`, `status: RUNNING`, empty `alarms`, and `arc_on_s` climbing 3, 4, 5, 6, 7. Echo ran in that container because a second container on Docker Desktop does not reliably discover DDS peers.

### 5. Bridge — done

`weld_cell_bridge_node` subscribes to `cell/weld_snapshot`, copies `cell_id` from the message, formats the ROS stamp as one UTC timestamp, and POSTs one `schema_version` 2 body per snapshot. It has no `cell_id` parameter. Compose starts it with `--profile weld`, together with the publisher and the local stub.

### 6. Ingest — done

`schema_version` 2 is accepted beside the v1 validator. One DynamoDB item is written per snapshot: `pk` = `cell_id`, nested `joints`, `process`, and `alarms`. One `CELL_ALERT` is logged per alarm; a running snapshot with an empty `alarms` array logs none. Version-1 batches still return 202. The local stub accepts the same body so the weld bridge gets 202. The live API needs a redeploy before it does.

---

Follow-on: [v2-architecture.md](v2-architecture.md) (historian, configurable weld-bridge reduction, several cells).

## Out of this plan

- AMR, AGV, and fleet-summary payloads
- MQTT or OPC UA
- Replacing or removing `CellTelemetry` / `schema_version` 1
- A full OEE model, seam tracking, or a 6-axis high-rate joint stream
