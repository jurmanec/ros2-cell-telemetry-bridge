# Contract — Cloud ingest log events (M3/M4)

**Status:** Active  
**Date:** 2026-07-15  
**Authority:** [ADR 0003](../decisions/0003-cloud-floor.md), [`cloud/src/handler.py`](../../cloud/src/handler.py)  
**Milestone:** M4 smoke asserts these events (and Dynamo persistence)

---

## Dual sinks (product shape)

Per successful validated ingest of a batch:

| Sink | Role |
|------|------|
| **DynamoDB** | Append-only historical samples (`pk=cell_id`, time-ordered `sk`) — demo proof + runway for later analytics / models |
| **CloudWatch Logs** | Structured JSON events — rule alerts and ingest outcome for smoke/debug |

Rules evaluate the **in-memory batch** after persist; they do not re-read Dynamo. Alert side effect for v1 is **log-only** (SNS/Slack deferred).

---

## Log envelope (runtime)

Lambda’s default Python logger wraps each line. Typical shape:

```text
TIMESTAMP  REQUEST_ID  LEVEL  {"event":"…", …}
```

Smoke and Insights should match on the **JSON object** inside the message (e.g. substring `"event":"CELL_ALERT"`), not assume the whole line is JSON.

Optional later (not required for M4 Done): emit pure-JSON messages so Insights can field-query without parsing wrappers.

---

## Events

All payloads are single-line JSON objects with a string **`event`** discriminator.

### `CELL_ALERT`

Emitted once per sample that trips a rule (`fault` and/or optional `temp_c` threshold).

| Field | Type | Notes |
|-------|------|--------|
| `event` | string | Always `CELL_ALERT` |
| `alert` | bool | Always `true` |
| `cell_id` | string | |
| `cycle` | int | |
| `reason` | string | Comma-joined: `fault`, `temp_c_threshold` |
| `temp_c` | number | |
| `vibration` | number | |
| `fault` | bool | |
| `stamp_sec` | int | |
| `stamp_nanosec` | int | |

### `INGEST_OK`

Emitted once after a batch is persisted and rules run.

| Field | Type | Notes |
|-------|------|--------|
| `event` | string | Always `INGEST_OK` |
| `cell_id` | string | |
| `accepted` | int | Samples written |
| `alerts` | int | Count of `CELL_ALERT`s for this batch |

### `INGEST_UNAUTHORIZED`

Emitted when `x-api-key` is missing or does not match `INGEST_API_KEY`.

| Field | Type | Notes |
|-------|------|--------|
| `event` | string | Always `INGEST_UNAUTHORIZED` |

---

## HTTP body vs logs

Successful ingest also returns JSON `{"status":"ok","accepted":N,"alerts":N}` (`202`).  
`alerts` in the response should match the number of `CELL_ALERT` log lines for that request. Smoke may assert either or both.

---

## Insights (example)

```sql
fields @timestamp, @message
| filter @message like /"event":"CELL_ALERT"/
| sort @timestamp desc
| limit 20
```

---

## Out of scope (this contract)

- CloudWatch field indexes / metric filters (optional polish)  
- Bridge ROS log schema (edge hop; separate from cloud events)  
- SNS/Slack / EMF / OpenTelemetry  
