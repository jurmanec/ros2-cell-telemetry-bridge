# v1 milestones

**Status:** v1 complete / frozen (2026-07-15)  
**Date:** 2026-07-13 (freeze 2026-07-15)  
**Authority:** [CHARTER.md](CHARTER.md), [ARCHITECTURE.md](ARCHITECTURE.md), [SHIPPED.md](SHIPPED.md)

Ordered thin slices toward charter **definition of done**. Prefer finishing one milestone before starting the next. Resolve TBDs only when a milestone needs them (short ADR if the choice matters).

Disposable spike under `spike/ros2/` stays historical; v1 work uses **ROS 2 Jazzy**.

---

## Overview

| ID | Milestone | Outcome |
|----|-----------|---------|
| **M0** | Repo / edge scaffold | ✅ Jazzy `rclcpp` cell package builds and publishes in Docker |
| **M1** | Telemetry contract | ✅ Documented wire shape; sim emits structured cell metrics |
| **M2** | Bridge | ✅ HTTPS JSON batches to stub ingest ([ADR 0002](decisions/0002-bridge-transport.md)) |
| **M3** | Cloud floor | ✅ AWS ingest + DynamoDB + `CELL_ALERT` logs ([ADR 0003](decisions/0003-cloud-floor.md)) |
| **M4** | Smoke + logs | ✅ `smoke_ingest.py` → HTTP + Dynamo + `CELL_ALERT` |
| **M5** | Docs freeze | ✅ README demo path; [SHIPPED.md](SHIPPED.md); charter v1 frozen |

---

## M0 — Repo / edge scaffold

**Status:** Done (2026-07-13)

**Goal:** Replace spike-quality setup with the real workspace layout (still edge-only).

**Done when:**
- ~~Colcon workspace + Docker (Jazzy) build/run path documented~~ — [`edge/`](../edge/)
- ~~At least one `rclcpp` cell publisher runs; topic visible via `ros2 topic echo`~~ — verified
- ~~C++ standard chosen~~ — **C++20** ([ADR 0001](decisions/0001-cpp-standard.md))

**Out:** Bridge, cloud, custom msg mandatory (String OK temporarily).

---

## M1 — Telemetry contract

**Status:** Done (2026-07-13)

**Goal:** Stable edge vocabulary for downstream bridge/cloud.

**Done when:**
- ~~Contract doc~~ — [docs/contracts/cell-telemetry.md](contracts/cell-telemetry.md)
- ~~Sim publishes fields~~ — `cell_msgs/msg/CellTelemetry` on `cell/telemetry`
- ~~Unit test(s)~~ — `cell_sim` `test_telemetry_sample` (run in image build)

**Out:** Cloud persistence; production-grade schema evolution.

---

## M2 — Bridge

**Status:** Done (2026-07-13)

**Goal:** Data leaves the ROS graph toward cloud ingest.

**Transport:** **HTTPS POST** (JSON batches) — [ADR 0002](decisions/0002-bridge-transport.md). MQTT/dual egress deferred.

**Done when:**
- ~~Forwarder~~ — `cell_bridge` + Compose stack ([`edge/`](../edge/)); contract [telemetry-batch-http.md](contracts/telemetry-batch-http.md)
- ~~Transport recorded~~ — ADR 0002
- ~~Failure/retry docs~~ — [bridge-http-retry.md](bridge-http-retry.md)

**Out:** Full IoT platform; multi-site routing; MQTT in v1/M2.

---

## M3 — Cloud floor

**Status:** Done (2026-07-15)

**Goal:** Credibility-floor cloud path (thin but real).

**Locks:** [ADR 0003](decisions/0003-cloud-floor.md) — AWS API GW + Python Lambda + DynamoDB; log alerts; real `dev` deploy; reuse HTTP batch contract.

**Code:** [`cloud/`](../cloud/) — Serverless + `src/handler.py`. Bridge `api_key` → `x-api-key`.

**Done when:**
- ~~Deployable ingest accepts batches and persists time series~~ — AWS `dev` + DynamoDB rows (curl and publisher→bridge)
- ~~Rule-based evaluator can raise an alert on a known sim condition~~ — `fault` → `"alerts":1` + CloudWatch `"event":"CELL_ALERT"` (also `INGEST_OK`)
- ~~Alert side effect observable~~ — structured JSON in Lambda log lines (timestamp/request-id wrapper is normal)
- ~~Cloud service choices recorded~~ — ADR 0003

**Out:** P0-parity CD/alarms; SPA; learned baselines; SNS/Slack in M3 Done.

---

## M4 — Smoke + logs

**Status:** Done (2026-07-15)  
**Goal:** Repeatable proof of the full chain.

**Locks (v1 smoke):**
- **Hero path:** HTTP ingest against AWS `dev` (unique `cell_id` / cycle per run) → assert persist + alert. Event shapes: [cloud-ingest-events.md](contracts/cloud-ingest-events.md).
- **Asserts:** HTTP `202` with `"status":"ok"` and `"alerts"≥1` for a `fault:true` sample; **DynamoDB** item for that `cell_id`; CloudWatch message containing `"event":"CELL_ALERT"`.
- **Done bar:** One documented command (script under `cloud/` or `scripts/`) using `.env` / AWS credentials — **CI optional**; not required for M4 Done.
- **Edge hop:** Publisher→bridge→cloud already verified in M3. M4 smoke need not rebuild the ROS stack; document the bridge recipe in README as the edge path. (Optional stretch: Compose override pointing at IngestUrl.)
- **Log format:** Current Lambda-wrapped JSON lines are acceptable; pure-JSON message bodies are a nice-to-have, not a blocker.

**Code:** [`cloud/scripts/smoke_ingest.py`](../cloud/scripts/smoke_ingest.py) — `cd cloud && .venv/bin/python scripts/smoke_ingest.py` (or `npm run smoke`).

**Done when:**
- ~~One automated smoke journey covering ingest → persist → alert~~ — script asserts HTTP + Dynamo + `CELL_ALERT`
- ~~Structured log contract documented~~ — [cloud-ingest-events.md](contracts/cloud-ingest-events.md)
- ~~Happy path green via that one-command run~~ — verified `SMOKE PASSED` on AWS `dev`

**Out:** Exhaustive negative-path matrix; field indexes / Powertools dashboards; mandatory GitHub Actions ROS+AWS CI; SNS/Slack.

---

## M5 — Docs freeze (v1)

**Status:** Done (2026-07-15)

**Goal:** Portfolio-ready pause, P0/P2 style.

**Done when:**
- ~~README: purpose, how to demo, link to charter/architecture~~ — root [README.md](../README.md)
- ~~Explicit shipped vs deferred list~~ — [SHIPPED.md](SHIPPED.md)
- ~~Charter status updated~~ — **v1 frozen** ([CHARTER.md](CHARTER.md))
- ~~Open follow-ons listed~~ — P3.1 MLOps, optional polish, robotics depth in SHIPPED + README

**Out:** Starting P3.1 unless explicitly chosen next.

---

## Parking lot (not v1 milestones)

- Operator SPA / thin read UI  
- Custom DDS tuning, multi-machine ROS  
- Fleet / multi-cell  
- ONNX / learned anomaly (**P3.1**)  

---

## Next action

**v1 frozen.** Choose an explicit follow-on when ready (see [SHIPPED.md](SHIPPED.md)): **P3.1 MLOps**, optional polish, or deeper robotics — do not silently expand v1.
