# Plan — v2 telemetry flow

**Status:** Proposed  
**Date:** 2026-11-07  
**Follows:** [weld-cell-snapshot.md](weld-cell-snapshot.md) (done)  
**Shape:** [v2-architecture/README.md](../v2-architecture/README.md)

The weld publisher, bridge, and version-2 ingest already move one full snapshot a second to DynamoDB. This plan changes that flow for the weld path only. The cell still publishes the full stream. A local historian keeps every snapshot. The bridge becomes configurable and forwards a reduced stream. Several weld cells run at once. The historian shows the full history. The cloud store shows only what the bridge forwarded.

`CellTelemetry` / `schema_version` 1 stays as it is. A mobile-robot path is a later decision, not a step in this plan.

---

## Goal

Simulate a plant-style split on the welding cell:

| Where | What you can see |
|-------|------------------|
| Local historian | Every `cell/weld_snapshot` from each running cell, at the publisher rate (default 1 Hz) |
| Cloud store (DynamoDB) | The reduced snapshots the weld bridge chose to POST. An alarm snapshot is still stored, and it still logs `CELL_ALERT` |

The bridge policy is a parameter, not a hardcoded drop. Forwarding every snapshot remains available, so the current demo still runs.

---

## Weld path

Publishers are unchanged: `weld_cell_publisher` on `cell/weld_snapshot`, `cell_id` and `period_sec` set per process, a distinct ROS node name (`-r __node:=...`) when more than one cell shares the domain.

One `weld_cell_bridge` subscribes to that topic. It still copies `cell_id` from the message and still has no `cell_id` parameter. Before POST it applies one reduction policy for every welding cell. The downsample count, or the last-POST time, is tracked per `cell_id`, so two cells do not share that state.

The historian is a separate subscriber on the same topic. It writes every message. It does not decide what the cloud receives.

---

## Steps

### 1. Bridge policy — done

[weld-bridge-reduction.md](../contracts/weld-bridge-reduction.md) names parameters `reduce` (default `all`), `every_n` (default 5), and `heartbeat_sec` (default 5.0). One policy applies to every welding cell. The count or last-POST time is per `cell_id`. An alarm is always forwarded. `weld_cell_bridge_node` reads these parameters. Source notes: [bridge_filtering_welding_and_amr.md](../v2-architecture/bridge_filtering_welding_and_amr.md).

| Policy | Behavior |
|--------|----------|
| `all` | POST every snapshot. This is the default. The two-cell demo sets `every_n` to 5 |
| `every_n` | POST every Nth snapshot for that `cell_id`. Default N is 5 (about every 5 s at 1 Hz) |
| `heartbeat` | POST on an interval (default 5 s) and immediately when `alarms` is non-empty |

`every_n` and `heartbeat` always forward a snapshot that carries an alarm, even when it would otherwise be skipped. The forwarded body stays `schema_version` 2. No averaging and no field pruning in this plan.

### 2. Bridge implementation — done

`weld_cell_bridge_node` reads `reduce`, `every_n`, and `heartbeat_sec`. The decision is `WeldReducePolicy`, with a count or last-POST time per `cell_id`. `test_weld_reduce` covers `all`, `every_n` (the 5th snapshot and the alarm at snapshot 7), separate counts for two cells, `heartbeat` including an alarm that resets the interval, and invalid parameters. The version-1 `cell_bridge` is unchanged. Default `reduce=all` keeps the current demo.

### 3. Local historian — done

InfluxDB 2.7 (`influxdb` service, bucket `weld`, 7-day retention) and `weld_historian_node` are on the `weld` profile. The node subscribes to `cell/weld_snapshot` and writes every snapshot, including joints, process, and alarms. `cell_id` is a tag, so a query can select one cell over a time range. The bridge does not read InfluxDB. `test_weld_line` checks the line protocol. Start with `docker compose --profile weld up weld_historian` from `edge/`. The image must include `cell_historian`.

### 4. Several cells, both stores

Launch from `edge/` with `docker compose --profile weld up weld_demo`. That starts InfluxDB and one container running `WELD_CELL_07`, `WELD_CELL_08`, `weld_historian_node`, and `weld_cell_bridge_node` (`reduce:=every_n`, `every_n:=5`). The URL and API key come from `cloud/.env`.

Prove:

- The historian has about one point per second per cell, including snapshots the bridge did not POST.
- DynamoDB has the reduced set for each `cell_id`. With `every_n` at 5, about one stored snapshot per five published, plus any alarm snapshot that fell between those posts.
- A `GAS_FLOW_LOW` snapshot is in both stores, and CloudWatch still shows `CELL_ALERT`.

Docker Desktop often fails to discover DDS peers across containers. The prove step runs the subscribers in a container that can see the publishers, or records that limit if a second container cannot.

### 5. Next path — decide only

After the weld path meets the goal, choose one follow-on. Do not start it in this plan.

- The original `cell/telemetry` path: the same historian-plus-filter split on `cell_bridge`, leaving `schema_version` 1 batches valid.
- A mobile-robot publisher and bridge, with its own reduction notes in the filtering doc. No fleet manager in that choice.

---

## Out of this plan

- Implementing the original cell path or a mobile-robot path
- Averaging, field pruning, and fleet-level rollup
- A plant historian product (PI, Canary) or a cloud time-series database
- MQTT or OPC UA
- Replacing or removing `CellTelemetry` / `schema_version` 1
