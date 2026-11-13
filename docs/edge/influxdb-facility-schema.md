# InfluxDB schema for the facility historian

How the `facility` bucket is shaped. The weld bucket is not reused. See [influxdb-schema.md](influxdb-schema.md) for the same rules on the weld measurement.

---

## How this schema is created

1. The org `p3` already exists for the weld bucket. Compose creates the bucket `facility` with retention `7d` only when `influxdb-data` is first created, via `edge/influxdb/init-facility-bucket.sh`. An existing volume does not re-run that script. On a volume that already has the weld bucket, create it once:

```bash
docker compose --profile weld exec influxdb \
  influx bucket create --name facility --org p3 --retention 7d --token p3-historian-dev-token
```

2. `facility_historian_node` writes one line per snapshot on `facility/asset_snapshot`. It drops an empty `asset_id`. It does not filter and it does not POST. The first successful write creates the measurement `facility_asset`.

---

## Tags and fields

| Tag | Why it is a tag |
|-----|-----------------|
| `site_id` | The site. This demo uses `SITE-01`. |
| `hall_id` | The hall. This demo uses `HALL-01`. A query can still show the cooling unit and the room together. |
| `asset_id` | The filter used to read one unit. |
| `asset_type` | `crah`, `room`, or `ups`. |
| `status` | `RUNNING` or `FAULTED`. A faulted point is a different series from the running points for the same asset. A query filtered only by `asset_id` still returns both. |

| Field | Kind |
|-------|------|
| `reading` | Float. Supply-air °C, room °C, or UPS load percent, depending on `asset_type`. |
| `alarm_count` | Integer. |
| `a{i}_code`, `a{i}_severity`, `a{i}_message` | Strings, present only when that snapshot has an alarm. |

The timestamp is the ROS stamp, in nanoseconds. With three assets and two statuses, the bucket holds six series once both faults have latched, and three while every asset is running.

---

## Related

- Read the bucket: [influxdb-facility.md](../how-to/influxdb-facility.md)
- Contract: [facility-asset-snapshot.md](../contracts/facility-asset-snapshot.md)
