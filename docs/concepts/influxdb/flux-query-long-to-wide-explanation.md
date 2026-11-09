# Flux Query Explanation

```flux
from(bucket: "weld")
  |> range(start: -15m)
  |> filter(fn: (r) => r._measurement == "weld_snapshot" and r.cell_id == "WELD_CELL_07")
  |> pivot(rowKey: ["_time"], columnKey: ["_field"], valueColumn: "_value")
  |> limit(n: 10)
```

This Flux query retrieves recent data for one weld cell and reshapes it into a wide, table-like format.

## Step-by-step

```flux
from(bucket: "weld")
```
Start reading from the bucket named `"weld"`.

```flux
|> range(start: -15m)
```
Limit the data to the **last 15 minutes**.

```flux
|> filter(fn: (r) => r._measurement == "weld_snapshot" and r.cell_id == "WELD_CELL_07")
```
Keep only points where:
- the measurement is `weld_snapshot`, **and**
- the tag `cell_id` equals `"WELD_CELL_07"`

```flux
|> pivot(rowKey: ["_time"], columnKey: ["_field"], valueColumn: "_value")
```
Convert the data from InfluxDB’s default **long format** to a **wide format**:

- Before pivot (long): one row per field
  ```
  _time                _field     _value
  2026-11-08T14:00:00Z current    142.5
  2026-11-08T14:00:00Z voltage    24.1
  2026-11-08T14:00:00Z temperature 68.3
  ```

- After pivot (wide): one row per timestamp, fields become columns
  ```
  _time                current  voltage  temperature
  2026-11-08T14:00:00Z 142.5    24.1     68.3
  ```

`rowKey: ["_time"]` means “group rows by timestamp”.  
`columnKey: ["_field"]` means “turn each field name into a column”.  
`valueColumn: "_value"` means “use the `_value` as the cell content”.

```flux
|> limit(n: 10)
```
Return only the first **10** rows of the result.

## Summary

The query says:

> “From the last 15 minutes of `weld_snapshot` data for cell `WELD_CELL_07`, give me a spreadsheet-style table (one row per timestamp, one column per field) and show me only the first 10 rows.”
