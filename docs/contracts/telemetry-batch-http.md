# Contract — Telemetry HTTP batch ingest

**Status:** Active (M2)  
**Date:** 2026-07-13  
**Transport:** HTTP(S) POST — [ADR 0002](../decisions/0002-bridge-transport.md)  
**ROS source:** [cell-telemetry.md](cell-telemetry.md) (`cell/telemetry`)

---

## Endpoint

| Item | Value |
|------|--------|
| Method | `POST` |
| Path | `/v1/telemetry/batch` |
| Content-Type | `application/json` |
| M2 stub base URL | `http://ingest:8080` (Docker Compose service name) |
| Success | `202 Accepted` or `200 OK` with JSON body `{"status":"ok","accepted":N}` (cloud may also return `"alerts":N`) |
| Client errors | `4xx` — do not retry as-is without fixing payload |
| Server/transient | `5xx` / connection failure — bridge may retry |
| Auth (M3 cloud) | Header **`x-api-key`** required; local M2 stub does not enforce auth |

TLS: **not required for M2 stub** (plain HTTP). Production/M3 should use HTTPS; same path and body.

---

## Request body

```json
{
  "schema_version": 1,
  "cell_id": "cell-001",
  "samples": [
    {
      "stamp_sec": 1783967375,
      "stamp_nanosec": 11192632,
      "cycle": 8,
      "temp_c": 44.0,
      "vibration": 0.12,
      "fault": false
    }
  ]
}
```

| Field | Type | Notes |
|-------|------|--------|
| `schema_version` | number | `1` for v1 |
| `cell_id` | string | Logical cell identity (param on bridge; default `cell-001`) |
| `samples` | array | One or more samples from `CellTelemetry` |
| `samples[].stamp_sec` / `stamp_nanosec` | number | From ROS `stamp` |
| `samples[].cycle` | number | uint64 |
| `samples[].temp_c` | number | °C |
| `samples[].vibration` | number | Sim units |
| `samples[].fault` | boolean | |

Empty `samples` arrays should not be POSTed.

---

## Bridge behavior (M2)

| Behavior | Default |
|----------|---------|
| Subscribe | `cell/telemetry` (`cell_msgs/msg/CellTelemetry`) |
| Flush | When batch size ≥ **5** **or** **1.0 s** since last successful flush attempt |
| Timeout | Connect/request ~**2 s** |
| Retry | Up to **3** attempts; brief backoff (~200 ms, ~500 ms) on connection error or 5xx |
| On 4xx | Log error; drop batch (no poison-loop retry) |
| `api_key` param | If set, bridge sends `x-api-key` (required for M3 cloud; empty for local stub) |

Details: [bridge-http-retry.md](../bridge-http-retry.md)

---

## Example (curl against stub)

```bash
curl -sS -X POST http://localhost:8080/v1/telemetry/batch \
  -H 'Content-Type: application/json' \
  -d '{"schema_version":1,"cell_id":"cell-001","samples":[{"stamp_sec":1,"stamp_nanosec":0,"cycle":1,"temp_c":40.5,"vibration":0.12,"fault":false}]}'
```
