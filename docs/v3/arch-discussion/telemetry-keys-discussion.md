# Primary and Secondary Keys in Time-Series Telemetry Storage

## Overview

This discussion examines whether traditional database concepts of primary keys and secondary keys apply when storing time-series telemetry data for anomaly detection. Two main scenarios are considered:

1. **Physical plant telemetry** written to cloud storage (with a focus on DynamoDB).
2. **Enterprise backend logs** written to Elasticsearch.

In both cases, a downstream system collects reduced summaries (aggregates, windowed statistics, etc.) for anomaly detection and alerting.

## Do Primary and Secondary Keys Apply?

**Yes**, the concepts apply, but they are adapted to the characteristics of time-series data (append-heavy, time-ordered, high volume).

### Telemetry in Cloud Storage / Time-Series Databases

- **Primary key / series identity**: Typically a combination of low-to-medium cardinality tags (e.g., `device_id`, `sensor_type`, `plant_zone`) plus a timestamp. This uniquely identifies a time series and controls physical storage layout, ordering, compression, and deduplication.
  - Examples: GreptimeDB (`PRIMARY KEY` on tags + `TIME INDEX` on timestamp), TimescaleDB (composite keys that must include the time partition column), ClickHouse, Amazon Timestream, InfluxDB-style models.
- **Secondary indexes**: Used for efficient filtering on other dimensions before aggregation.
- **Pure object storage** (S3, etc.): The concepts appear as partitioning strategies and object-key design rather than formal primary/secondary keys. A query engine on top restores the classic concepts.

### Logs in Elasticsearch

- **Primary key**: The document `_id` (auto-generated or explicitly set, e.g., timestamp + fingerprint for deduplication). In Elasticsearch Time Series Data Streams (TSDS), dimensions + `@timestamp` drive the document identity.
- **Secondary indexes**: Field mappings and indexes on attributes such as `host`, `service`, `level`, `@timestamp`, etc., enable the queries that produce reduced summaries.

In both cases the downstream anomaly-detection system keys its summaries on the same series identity (device + metric, service + host, etc.) plus a time bucket.

## DynamoDB for Plant Telemetry

DynamoDB is a common destination for IoT/plant telemetry. Primary keys (partition key + optional sort key) and secondary indexes (GSIs/LSIs) are first-class concepts.

### How the Plant Sends Data

Typical paths:

- Direct writes from an edge gateway using the AWS SDK (`PutItem` / `BatchWriteItem`).
- AWS IoT Core MQTT topics + IoT Rules that write to DynamoDB.
- Kinesis Data Streams + Lambda for buffering and ordering.
- Aggregated writes (multiple samples consolidated into one item representing a short window) for high-frequency sensors.

### Common Key Patterns

| Pattern | Partition Key | Sort Key | Notes |
|---------|---------------|----------|-------|
| Simple | `device_id` / `asset_id` | `timestamp` | Efficient per-asset time-range and “latest reading” queries |
| Time-bucketed | `device_id#YYYY-MM-DD` (or hour) | `timestamp` | Bounds partition size; preferred for high volume or long retention |
| Composite series | `device_id#metric` | `timestamp` | When one asset produces many independent series |

**Secondary indexes (GSIs)** are added only for additional access patterns required by the summary/anomaly system (e.g., plant-wide queries, sparse alarm indexes).

Hot-partition risk is managed with time bucketing, optional sharding suffixes, and on-demand capacity. TTL is commonly used so raw data ages out while summaries are retained longer.

## Recommended Schema for Data-Center Facilities Telemetry

### Sample Data Point

| Field       | Type      | Example / Notes                          |
|-------------|-----------|------------------------------------------|
| site_id     | string    | SITE-01                                  |
| hall_id     | string    | HALL-01                                  |
| asset_id    | string    | CRAH-01, ROOM-01, UPS-01                 |
| asset_type  | string    | crah, room, ups                          |
| status      | string    | RUNNING or FAULTED                       |
| reading     | float64   | Supply-air °C, room °C, or UPS load %    |
| unit        | string    | C or pct                                 |
| stamp       | time      | UTC timestamp                            |
| alarms      | array     | Empty while RUNNING                      |

### Base Table Primary Key

| Key | Attribute   | Example Value                      | Purpose |
|-----|-------------|------------------------------------|---------|
| **PK** | `asset_key` | `SITE-01#HALL-01#CRAH-01`         | Groups all readings for one physical asset; forms the natural series identity |
| **SK** | `stamp`     | `2026-11-13T15:04:05.123Z`        | Enables efficient time-range queries and latest-reading lookups |

**Example item**

```json
{
  "asset_key": "SITE-01#HALL-01#CRAH-01",
  "stamp": "2026-11-13T15:04:05.123Z",
  "site_id": "SITE-01",
  "hall_id": "HALL-01",
  "asset_id": "CRAH-01",
  "asset_type": "crah",
  "status": "RUNNING",
  "reading": 22.4,
  "unit": "C",
  "alarms": []
}
```

### Time-Bucketing Variant (Recommended for Higher Volume)

- **PK**: `asset_key#date` → `SITE-01#HALL-01#CRAH-01#2026-11-13`
- **SK**: `stamp`

This keeps partitions bounded in size and time. The downstream collector queries the relevant day (or hour) buckets.

### Optional Secondary Indexes

| Index          | Partition Key     | Sort Key | Use Case |
|----------------|-------------------|----------|----------|
| GSI-SiteHall   | `site_id#hall_id` | `stamp`  | All assets in a hall over a time window |
| GSI-AssetType  | `asset_type`      | `stamp`  | All CRAHs (or rooms/UPS) across the estate |
| Sparse GSI-Alarms | `alarm_flag` (only present when alarms exist or status = FAULTED) | `stamp` | Fast retrieval of current faulted assets |

### Edge-to-Cloud Write Path

1. Edge gateway (or AWS IoT Core rule / Lambda) constructs the item with the keys above.
2. Writes via `PutItem` or `BatchWriteItem`.
3. Optional TTL attribute (e.g., 30–90 days) so raw high-resolution points expire automatically while reduced summaries are retained longer.

## Summary

- Primary-key and secondary-key concepts apply to both plant telemetry and Elasticsearch logs, specialized for time-series workloads.
- In DynamoDB the primary key is almost always a series identity (asset/device, possibly time-bucketed) + timestamp as the sort key.
- Secondary indexes (mainly GSIs, including sparse ones) are added only for the additional query patterns required by the anomaly-detection collection system.
- The concrete schema recommended for the data-center facilities telemetry uses `site#hall#asset` (optionally with a date bucket) as the partition key and the UTC timestamp as the sort key, giving the downstream system efficient per-series time-range access.
