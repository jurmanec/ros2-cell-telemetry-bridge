# InfluxDB schema for the weld historian

How the `weld` bucket is shaped, and where that shape comes from.

InfluxDB's own guidance is the rule used here: structure measurements, fields, and tags so queries stay easy to write, and keep series cardinality low so those queries stay fast. A series is one measurement plus one combination of tag values. A tag is a low-cardinality label used to filter or group. A field is the value recorded at that time.

---

## How this schema is created

Nothing in the repo issues a create-measurement or create-field call. InfluxDB 2.7 builds the schema from the data that arrives.

Two steps run before any point exists:

1. Compose creates the empty database. The `influxdb` service in `edge/docker-compose.yml` sets `DOCKER_INFLUXDB_INIT_ORG` to `p3`, `DOCKER_INFLUXDB_INIT_BUCKET` to `weld`, and `DOCKER_INFLUXDB_INIT_RETENTION` to `7d`. Those variables apply when the `influxdb-data` volume is empty. They create the org, the bucket, and the retention. They do not define a measurement, a tag, or a field.
2. `weld_historian_node` writes one line of line protocol per ROS snapshot. The node subscribes to `cell/weld_snapshot`, drops a snapshot with an empty `cell_id`, and calls `weld_snapshot_to_line` in `edge/ws/src/cell_historian/src/weld_line.cpp`. `influx_write` POSTs that line to `/api/v2/write`. The first successful write creates the measurement `weld_snapshot` and the tag keys and field keys in that line. Later writes add a key the server has not seen yet. Alarm fields such as `a0_code` appear on the first snapshot that carries an alarm.

The node parameters `influx_url`, `influx_org`, `influx_bucket`, and `influx_token` choose where the line goes. They do not name the measurement or the columns. The measurement name is the literal `weld_snapshot` at the start of the line. The ROS topic `cell/weld_snapshot` is a separate name.

---

## One point per snapshot

Each ROS message becomes one Influx point. The joints are columns on that point (`j1_temp_c`, `j2_current_a`, and so on), not a tag value of `axis` and not a second measurement. One snapshot stays one row, and the series count does not grow by three axes per cell.

The timestamp is the ROS stamp, in nanoseconds, not the time the HTTP write happened.

---

## Tags

| Tag | Why it is a tag |
|-----|-----------------|
| `cell_id` | The filter used to read one cell. A handful of cell ids. |
| `cell_type` | One label for this demo, `robotic_welding`. |
| `status` | `RUNNING` or `FAULTED`. Queries select a state, and the set of values stays at two, so each cell has two series rather than a new series on every snapshot. |
| `program` | The weld program name. The simulator publishes one program. |

With two cells, one cell type, two statuses, and one program, the bucket holds four series. That is the cardinality this schema is aiming for.

`status` does change over time. A `FAULTED` point is a different series from the `RUNNING` points for the same `cell_id`. A query filtered only by `cell_id` still returns both.

---

## Fields

Fields are the readings on that point. They are not indexed, and they are not used to split series.

| Fields | Kind |
|--------|------|
| `cycle_time_last_s`, `oee_availability`, `weld_current_a`, `weld_voltage_v`, `wire_feed_m_min`, `arc_on_s`, `gas_flow_l_min`, `j{axis}_temp_c`, `j{axis}_current_a` | Floats. These move on the 1 Hz stream. |
| `parts_total_shift`, `parts_scrap_shift`, `alarm_count` | Integers (`i` suffix in the line). Counts, including a running part total. |
| `a{i}_code`, `a{i}_severity`, `a{i}_message` | Strings, present only when that snapshot has an alarm. Free text such as an alarm message would grow the series set if it were a tag, and a timeline is not grouped by the message text. |

`gas_flow_l_min` stays a field even though a gas alarm is visible there. The number is the measurement. `status` is the label a query filters on.

---

## Related

- Read the bucket: [influxdb-weld.md](../how-to/influxdb-weld.md)
- Why the demo stays on InfluxDB 2.7: [version-and-tools.md](../concepts/influxdb/version-and-tools.md)
- Line protocol: `edge/ws/src/cell_historian/src/weld_line.cpp`
