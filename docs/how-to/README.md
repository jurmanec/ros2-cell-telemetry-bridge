# How-to guides

Operational recipes for demos. Product docs stay under [CHARTER](../CHARTER.md), [ARCHITECTURE](../ARCHITECTURE.md), and [contracts/](../contracts/).

| Guide | When to use |
|-------|-------------|
| [Edge → AWS ingest](edge-to-aws.md) | Run ROS publisher + bridge pointed at live API Gateway (not the local stub) |
| [Two-cell weld demo](weld-demo.md) | Start both weld cells, the historian, and the reduced bridge with one Compose command. Assumes `p3-edge:jazzy` is already built |
| [InfluxDB weld history](influxdb-weld.md) | Read `weld_snapshot` rows in the local InfluxDB UI at `localhost:8086` |
| [DynamoDB sample queries](dynamodb-queries-p3-cell-telemetry-dev-samples.md) | Query `p3-cell-telemetry-dev-samples` in the AWS PartiQL editor |
| [Welding-cell publisher and bridge](weld-publisher-bridge.md) | Run `weld_cell_publisher` and `weld_cell_bridge` against live API Gateway, using `INGEST_URL` and `INGEST_API_KEY`. Assumes `p3-edge:jazzy` is already built |
