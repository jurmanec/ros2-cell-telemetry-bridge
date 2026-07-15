# Shipped vs deferred (v1 freeze)

**Date:** 2026-07-15  
**Authority:** [CHARTER.md](CHARTER.md), [MILESTONES.md](MILESTONES.md)

Honest scope: v1 demonstrates an **industrial-shaped edge→cloud telemetry bridge**, not plant robotics or ML.

---

## Shipped

| Area | What |
|------|------|
| **Edge** | ROS 2 **Jazzy** / **C++20** `rclcpp`: `cell_sim` publisher, custom `CellTelemetry`, Docker/Compose on macOS |
| **Bridge** | `cell_bridge`: subscribe → JSON batches → HTTPS POST; retries documented; `api_key` → `x-api-key` |
| **Local stub** | Python HTTP ingest for M2 demos (`run_m2_stack.sh`) |
| **Cloud** | AWS API Gateway HTTP API + Python 3.12 Lambda + DynamoDB (`dev`); `useDotenv`; Serverless |
| **Rules / alert** | In-batch `fault` (+ optional temp threshold); observable side effect = CloudWatch **`CELL_ALERT`** (log event = alert channel for v1) |
| **Contracts / ADRs** | Cell telemetry, HTTP batch, ingest events; ADRs 0001–0003 |
| **Smoke** | `cloud/scripts/smoke_ingest.py` — HTTP → Dynamo → `CELL_ALERT` |
| **Docs** | Charter, architecture, milestones, cloud/edge READMEs |

---

## Deferred (explicit)

| Item | Notes |
|------|--------|
| Real sensors, actuators, robot models, motion/control | Out of charter v1; simulation only |
| Learned / historical baselines, ONNX serving | **P3.1** |
| SNS / Slack / PagerDuty notify | Log alerts only in v1 |
| Operator SPA / rich dashboard | P2 already covers console-shaped demos |
| MQTT / dual egress | HTTPS only ([ADR 0002](decisions/0002-bridge-transport.md)) |
| Fleet / multi-cell / digital twin / MES | Parking lot |
| Pure-JSON Lambda log lines, CW field indexes | Nice-to-have; smoke matches embedded JSON |
| GitHub Actions CI (ROS + AWS) | Documented one-command smoke instead |
| P0-parity CD / alarm taxonomy | Cloud kept at credibility floor |

---

## Open follow-ons

1. **P3.1 — MLOps** — train or fit a small anomaly model on the Dynamo time series; deploy (e.g. ONNX); observe next to or instead of pure rules; fail gracefully. Dynamo exists partly as that series runway.  
2. **Optional v1.x polish** — notify channel, Compose profile pointing at IngestUrl, Insights dashboards.  
3. **Robotics depth** — richer cell behavior or real hardware *only* if charter is deliberately reopened.

Do **not** start P3.1 by inflating v1; freeze stands until you choose a follow-on slice.
