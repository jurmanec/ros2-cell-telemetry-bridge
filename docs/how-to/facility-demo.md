# How-to: Facility asset demo

Start here to run the cooling unit, the room, the UPS, the local historian, and the bridge with one command.

The container publishes `CRAH-01`, `ROOM-01`, and `UPS-01` on `facility/asset_snapshot` at 1 Hz. The historian writes every snapshot to the InfluxDB bucket `facility`. The bridge posts every 5th snapshot for each asset, and posts any snapshot that carries an alarm.

`CRAH-01` latches `FAULTED` at tick 180 (about 3 minutes) with `COOLING_SUPPLY_HIGH`. `ROOM-01` latches `FAULTED` at tick 360 (about 6 minutes) with `ROOM_TEMP_HIGH`. `UPS-01` stays `RUNNING`.

**Assumption:** `p3-edge:jazzy` is already built and includes `facility_asset_publisher`, `facility_historian_node`, and `facility_bridge_node`. Compose uses that image and does not build it. If an executable is missing, build from `edge/` with `./scripts/build.sh`.

**Prerequisites:** Docker Desktop; the AWS `dev` stack deployed with the `schema_version` 3 handler; `cloud/.env` has `INGEST_URL` and `INGEST_API_KEY` for that stack. The `facility` bucket exists. A fresh InfluxDB volume creates it from `edge/influxdb/init-facility-bucket.sh`. An older volume needs the create command in [influxdb-facility-schema.md](../edge/influxdb-facility-schema.md).

---

## 1. Start

From `edge/`:

```bash
docker compose --profile facility up facility_demo
```

That starts InfluxDB, then one container. The container runs the three publishers, `facility_historian_node`, and `facility_bridge_node` (`reduce:=every_n`, `every_n:=5`). `INGEST_URL` and `INGEST_API_KEY` come from `cloud/.env`.

The weld demo is a different profile and a different container. Leave it stopped if this demo is the one you want running. Both can share the InfluxDB container. They do not share a ROS graph.

---

## 2. What lands in each store

InfluxDB keeps every snapshot, including the ones the bridge skips. Read it with [influxdb-facility.md](influxdb-facility.md).

DynamoDB keeps about one item per five snapshots for each asset until a fault latches. After `CRAH-01` or `ROOM-01` faults, every later snapshot for that asset is posted, because each one still carries the alarm. `UPS-01` stays on the one-in-five rule. The partition key is `asset_id`.

---

## 3. Grafana

The facility board is provisioned from `edge/grafana/provisioning`. It is not saved only in the UI. From `edge/`, with InfluxDB already up:

```bash
docker compose --profile facility up grafana
```

Open [http://localhost:3000](http://localhost:3000) and sign in as `p3` / `p3-grafana-dev`. The dashboard is **Facility historian**. The reading panel is one line per asset. The status panel is green while `RUNNING` and red while `FAULTED`. Set the time range to the demo window. `docker compose --profile facility up facility_demo` does not start Grafana. Name the service, or start both together:

```bash
docker compose --profile facility up facility_demo grafana
```

The password is applied when the `grafana-data` volume is first created. A volume that already has a user keeps that user. The weld datasource stays the default source for the weld bucket.

---

## 4. Dev check, 2026-11-13

The dev table was emptied, the ingest handler was deployed, and `facility_demo` ran from 16:35:51Z to 16:36:25Z. The container was then stopped. InfluxDB and Grafana were left running.

DynamoDB held 15 items, 5 for each of `CRAH-01`, `ROOM-01`, and `UPS-01`. Each item was `schema_version` 3, `status` `RUNNING`, and `sk` was the timestamp with nine fractional digits and no random suffix. One example is `CRAH-01` / `2026-11-13T16:36:00.716143294Z`. The bridge logged 15 successful posts and 72 skips. One post timed out and succeeded on the second attempt. No fault was expected: the cooling fault is at about 3 minutes.

---

## Related

- Contract: [facility-asset-snapshot.md](../contracts/facility-asset-snapshot.md)
- Plan: [facility-snapshot-plan.md](../v3/facility-snapshot-plan.md)
- Weld demo, unchanged: [weld-demo.md](weld-demo.md)
