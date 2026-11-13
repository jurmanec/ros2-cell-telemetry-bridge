# ADR 0004 — Snapshot sort key

**Status:** Accepted (deployed to `dev` and checked 2026-11-13; see [facility-demo.md](../how-to/facility-demo.md))  
**Date:** 2026-11-13  
**Applies to:** `schema_version` 2 (weld) and 3 (facility)

## Decision

The DynamoDB sort key for a weld or facility snapshot is the snapshot timestamp alone.

| Choice | Lock |
|--------|------|
| Sort key | The UTC timestamp string, nine fractional digits: `2026-11-13T15:04:05.123456789Z` |
| Same asset, same timestamp | `PutItem` replaces the existing item |
| Partition key | Unchanged: `cell_id` for weld, `asset_id` for facility |
| Version 1 cell samples | Unchanged. The sort key stays padded seconds, nanoseconds, cycle, and a random suffix |
| Rows already stored | Left in place under `timestamp` plus `#` and eight random hex characters |

The bridge already formats the timestamp this way. The ingest validator must require the same nine digits so a shorter fraction cannot sort out of time order.

## Context

Weld and facility items are written with `sk = timestamp#<8 random hex>`. A second post of the same snapshot inserts a new row. A replay of a selected snapshot, including a later store-and-forward drain, would be counted twice by a collector that counts items.

One asset has one reading at one sample time. The local historian already uses that identity: the Influx point time is the ROS stamp, and a second line for the same asset and stamp overwrites the fields. The cloud item should be the same point.

Key patterns for this hall are surveyed in [telemetry-keys-discussion.md](../v3/arch-discussion/telemetry-keys-discussion.md). This ADR takes the timestamp sort key from that note and leaves the rest of its schema alone.

## Options considered

| Option | Result |
|--------|--------|
| **Timestamp alone** | A retry or a corrected post of the same sample replaces the row. String order matches time order when the fraction is nine digits. |
| Random suffix (current) | Every post inserts a row. A replay is a second item. |
| Timestamp plus a hash of the body | An identical retry collapses. A corrected body at the same time becomes a second item. |
| Timestamp plus a publisher sequence | Keeps two readings that share a nanosecond. Nothing in the weld or facility publishers emits that pair. A sequence minted in the Lambda would change on every post. |
| `site#hall#asset` partition, date buckets, secondary indexes | Access patterns from the keys note. No reader queries by hall, asset type, or alarm flag. |

## Consequences

- `snapshot_sort_key` returns the timestamp and stops appending a random suffix.
- The facility and weld timestamp check requires nine fractional digits.
- A collector that counts items counts one snapshot once.
- A range query on `sk` is a time range for items written after this change. Items already stored keep the old key until they age out or are cleared.
- Version 1 `sample_sort_key` is not part of this change. Dropping its random suffix is the same idea. Dropping the cycle would fold two samples that share a timestamp into one item.

## Non-goals

- Changing the partition key
- A date bucket on the partition
- A secondary index
- Time-to-live on the table
- Rewriting items already stored under the random suffix
