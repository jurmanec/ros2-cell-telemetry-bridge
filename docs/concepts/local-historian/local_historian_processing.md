# Processing on the local historian

Optional InfluxDB processing for the weld historian. None of this runs today. `weld_historian_node` only writes every snapshot into the `weld` bucket.

InfluxDB calls this area **Process data**. In the 2.7 docs that means Flux transforms, and tasks that run Flux on a schedule and write the result back into a bucket. On a plant historian the same jobs are local trending, a downsampled series for the floor, and a watch that looks across more than one sample. This bucket can stand in for those jobs. The bridge does not read the result. It still decides the DynamoDB copy from the live ROS message.

The raw stream stays in `weld`: about one `weld_snapshot` per second per cell, for 7 days. Tags `cell_id` and `status` are what a query filters and groups on. Fields such as `gas_flow_l_min`, `weld_current_a`, and `parts_total_shift` are what it aggregates. Schema: [influxdb-schema.md](../../edge/influxdb-schema.md).

---

## Shape a query

A one-shot Flux query is the historian's ad-hoc trend. The how-to already does the first two steps.

| Flux | What it stands in for on the historian |
|------|----------------------------------------|
| `filter` on `cell_id` | One cell's trace |
| `pivot` | One row per snapshot, so a screen can show gas flow, current, and alarm count together. Line-by-line: [flux-query-long-to-wide-explanation.md](../influxdb/flux-query-long-to-wide-explanation.md) |
| `difference` on `parts_total_shift` | A part rate. The field is a running total |
| `aggregateWindow` on `weld_current_a` | A one-minute mean for a chart, without throwing away the 1 Hz points |

Exporting that result as CSV and opening it in Pandas is the same kind of look, frozen at export time. The next snapshot is not in the file until the next export.

---

## Keep a reduced local series

A task runs the aggregate on a schedule and writes it to a second bucket. The `weld` bucket stays the full history. The task bucket is the short series an on-site display would read: a one-minute mean of `gas_flow_l_min`, or a count of `FAULTED` points, per `cell_id`.

That is the local version of "keep detail here, publish a thinner stream." The cloud's thinner stream is different. `weld_cell_bridge` drops snapshots with `every_n` before the POST. It does not query InfluxDB, and a task cannot change what DynamoDB already stored.

---

## Watch a window

A task can require a condition to hold across several samples of the full history, including snapshots the bridge never posted.

| Watch | Local historian behavior it simulates |
|-------|----------------------------------------|
| `gas_flow_l_min` stays at 8 for several samples | Shielding gas low long enough to matter, not a single noisy point |
| `alarm_count` above zero | The fault is in the local record even when that snapshot was not the one posted |
| No new point for a cell inside the expected second | The cell stopped publishing. The cloud cannot see that gap in the samples it never received |

The cloud `CELL_ALERT` still fires from the one snapshot the bridge forwarded. The local watch is the floor's view of the whole series.

---

## Related

- What a local historian is: [local_historian.md](../local_historian.md)
- Read the raw bucket: [influxdb-weld.md](../../how-to/influxdb-weld.md)
- Why this demo uses InfluxDB 2.7: [version-and-tools.md](../influxdb/version-and-tools.md)
