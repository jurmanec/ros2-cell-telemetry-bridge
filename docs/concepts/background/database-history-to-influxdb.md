# Background: From General-Purpose Databases to Specialized Systems like InfluxDB

## Early Databases (1960s–1980s)

The first database systems were hierarchical and network models (e.g., IMS, CODASYL). They were rigid and tightly coupled to specific applications.

In the 1970s, **relational databases** (RDBMS) emerged from Edgar Codd’s work. Systems like System R, Oracle, DB2, and later MySQL and PostgreSQL became dominant. Their strengths were:

- Structured data in tables
- SQL as a declarative query language
- ACID transactions
- Strong consistency

They excelled at business data (customers, orders, inventory) but were not optimized for high-volume, append-only, time-ordered data.

## The 2000s: Scale and Specialization

As internet-scale applications grew, traditional RDBMS struggled with:

- Massive write throughput
- Horizontal scaling
- Flexible or semi-structured data

This led to the **NoSQL** movement (roughly 2005–2015):

- Key-value stores (Redis, Dynamo)
- Document stores (MongoDB, CouchDB)
- Wide-column stores (Cassandra, HBase)
- Graph databases (Neo4j)

These systems traded some consistency or query flexibility for scale and specific access patterns. The industry learned that “one database for everything” was often suboptimal.

## The Rise of Time Series Data

Around the same period, a new workload became critical:

- Server and application monitoring (metrics)
- IoT sensor data
- Financial tick data
- Industrial telemetry
- Observability (logs, metrics, traces)

This data has distinct characteristics:

- Extremely high write volume
- Almost always appended (rarely updated)
- Queried by time range + filters (tags/labels)
- High cardinality in some dimensions
- Compression and downsampling are important

General-purpose databases (even NoSQL ones) handled this poorly:

- Row-oriented storage wasted space and I/O
- Indexes became expensive at high cardinality
- Query patterns didn’t match B-tree or document models well

## Purpose-Built Time Series Databases

In the early-to-mid 2010s, specialized **time series databases (TSDBs)** appeared:

| System         | Approx. origin | Notes |
|----------------|----------------|-------|
| **InfluxDB**   | 2013           | Created by Paul Dix / Errplane (later InfluxData). Focused on metrics, developer experience, and a simple data model. |
| **Prometheus** | 2012–2015      | Born at SoundCloud; pull-based, strong in cloud-native monitoring. |
| **OpenTSDB**   | ~2010          | Built on HBase. |
| **Graphite**   | ~2006–2008     | Early metrics system (Whisper storage). |
| **TimescaleDB**| 2017           | PostgreSQL extension bringing time-series features to a relational system. |

**InfluxDB** specifically emerged because existing tools were either too limited (Graphite), too operationally heavy (OpenTSDB on HBase), or not designed as a general-purpose metrics store. It introduced:

- A simple line protocol for ingestion
- The measurement / tag / field model
- Built-in retention policies, continuous queries (later tasks), and a UI
- The TSM (Time-Structured Merge Tree) storage engine optimized for time-ordered data

## Why This Specialization Happened

The pattern is consistent across database history:

1. A new dominant workload appears (OLTP → analytics → documents → graphs → time series).
2. General-purpose systems can do it, but inefficiently.
3. Purpose-built systems optimize storage layout, indexing, compression, and query patterns for that workload.
4. Over time, some ideas flow back into general-purpose databases (e.g., time-series features in PostgreSQL, columnar storage everywhere).

InfluxDB is a product of that cycle: the explosion of metrics and telemetry data in the cloud/DevOps era created demand for a database that treated time as a first-class citizen and optimized for high-ingest, tag-filtered, time-range queries.

**In short:** Databases evolved from general-purpose relational systems to a diverse ecosystem of specialized stores. InfluxDB arose in the early 2010s because monitoring, IoT, and observability workloads had become large enough—and distinct enough—to justify a purpose-built time series database.
