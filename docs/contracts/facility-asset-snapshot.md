# Contract — Facility asset snapshot

**Status:** In progress  
**Date:** 2026-11-12  
**Plan:** [facility-snapshot-plan.md](../v3/facility-snapshot-plan.md)  
**ROS type:** `cell_msgs/msg/FacilityAssetSnapshot`  
**Topic:** `facility/asset_snapshot` (relative; fully qualified `/facility/asset_snapshot` in the default namespace)

This contract does not replace [cell-telemetry.md](cell-telemetry.md) or [weld-cell-snapshot.md](weld-cell-snapshot.md). `schema_version` 1 and `schema_version` 2 stay as they are.

One snapshot is one asset. `site_id`, `hall_id`, and `asset_id` are copied from the message. The bridge has no asset parameter.

---

## Topic

| Item | Value |
|------|--------|
| Name | `facility/asset_snapshot` |
| Type | `cell_msgs/msg/FacilityAssetSnapshot` |
| QoS | Reliable, depth 10 (publisher default) |
| Rate (sim) | 1 Hz |
| Publisher | `facility_asset_publisher`, one process per asset |

Each process sets `site_id`, `hall_id`, `asset_id`, `asset_type`, `period_sec` (default 1.0), and `fault_at_tick` (default 0, which never faults). A second and third process remap the node name. Readings come from `asset_type`: `crah` is 16 °C until tick 180, then 28 °C with `COOLING_SUPPLY_HIGH`; `room` is 22 °C until tick 360, then 32 °C with `ROOM_TEMP_HIGH`; `ups` stays at 40 percent. A fault latches.

---

## ROS type

`FacilityAssetSnapshot.msg` lives under `edge/ws/src/cell_msgs/msg/`. Alarms reuse `CellAlarm.msg`.

| Field | Type | Meaning | Demo |
|-------|------|---------|------|
| `site_id` | `string` | Site that owns the hall | `SITE-01` |
| `hall_id` | `string` | Hall inside the site | `HALL-01` |
| `asset_id` | `string` | The unit this snapshot describes | `CRAH-01`, `ROOM-01`, `UPS-01` |
| `asset_type` | `string` | Kind of unit | `crah`, `room`, `ups` |
| `status` | `string` | `RUNNING` or `FAULTED` | `RUNNING` while `alarms` is empty |
| `reading` | `float64` | The one number for this asset | Supply-air °C, room °C, or UPS load percent |
| `unit` | `string` | Unit of `reading` | `C` or `pct` |
| `stamp` | `builtin_interfaces/Time` | Sample time | Publisher clock |
| `alarms` | `CellAlarm[]` | Zero or more alarms | Empty while `RUNNING` |

---

## HTTP body

Same path as [telemetry-batch-http.md](telemetry-batch-http.md): `POST /v1/telemetry/batch`. One snapshot per request. `schema_version` **3** distinguishes it from a v1 batch and a weld snapshot. The bridge formats `stamp` as one UTC timestamp.

Healthy cooling unit:

```json
{
  "schema_version": 3,
  "timestamp": "1970-01-01T00:00:00.111192632Z",
  "site_id": "SITE-01",
  "hall_id": "HALL-01",
  "asset_id": "CRAH-01",
  "asset_type": "crah",
  "status": "RUNNING",
  "reading": 16,
  "unit": "C",
  "alarms": []
}
```

Cooling fault:

```json
{
  "schema_version": 3,
  "timestamp": "1970-01-01T00:00:00.111192632Z",
  "site_id": "SITE-01",
  "hall_id": "HALL-01",
  "asset_id": "CRAH-01",
  "asset_type": "crah",
  "status": "FAULTED",
  "reading": 28,
  "unit": "C",
  "alarms": [
    {
      "code": "COOLING_SUPPLY_HIGH",
      "severity": "warning",
      "message": "Supply air above setpoint"
    }
  ]
}
```

| Field | Type | Notes |
|-------|------|--------|
| `schema_version` | number | `3` |
| `timestamp` | string | UTC from ROS `stamp`, with nine fractional digits. Ingest stores this string as the DynamoDB sort key, so a second post of the same asset and timestamp replaces the item. |
| `asset_id` | string | Copied from the snapshot. Cloud partition key |
| `alarms` | array | Empty when `status` is `RUNNING`. Objects use `code`, `severity`, `message` |

The cloud sort key is time-ordered, as on the weld items. A collector can count `status` for one `asset_id` over a time range. This body does not carry a runbook link or a team.

---

## Historian

`facility_historian_node` writes every snapshot to the InfluxDB bucket `facility`, measurement `facility_asset`. It drops an empty `asset_id`. It does not filter and it does not POST. Tags are `site_id`, `hall_id`, `asset_id`, `asset_type`, and `status`. The reading is a field. Schema: [influxdb-facility-schema.md](../edge/influxdb-facility-schema.md).

---

## Bridge

`facility_bridge_node` subscribes to the same topic and POSTs this body. It copies `asset_id` from the message. The reduction rule is the weld rule in [weld-bridge-reduction.md](weld-bridge-reduction.md), with `asset_id` in place of `cell_id`. The demo uses `reduce=every_n` and `every_n` 5, so each asset is posted on its own fifth snapshot, and any snapshot with an alarm is posted as well. An empty `asset_id` is dropped. Ingest stores the item and does not emit a CloudWatch alarm for this body.

---

## Compatibility

- `CellTelemetry` / `schema_version` 1 is unchanged.
- `WeldCellSnapshot` / `schema_version` 2 is unchanged.
