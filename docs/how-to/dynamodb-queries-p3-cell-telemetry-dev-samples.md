# DynamoDB Useful Queries for `p3-cell-telemetry-dev-samples`

**Table:** `p3-cell-telemetry-dev-samples`  
**Partition key:** `pk` (String)  
**Sort key:** `sk` (String) — starts with ISO-8601 timestamp (e.g. `2026-11-07T19:15:42.620480047Z#...`)

These queries are designed for the **AWS Management Console → DynamoDB → PartiQL editor**.

Because `sk` begins with an ISO timestamp, it sorts chronologically as a string.

---

## 1. Most recent 20 records for a cell

```sql
SELECT *
FROM "p3-cell-telemetry-dev-samples"
WHERE pk = 'WELD_CELL_07'
ORDER BY sk DESC
```

Set **Limit** = `20` in the request parameters.

---

## 2. Records for a cell in a time range

```sql
SELECT *
FROM "p3-cell-telemetry-dev-samples"
WHERE pk = 'WELD_CELL_07'
  AND sk BETWEEN '2026-11-07T19:00:00' AND '2026-11-07T20:00:00Z~'
ORDER BY sk DESC
```

> The trailing `~` helps include the full range because of the `#uuid` suffix on the sort key.

Alternative for a specific hour:

```sql
SELECT *
FROM "p3-cell-telemetry-dev-samples"
WHERE pk = 'WELD_CELL_07'
  AND begins_with(sk, '2026-11-07T19:')
ORDER BY sk DESC
```

---

## 3. Records that have alarms (non-empty `alarms` list)

### For one cell (recommended)

```sql
SELECT *
FROM "p3-cell-telemetry-dev-samples"
WHERE pk = 'WELD_CELL_07'
  AND size(alarms) > 0
ORDER BY sk DESC
```

### Across all cells (full table scan — acceptable given the small table size)

```sql
SELECT *
FROM "p3-cell-telemetry-dev-samples"
WHERE size(alarms) > 0
```

---

## 4. Other useful filters

### By status

```sql
SELECT *
FROM "p3-cell-telemetry-dev-samples"
WHERE pk = 'WELD_CELL_07'
  AND status = 'RUNNING'
ORDER BY sk DESC
```

### By program

```sql
SELECT *
FROM "p3-cell-telemetry-dev-samples"
WHERE pk = 'WELD_CELL_07'
  AND program = 'DOOR_FRAME_WELD_V4'
ORDER BY sk DESC
```

### High cycle time or scrap

```sql
SELECT *
FROM "p3-cell-telemetry-dev-samples"
WHERE pk = 'WELD_CELL_07'
  AND (cycle_time_last_s > 60 OR parts_scrap_shift > 0)
ORDER BY sk DESC
```

### Alarms in a time window

```sql
SELECT *
FROM "p3-cell-telemetry-dev-samples"
WHERE pk = 'WELD_CELL_07'
  AND sk BETWEEN '2026-11-07T18:00:00' AND '2026-11-07T20:00:00Z~'
  AND size(alarms) > 0
ORDER BY sk DESC
```

---

## Classic Query UI alternative

In **Explore table items → Query**:

1. Partition key: `pk` = `WELD_CELL_07`
2. Optional sort key condition (`BETWEEN` or `begins_with`)
3. Set **Scan index forward** = **false** (newest first)
4. Set **Limit** = `20`
5. Put non-key filters (e.g. `size(alarms) > 0`, `status = 'RUNNING'`) in the Filter expression

---

## Tips

- Always prefer queries that specify the partition key (`pk`) — they are efficient.
- `ORDER BY sk DESC` works cleanly when filtering on a single partition key.
- The current key design is well-suited for “most recent N” and time-range access patterns.