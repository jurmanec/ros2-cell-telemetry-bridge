# Local Historian

A **local historian** (or plant historian / edge historian) is a time-series database that runs **on-site**, close to the machines or cells, and stores operational data locally.

## What It Does

- Continuously records process values, equipment status, alarms, and production counts over time.
- Keeps a high-resolution history (seconds, minutes, or longer) even if the connection to the cloud or central system is lost.
- Allows local queries, trending, and short-term analysis without depending on the network.

## Typical Characteristics

| Aspect              | Local Historian                          | Cloud / Central System                  |
|---------------------|------------------------------------------|-----------------------------------------|
| Location            | On-premises or at the edge               | Cloud or central data center            |
| Resolution          | Often high (1 s or faster)               | Usually lower (summarized)              |
| Retention           | Days to months                           | Months to years                         |
| Availability        | Works even if internet/WAN is down       | Depends on connectivity                 |
| Common tools        | OSIsoft PI, Canary, Ignition, InfluxDB, TimescaleDB, vendor historians | AWS Timestream, Azure Data Explorer, cloud versions of the above |

## Why It Exists in Manufacturing

- Many cells and lines need local trending and troubleshooting even when the plant network or cloud link is unavailable.
- High-frequency data (joint currents, arc parameters, sensor readings) can be stored locally at full rate, then only summaries or exceptions are sent upstream.
- It reduces bandwidth and cloud cost while still giving operators and engineers immediate historical context on the plant floor.

## Summary

A local historian is the on-site system of record for recent operational data, sitting between the cell/PLC and any higher-level cloud monitoring platform.
