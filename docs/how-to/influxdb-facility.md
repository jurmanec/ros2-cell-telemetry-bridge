# How-to: InfluxDB facility history

Read snapshots the facility historian wrote to the local InfluxDB bucket. The UI is available while the `influxdb` container is running (`docker compose --profile weld up -d influxdb` from `edge/`).

**Login:** [http://localhost:8086](http://localhost:8086)  
**User:** `p3`  
**Password:** `p3-historian-dev`  
**Organization:** `p3`  
**Bucket:** `facility`  
**Measurement:** `facility_asset`

`asset_id` is a tag (`CRAH-01`, `ROOM-01`, `UPS-01`). The historian writes about one point per second per asset, including snapshots a later bridge will not POST.

---

## Count points in the last hour

From `edge/`, with the container already up:

```bash
docker compose --profile weld exec influxdb influx query --org p3 --token p3-historian-dev-token \
  'from(bucket:"facility") |> range(start:-1h) |> filter(fn:(r) => r._measurement == "facility_asset" and r._field == "reading") |> group(columns:["asset_id"]) |> count()'
```

Expect about one row per second for each asset id that was publishing in that hour. A count near the number of seconds the publishers ran, for all three ids, means the historian kept every snapshot.

---

## One asset, wide rows

```flux
from(bucket: "facility")
  |> range(start: -1h)
  |> filter(fn: (r) => r._measurement == "facility_asset" and r.asset_id == "CRAH-01")
  |> pivot(rowKey: ["_time"], columnKey: ["_field"], valueColumn: "_value")
  |> limit(n: 10)
```

`reading` stays near 16 while `CRAH-01` is `RUNNING` and moves to 28 on `COOLING_SUPPLY_HIGH`. `ROOM-01` stays near 22 until `ROOM_TEMP_HIGH`. `UPS-01` stays at 40.

---

## Related

- Schema: [influxdb-facility-schema.md](../edge/influxdb-facility-schema.md)
- Weld bucket, unchanged: [influxdb-weld.md](influxdb-weld.md)
