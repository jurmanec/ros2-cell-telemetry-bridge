# How-to: InfluxDB weld history

Read snapshots the historian wrote to the local InfluxDB bucket. The UI is available only while the `influxdb` container is running (`docker compose --profile weld up weld_demo` from `edge/`).

**Login:** [http://localhost:8086](http://localhost:8086)  
**User:** `p3`  
**Password:** `p3-historian-dev`  
**Organization:** `p3`  
**Bucket:** `weld`  
**Measurement:** `weld_snapshot`

`cell_id` is a tag (`WELD_CELL_07`, `WELD_CELL_08`). The historian writes about one point per second per cell, including snapshots the bridge did not POST.

---

## 1. A few rows for one field

In the left sidebar, open **Data Explorer**. Set the bucket to `weld` and the time range to the past 15 minutes. In the query builder, filter:

- `_measurement` = `weld_snapshot`
- `cell_id` = `WELD_CELL_07` (or `WELD_CELL_08`)
- `_field` = `gas_flow_l_min`

Switch the view from **Graph** to **Table**, then click **Submit**. Expect about one row per second. `gas_flow_l_min` stays near 15 while the cell is running and drops to 8 on a `GAS_FLOW_LOW` snapshot.

Any other numeric field (`weld_current_a`, `arc_on_s`, a joint temperature) works the same way. The builder needs one field selected before it returns rows.

---

## 2. Full snapshots

In **Data Explorer**, open **Script Editor** and submit:

```flux
from(bucket: "weld")
  |> range(start: -15m)
  |> filter(fn: (r) => r._measurement == "weld_snapshot" and r.cell_id == "WELD_CELL_07")
  |> pivot(rowKey: ["_time"], columnKey: ["_field"], valueColumn: "_value")
  |> limit(n: 10)
```

Each result row is one snapshot. Tags `cell_id` and `status` are columns, along with fields such as `weld_current_a`, `arc_on_s`, and `alarm_count`. Change `cell_id` or `limit` to see the other cell or more rows.

An empty table means the historian has not written in that window. The demo container should be logging `historian wrote` about once a second for each cell.

---

## 3. Same query from the shell

From `edge/`, with the container already up:

```bash
docker compose --profile weld exec influxdb influx query --org p3 --token p3-historian-dev-token \
  'from(bucket:"weld") |> range(start:-1h) |> filter(fn:(r) => r.cell_id == "WELD_CELL_07")'
```

This prints one line per field. The script in section 2 pivots those fields into one row per snapshot.

---

## Related

- Start the demo that fills this bucket: [weld-demo.md](weld-demo.md)
- Reduced copies of the same cells in DynamoDB: [dynamodb-queries-p3-cell-telemetry-dev-samples.md](dynamodb-queries-p3-cell-telemetry-dev-samples.md)
