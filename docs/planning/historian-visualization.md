# Plan — Local historian visualization

**Status:** Proposed  
**Date:** 2026-11-08  
**Follows:** [v2-architecture.md](v2-architecture.md) steps 1–3 (historian is writing). Step 4's proof can use these screens. It does not have to wait on them.  
**Context:** [local_historian.md](../concepts/local_historian.md), [local_historian_processing.md](../concepts/local-historian/local_historian_processing.md)

The `weld` bucket already stores every snapshot. This plan adds screens on that bucket so the full local history can be seen the way a plant historian is seen: an engineer querying one cell, and a board that stays up for the shift. The cloud view stays DynamoDB. These screens do not change what `weld_cell_bridge` posts.

InfluxDB's visualization options are the built-in UI, Chronograf, and Grafana. This plan uses the first and the third. Chronograf is the InfluxDB 1.x UI. InfluxDB 2.7 already includes that explorer on port 8086, so a separate Chronograf container would repeat it.

---

## Goal

Show both weld cells from the historian at full rate, including snapshots the bridge did not POST, and make a `FAULTED` / `GAS_FLOW_LOW` point obvious on that screen.

| Screen | Production role it stands in for | In this project |
|--------|----------------------------------|-----------------|
| InfluxDB UI | Engineer pulls one cell and one field during a fault. A saved dashboard can sit open for the shift | Already running at [http://localhost:8086](http://localhost:8086). No new service |
| Grafana | Wall board that can later sit beside more than one store | A Compose service on the same network as `influxdb`, reading bucket `weld` |
| Chronograf | Dashboard for a plant still on InfluxDB 1.x | Not in this plan |

---

## Steps

### 1. InfluxDB UI dashboard

From a running weld demo, save a dashboard in the 2.7 UI. Org `p3`, bucket `weld`.

- One graph of `gas_flow_l_min` for `WELD_CELL_07` and one for `WELD_CELL_08`, past hour.
- A table of points with `status` = `FAULTED`.

Done when a `GAS_FLOW_LOW` snapshot is visible on that dashboard (gas flow at 8, `status` `FAULTED`) about 10.5 minutes after the publishers start, and the graphs keep updating while the demo runs. Query steps: [influxdb-weld.md](../how-to/influxdb-weld.md).

### 2. Grafana board

Add Grafana to the `weld` profile. Its data source is the InfluxDB 2.7 API at `http://influxdb:8086`, org `p3`, bucket `weld`, using the existing demo token.

- The same two gas-flow graphs, one per `cell_id`.
- `FAULTED` points marked on those graphs.

Done when the board shows every snapshot in the window, and a DynamoDB query for the same cell and the same window shows about one item per five snapshots plus the alarm item. That picture is the step 4 split: full history local, reduced set in the cloud.

---

## Out of this plan

- Chronograf, or any second UI pointed at the same 2.7 explorer
- A cloud dashboard as the primary view of cell history
- Changing the bridge, the reduction policy, or the line protocol so a screen can read new fields
- Influx tasks, downsampled buckets, and windowed watches. Those are separate historian-processing experiments: [local_historian_processing.md](../concepts/local-historian/local_historian_processing.md)
