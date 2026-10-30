# Shipped vs deferred (v1)

**Date:** 2026-07-15  
**Authority:** [CHARTER.md](CHARTER.md), [MILESTONES.md](MILESTONES.md)

**Scope:** v1 is an **industrial-shaped edge→cloud telemetry bridge**, not plant robotics or ML.

---

## Shipped

| Area | What |
|------|------|
| **Edge** | ROS 2 **Jazzy** / **C++20** `rclcpp`: `cell_sim` publisher, custom `CellTelemetry`, Docker/Compose on macOS |
| **Bridge** | `cell_bridge`: subscribe → JSON batches → HTTPS POST; retries documented; `api_key` → `x-api-key` |
| **Local stub** | Python HTTP ingest for local demos (`run_m2_stack.sh`) |
| **Cloud** | AWS API Gateway HTTP API + Python 3.12 Lambda + DynamoDB (`dev`); `useDotenv`; Serverless |
| **Rules / alert** | In-batch `fault` (+ optional temp threshold); observable side effect = CloudWatch **`CELL_ALERT`** (log event = alert channel for v1) |
| **Contracts / ADRs** | Cell telemetry, HTTP batch, ingest events; ADRs 0001–0003 |
| **Smoke** | `cloud/scripts/smoke_ingest.py` — HTTP → Dynamo → `CELL_ALERT` |
| **Docs** | Charter, architecture, milestones, cloud/edge READMEs |

---

## Deferred (explicit)

| Item | Notes |
|------|--------|
| Real sensors, actuators, robot models, motion/control | Simulation only in v1 |
| Learned / historical baselines, ONNX serving | Follow-on |
| SNS / Slack / PagerDuty notify | Log alerts only in v1 |
| Operator SPA / rich dashboard | Deferred |
| MQTT / dual egress | HTTPS only ([ADR 0002](decisions/0002-bridge-transport.md)) |
| Fleet / multi-cell / digital twin / MES | Not scheduled |
| Pure-JSON Lambda log lines, CW field indexes | Nice-to-have; smoke matches embedded JSON |
| GitHub Actions CI (ROS + AWS) | Documented one-command smoke instead |
| Full CI/CD and ops tooling | Cloud kept intentionally thin |

---

## Open follow-ons

1. **Anomaly model** — train or fit a small model on the Dynamo time series; deploy (e.g. ONNX); observe next to or instead of pure rules; fail gracefully.  
2. **Optional polish** — notify channel, Compose profile pointing at IngestUrl, Insights dashboards.  
3. **Robotics depth** — richer cell behavior or real hardware *only* if the charter is deliberately revised.

Treat listed items as follow-ons, not silent growth of v1.
