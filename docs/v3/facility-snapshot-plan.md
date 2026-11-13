# Plan — Facility asset snapshots

**Status:** Step 5 done.  
**Date:** 2026-11-12  
**Follows:** [facility-telemetry.md](facility-telemetry.md), [building-management-system.md](building-management-system.md)  
**Frozen:** [v2.0.0](https://github.com/jurmanec/ros2-cell-telemetry-bridge/releases/tag/v2.0.0). Do not change `cell/telemetry`, `schema_version` 1, `cell/weld_snapshot`, or `schema_version` 2.

ROS 2 publishers stand in for a building-management system. Each asset publishes one snapshot per second. A site historian stores every snapshot. A bridge posts a reduced copy. This plan stops at that cloud copy. It does not add a monitor, a page, or a shift.

---

## Contract

| Item | Value |
|------|--------|
| Topic | `facility/asset_snapshot` |
| ROS type | `cell_msgs/msg/FacilityAssetSnapshot` |
| Rate | 1 Hz |
| Cloud body | `schema_version` 3, one snapshot per POST |
| Identity | `asset_id` on the message. The bridge has no asset parameter |

`site_id`, `hall_id`, and `asset_id` are tags. `asset_type` is a tag with three values in this demo: `crah`, `room`, `ups`. The reading is one number. Alarms reuse `CellAlarm`.

| Field | Type | Notes |
|-------|------|--------|
| `site_id` | string | Demo value `SITE-01` |
| `hall_id` | string | Demo value `HALL-01` |
| `asset_id` | string | `CRAH-01`, `ROOM-01`, or `UPS-01` |
| `asset_type` | string | `crah`, `room`, or `ups` |
| `status` | string | `RUNNING` or `FAULTED` |
| `reading` | float64 | Supply-air °C, room °C, or UPS load percent |
| `unit` | string | `C` or `pct` |
| `stamp` | time | ROS stamp. The cloud body uses one UTC timestamp |
| `alarms` | `CellAlarm[]` | Empty while `RUNNING` |

Demo setpoints while healthy:

| Asset | Reading | Unit |
|-------|---------|------|
| `CRAH-01` | 16 | `C` |
| `ROOM-01` | 22 | `C` |
| `UPS-01` | 40 | `pct` |

Scripted faults, counted from each publisher's first tick. The processes start together. They do not subscribe to each other.

| Asset | Tick | Status | Reading | Alarm code | Message |
|-------|------|--------|---------|------------|---------|
| `CRAH-01` | 180 | `FAULTED` | 28 | `COOLING_SUPPLY_HIGH` | Supply air above setpoint |
| `ROOM-01` | 360 | `FAULTED` | 32 | `ROOM_TEMP_HIGH` | Hall temperature above limit |
| `UPS-01` | never | `RUNNING` | 40 | — | — |

Alarm `severity` is `warning`. At 1 Hz the cooling fault is at 3 minutes and the room fault is at 6 minutes.

The cloud item uses `asset_id` as the partition key and a time-ordered sort key. A collector can count `status` for one asset over a time range. This repo does not name that collector. The runbook and the team are not fields on the snapshot.

InfluxDB uses the existing org and a new bucket `facility`, retention 7 days. The measurement is `facility_asset`. The first write creates the measurement. The weld bucket is not reused.

---

## Path

```text
facility_asset_publisher  (one process per asset, 1 Hz)
        │
        ▼
facility/asset_snapshot
        ├──► facility historian  →  InfluxDB bucket facility  (every snapshot)
        └──► facility bridge     →  POST schema_version 3
                                        every 5th snapshot for that asset_id
                                        plus any snapshot with an alarm
                                        ▼
                                   DynamoDB
```

The bridge copies `asset_id` from the message. The downsample count is per `asset_id`. An empty `asset_id` is dropped.

---

## Steps

### 1. Message and HTTP body

Add `FacilityAssetSnapshot.msg`. Document the topic, the fields above, and the `schema_version` 3 JSON body.

Done when a unit test locks one healthy `CRAH-01` body and one `COOLING_SUPPLY_HIGH` body. That test is `test_facility_snapshot_json`, and it passed in the `p3-edge:jazzy` image build. The body is documented in [facility-asset-snapshot.md](../contracts/facility-asset-snapshot.md).

### 2. Publisher

One node, `facility_asset_publisher`. Parameters: `site_id`, `hall_id`, `asset_id`, `asset_type`, `period_sec` (default 1.0), `fault_at_tick` (default 0, which disables the fault). A second and third process remap the node name.

Done when three processes publish `CRAH-01`, `ROOM-01`, and `UPS-01` on `facility/asset_snapshot`. `test_facility_snapshot` locks the healthy readings and both fault ticks. A one-container run showed three publishers on that topic: `CRAH-01` at 16 °C, `ROOM-01` at 22 °C, and `UPS-01` at 40 percent, each `RUNNING`.

### 3. Historian

A subscriber writes every snapshot to bucket `facility`. It does not filter and it does not POST.

Done when a one-hour query returns about one point per second for each of the three asset ids, including ticks the bridge will skip. `test_facility_line` locks a healthy `CRAH-01` line and a `COOLING_SUPPLY_HIGH` line. A 19-second run at 1 Hz wrote 19 `reading` points for each of `CRAH-01`, `ROOM-01`, and `UPS-01`. That is every snapshot, not every fifth.

### 4. Bridge

Subscribe to the same topic. Reuse the weld reduction rule with `every_n` 5, keyed by `asset_id`. POST the version-3 body. Ingest stores the item. This plan does not use a CloudWatch check as proof.

Done when, over the same window, DynamoDB holds about one item per five snapshots for each asset, plus the cooling item and the room item. A 15-second run at 1 Hz with no faults posted 3 items for each asset and skipped the other 12. The table also holds `CRAH-01` at reading 28 with `COOLING_SUPPLY_HIGH`, `ROOM-01` at reading 32 with `ROOM_TEMP_HIGH`, and no faulted `UPS-01` item. After a fault latches, every later snapshot is posted because it carries an alarm.

### 5. Demo

One Compose command on a new profile starts InfluxDB, the three publishers, the historian, and the bridge. The publishers and subscribers share a container.

Done when the cooling snapshot is `FAULTED` at tick 180 in both stores, the room snapshot is `FAULTED` at tick 360 in both stores, and `UPS-01` has no alarm in either store. The command is `docker compose --profile facility up facility_demo` from `edge/`. See [facility-demo.md](../how-to/facility-demo.md). A run started at 21:42:28Z wrote `CRAH-01` reading 28, `FAULTED`, `COOLING_SUPPLY_HIGH` at 21:45:28Z in both InfluxDB and DynamoDB, and `ROOM-01` reading 32, `FAULTED`, `ROOM_TEMP_HIGH` at 21:48:28Z in both. `UPS-01` had no faulted item in that window.

---

## Out of this plan

- The software desk, hot hosts, and lost server capacity
- A monitor, a notification channel, and a shift record
- Changes to the weld publisher, weld bridge, weld historian, or version-2 ingest
- BACnet, Modbus, or a real building-management system
