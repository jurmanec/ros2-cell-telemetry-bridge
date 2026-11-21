# ADR 0005 — DynamoDB keys for plant telemetry

**Status:** Accepted (matches deployed `dev` handler; facility checked with [facility-demo.md](../how-to/facility-demo.md))  
**Date:** 2026-11-21  
**Applies to:** `schema_version` 2 (weld) and 3 (facility)  
**Sort key detail:** [0004-snapshot-sort-key.md](0004-snapshot-sort-key.md)

## Decision

One cloud item is one snapshot for one series. The DynamoDB primary key is a simple series id plus the snapshot timestamp.

| Choice | Lock |
|--------|------|
| Weld partition key (`pk`) | `cell_id` |
| Facility partition key (`pk`) | `asset_id` |
| Sort key (`sk`) | Snapshot UTC timestamp alone (nine fractional digits). See ADR 0004 |
| Site / hall on facility items | Attributes on the item (`site_id`, `hall_id`). Not part of `pk` |
| Composite `site#hall#asset` partition | Deferred |
| Date (or hour) bucket on the partition | Deferred |
| GSIs (hall, asset type, sparse alarms) | Deferred |
| TTL on raw points | Deferred |
| Version 1 cell samples | Unchanged: `pk` = `cell_id`; sort key remains the padded sample key from the v1 path |

Primary-key and secondary-index ideas still apply to time-series store design. This repo’s thin DynamoDB copy only needs per-asset (or per-cell) time-range access and idempotent replace on replay. Extra access patterns wait for a real reader.

## Context

A keys survey for plant telemetry in DynamoDB (and log-style stores such as Elasticsearch) recommended `pk = site#hall#asset` (optionally with a date bucket), `sk = stamp`, and optional GSIs for hall-wide, type-wide, and faulted-asset queries.

This repository stops at the reduced cloud copy. Weld and facility ingest already partition on `cell_id` / `asset_id`. No collector queries by hall, asset type, or alarm flag against DynamoDB today. ADR 0004 already locked the timestamp sort key so a second post of the same sample replaces the row. This ADR records the partition choice and the deferred parts of that survey.

## Options considered

| Option | Result |
|--------|--------|
| **`pk` = `asset_id` / `cell_id`, `sk` = timestamp** | Matches the deployed handler. Per-series time range and latest item stay simple. Hall and site remain filterable only by scanning attributes or by knowing the asset id. |
| `pk` = `site#hall#asset`, `sk` = stamp | Natural series identity from the keys note. No reader needs that composite today; would force a migrate or dual-write. |
| Time-bucketed `pk` (`…#YYYY-MM-DD`) | Bounds partition size for high volume. Demo rates do not need it. |
| GSIs for hall, type, or sparse alarms | Useful when a collector queries those shapes in DynamoDB. Deferred until that reader exists. |
| Elasticsearch-style document `_id` + field indexes | Out of scope for this store. The cloud floor remains DynamoDB ([ADR 0003](0003-cloud-floor.md)). |

## Consequences

- Facility items keep `pk = asset_id` and carry `site_id` / `hall_id` as attributes.
- Weld items keep `pk = cell_id`.
- Sort-key behavior stays entirely under ADR 0004.
- Changing to a composite or bucketed partition later is a new ADR and a data migration or dual-write plan.
- This ADR and ADR 0004 are the locks for DynamoDB keys on weld and facility items.

## Non-goals

- Rewriting existing table items under a new partition shape
- Adding GSIs or TTL in this change
- Changing the Influx tag model (local historian stays separate)
- Adopting Elasticsearch or another log store for the thin copy
