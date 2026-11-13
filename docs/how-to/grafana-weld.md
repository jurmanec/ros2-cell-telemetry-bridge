# How-to: Grafana for the weld historian

Start Grafana beside the weld demo and confirm it can read the `weld` bucket. This is steps 1 and 2 of [historian-visualization.md](../planning/historian-visualization.md). The gas-flow graph and the fault heatmap are not in this setup yet.

Grafana does not run ROS. It queries InfluxDB over HTTP. The edge image is not rebuilt.

---

## 1. Compose service

`edge/docker-compose.yml` has a `grafana` service on the `weld` profile.

| Setting | Value |
|---------|--------|
| Image | `grafana/grafana:11.4.0` |
| Host | [http://localhost:3000](http://localhost:3000) |
| User | `p3` |
| Password | `p3-grafana-dev` |
| Starts after | `influxdb` is healthy |
| UI state | Named volume `grafana-data` |

`docker compose --profile weld up weld_demo` does not start Grafana. Name the service.

From `edge/`, with the demo already up or starting now:

```bash
docker compose --profile weld up grafana
```

To start the demo and the board together:

```bash
docker compose --profile weld up weld_demo grafana
```

The password is applied when the `grafana-data` volume is first created. A volume that already has a user keeps that user.

---

## 2. InfluxDB data source

`edge/grafana/provisioning/datasources/influx.yml` is mounted at `/etc/grafana/provisioning`. Grafana loads it on startup. The source is not typed into the UI.

| Setting | Value |
|---------|--------|
| Name | Weld historian |
| URL | `http://influxdb:8086` (the Compose network, not `localhost`) |
| Query language | Flux |
| Organization | `p3` |
| Bucket | `weld` |
| Token | `p3-historian-dev-token` |

That token is the same local demo token InfluxDB is initialized with. It is already in `edge/docker-compose.yml`. A second file in the same directory, `facility.yml`, points at the `facility` bucket. That board is [facility-demo.md](facility-demo.md).

Open [http://localhost:3000](http://localhost:3000), sign in as `p3`, and go to **Connections → Data sources → Weld historian**. The page should show the Flux source for org `p3` and bucket `weld`. **Save & test** should report that the data source is working, once the `influxdb` container is healthy.

---

## Related

- Read the same bucket in the InfluxDB UI: [influxdb-weld.md](influxdb-weld.md)
- Start the demo that fills the bucket: [weld-demo.md](weld-demo.md)
- Plan for the graphs that are not built yet: [historian-visualization.md](../planning/historian-visualization.md)
