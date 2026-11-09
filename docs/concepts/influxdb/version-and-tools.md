# InfluxDB version and nearby tools

What this project runs for the local weld historian, and how that covers the jobs InfluxData assigns to Telegraf and InfluxDB 3 Explorer.

The current InfluxDB generation is **InfluxDB 3** (Core and Enterprise). Its query language is SQL. The UI for that server is a separate product, InfluxDB 3 Explorer. The image tag `latest` tracks that generation.

This project runs **InfluxDB 2.7**.

---

## Why 2.7

The historian is a local stand-in for a plant historian. It stores every `cell/weld_snapshot` so the site keeps the full stream while the bridge posts a reduced copy to DynamoDB. The demo writes about two small snapshots a second and keeps them for 7 days. That load does not need the high-cardinality engine InfluxDB 3 is built for.

2.7 is the version the client and the how-to are written for. One Compose service covers setup, writes, and the screen used to read the bucket.

| Piece | What the demo uses |
|-------|--------------------|
| Image | `influxdb:2.7` in `edge/docker-compose.yml` |
| First-run setup | `DOCKER_INFLUXDB_INIT_*`: user `p3`, org `p3`, bucket `weld`, retention `7d`, admin token |
| Write path | `weld_historian_node` posts line protocol to `/api/v2/write?org=&bucket=&precision=ns` with `Authorization: Token` |
| Read path | Flux in the built-in UI at [http://localhost:8086](http://localhost:8086), and `influx query` from the container |

Pinning `2.7` keeps that contract stable. A floating `latest` tag would pull InfluxDB 3, whose port, auth, and query language do not match `influx_write` or [influxdb-weld.md](../../how-to/influxdb-weld.md).

Moving the demo to InfluxDB 3 would replace the write call, the bucket setup, the Flux queries, and the built-in UI. The cell, the historian, and the reduced cloud path would tell the same story afterward.

---

## Telegraf

Telegraf is InfluxData's collection agent. It scrapes sources such as CPU, disk, MQTT, or OPC UA and writes line protocol into InfluxDB. On an InfluxDB 3 stack it is the usual producer: Telegraf collects, the database stores, Explorer or Grafana displays.

This repo already has a producer. `weld_historian_node` subscribes to `cell/weld_snapshot` and writes the `weld` bucket. Telegraf does not speak that ROS message. A second agent writing the same snapshots would duplicate the historian. Host metrics, which are Telegraf's default payload, are not what this demo stores. An OPC UA or MQTT input would be a new plant protocol, which the v2 plan leaves out.

---

## InfluxDB 3 Explorer

Explorer is the web UI for InfluxDB 3 Core and Enterprise. It queries that server with SQL, manages databases and tokens, and can help configure Telegraf. It does not connect to an InfluxDB 2.7 server.

The 2.7 container already includes Data Explorer on port 8086. [influxdb-weld.md](../../how-to/influxdb-weld.md) uses that UI to read `weld_snapshot` rows for one `cell_id`. A later Grafana board would use the same bucket and the same token. Explorer becomes useful only if the database itself moves to InfluxDB 3.

---

## Related

- Read the bucket: [influxdb-weld.md](../../how-to/influxdb-weld.md)
- Historian role in the weld flow: [v2-architecture.md](../../planning/v2-architecture.md)
- What a local historian is for: [local_historian.md](../local_historian.md)
