# Contract — Weld bridge reduction

**Status:** Active  
**Date:** 2026-11-07  
**Plan:** [v2-architecture.md](../planning/v2-architecture.md) step 1  
**Node:** `weld_cell_bridge` (`cell_bridge` / `weld_cell_bridge_node`)  
**Topic:** `cell/weld_snapshot`  
**Body:** [weld-cell-snapshot.md](weld-cell-snapshot.md) (`schema_version` 2)  
**Source:** [bridge_filtering_welding_and_amr.md](../v2-architecture/bridge_filtering_welding_and_amr.md)

The weld publisher emits every snapshot. This contract is the rule `weld_cell_bridge_node` uses to choose which of those snapshots to POST. A forwarded body is the unchanged `schema_version` 2 JSON. A skipped snapshot is not sent and is not rewritten.

`cell_bridge` and `schema_version` 1 are unchanged. Averaging and field pruning are outside this contract.

`weld_cell_bridge_node` reads `reduce`, `every_n`, and `heartbeat_sec`. The default `reduce=all` forwards every snapshot.

---

## Policy

One policy for every welding cell on this bridge. It is a node parameter, not a per-cell setting and not a field in the JSON.

The bridge keeps a small piece of state per `cell_id`: a count for `every_n`, or the time of the last forwarded snapshot for `heartbeat`. Cells that share the topic do not share that state. `WELD_CELL_07` and `WELD_CELL_08` each get every fifth of their own snapshots, not every fifth message on the topic.

`cell_id` comes from the snapshot. The bridge has no `cell_id` parameter. An empty `cell_id` is dropped before the policy runs and before any POST.

---

## Parameters

| Parameter | Type | Default | Used by |
|-----------|------|---------|---------|
| `reduce` | string | `all` | Policy name: `all`, `every_n`, or `heartbeat` |
| `every_n` | int | `5` | `every_n` only. Values below 1 are invalid. `1` forwards every snapshot |
| `heartbeat_sec` | double | `5.0` | `heartbeat` only. Interval in seconds. Values below or equal to 0 are invalid |

`ingest_url`, `api_key`, and `max_attempts` stay as they are. An unknown `reduce` value is invalid.

At 1 Hz, `every_n` of 5 and `heartbeat_sec` of 5.0 both land near one cloud POST per cell every 5 seconds, plus any alarm.

---

## Decisions

The clock for `heartbeat` is the bridge node clock when the snapshot is handled, not the ROS stamp inside the message.

A snapshot with a non-empty `alarms` array is forwarded under `every_n` and `heartbeat` even when the count or the interval would skip it. The alarm snapshot counts as one of the N, and it starts the next heartbeat interval, so the same alarm is not followed immediately by the periodic sample.

| `reduce` | Forward when |
|----------|----------------|
| `all` | Every snapshot. `every_n` and `heartbeat_sec` are ignored |
| `every_n` | The snapshot count for that `cell_id` is a multiple of `every_n`, or `alarms` is non-empty. The count starts at 1 for the first snapshot from that cell |
| `heartbeat` | That `cell_id` has not been forwarded yet, or at least `heartbeat_sec` has passed since its last forwarded snapshot, or `alarms` is non-empty |

A selected snapshot uses the existing POST retry: up to 3 attempts, retry on connection failure or HTTP 5xx, drop on HTTP 4xx. The count or the last-forward time advances when the snapshot is selected. A later failure does not rewind it, and the skipped samples are not replayed.

---

## Examples (`every_n` = 5)

| Snapshot from one cell | `alarms` | Posted |
|------------------------|----------|--------|
| 1–4 | empty | no |
| 5 | empty | yes |
| 6–9 | empty | no |
| 7 | one alarm | yes |
| 10 | empty | yes |

Snapshot 7 is posted because it carries an alarm. Snapshot 10 is still the next multiple of 5.
