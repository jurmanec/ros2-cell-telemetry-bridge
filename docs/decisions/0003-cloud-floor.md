# ADR 0003 — Cloud floor (M3)

**Status:** Accepted (deployed and verified on AWS `dev`, 2026-07-15)  
**Date:** 2026-07-14  
**Milestone:** M3

## Decision

Build a **thin, deployable AWS** cloud path that accepts the existing HTTP batch contract, persists samples, evaluates a **rule-based** health check, and records alerts via **structured logs** (notify channels deferred).

| Choice | Lock |
|--------|------|
| Provider | **AWS** (`dev` stage deployable) |
| Ingest | **API Gateway HTTP API** + **Lambda** |
| Lambda language | **Python 3.12+** |
| Store | **DynamoDB** (on-demand) |
| Rule evaluation | **In-process** in the ingest Lambda (persist, then evaluate) |
| Alert side effect | **Structured alert log** (CloudWatch); optional metric filter OK |
| Auth | **API key** (API Gateway) or equivalent shared secret for demo |
| Wire contract | Reuse [telemetry-batch-http.md](../contracts/telemetry-batch-http.md); HTTPS to API GW |
| IaC | **Serverless Framework** (SAM acceptable if scaffold friction favors it) |

## Context

M3 needs a minimal deployable cloud path: real ingest, persistence, and a rule alert — not full ops maturity and not an operator console.

Edge already POSTs JSON batches (M2). Cloud should swap behind the same path/body so `cell_bridge` only changes `ingest_url` (+ API key header when required).

Prefer **Python** for the cloud handler while keeping **C++** on ROS nodes. Deploy to **real AWS**, not LocalStack-first.

Alert options considered: log-only vs SNS/Slack. Log-only meets “observable” for v1 without adding a notify product surface.

## Options considered (summary)

| Topic | Chosen | Rejected / deferred for M3 |
|-------|--------|----------------------------|
| Store | DynamoDB | Timestream (richer TS, more novelty); keep schema simple |
| Rules | Sync in ingest Lambda | SQS/async worker (add if latency/scale needs it) |
| Alert | Structured logs | SNS/Slack (optional later; not M3 Done) |
| Lambda language | Python | Java (continuity) — acceptable alternate; Python preferred for this slice |
| Local cloud emulator | Skip as default | LocalStack — optional only if AWS access blocked |

## Rule v1 (implementation hint)

- Primary: any sample in the batch with `fault: true` → emit alert log (cell_id, cycle, reason).  
- Optional add-on: `temp_c` above a configured threshold.  
- Not in M3: learned baselines / ML.

## Consequences

- Add `cloud/` (or equivalent) with IaC + Python ingest handler.  
- Persist each sample (or batch with addressable items) in DynamoDB keyed for cell + time.  
- On rule match, log a clear alert event — field names in [cloud-ingest-events.md](../contracts/cloud-ingest-events.md) (M4 smoke).  
- Configure bridge for cloud mode: HTTPS URL + API key; keep Compose stub for local edge-only demos.  
- SNS/Slack, rich CD, SPA, Timestream, MQTT — **out of M3** unless a later ADR reopens them.  
- Charter open items for cloud services / alert channel → locked by this ADR.

## Non-goals (M3)

- Advanced dashboards, alarms, multi-stage CD  
- Operator UI  
- Exactly-once / full audit store  
- Multi-cell tenancy beyond `cell_id` on the batch  
