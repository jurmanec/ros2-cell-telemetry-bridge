# Charter — Industrial Cell Telemetry Bridge (P3)

**Status:** v1 frozen (2026-07-15)  
**Portfolio:** P3 (follows P0 event-driven middleware, P1 serverless orchestration, P2 anomaly monitoring)  
**Last updated:** 2026-07-15

v1 milestones **M0–M5** are complete. Treat this charter as closed for scope expansion unless you explicitly reopen it for a follow-on slice (e.g. **P3.1**). What shipped vs deferred: [SHIPPED.md](SHIPPED.md).

---

## Purpose

Build a freezeable portfolio tracer that connects a **simulated manufacturing cell** on a **C++ / ROS2** edge to a **thin but real cloud path** for ingest, persistence, and rule-based health alerts.

**Career signal:** Advance beyond Java/cloud-only demos into **systems programming, robotics (ROS2), and industrial edge→cloud**—while reusing P0–P2 engineering practices (contracts, smoke, observability basics, explicit deferrals).

**Portfolio posture:** P0–P2 were effectively demoable clones of systems shaped by prior production experience. **P3 is the first project that signals a new / aspired direction** rather than re-demonstrating familiar ground. Judge v1 by an honest, demoable learning tracer—not by parity with a plant system already shipped at work.

**Honest read of v1:** This is an **edge→cloud data-path bridge** (ROS 2 / `rclcpp` scaffolding + cloud-native ingest). It is **not** a demonstration of real sensors, control, actuators, robot models, or trained ML. Expand later for deeper robotics and/or MLOps.

**Personal bridge (non-scope):** The domain reflects interest in real-economy systems, industrial capability, and resilient infrastructure. Motivation informs *why this exists*; it does not expand v1 into smart-city, structural health, or multi-site platforms.

---

## One-liner

A simulated manufacturing cell publishes sensor/state telemetry from a **C++ ROS2** edge; a bridge forwards batches to the cloud; the cloud stores time series and evaluates **rule-based** health. Optional ML is a later slice (**P3.1**), not v1.

---

## Emphasis lock (v1)

| Layer | Investment |
|-------|------------|
| **Primary** | Modern C++ (`rclcpp`) cell nodes, idiomatic edge design, local sim quality |
| **Cloud** | Credibility floor only: deployable ingest → durable series → one rule-alert path → structured logs + smoke |
| **Not v1** | P0-parity multi-stage CD/alarm taxonomy, operator SPA, fleet, digital twin, learned models |

**Language:** Edge implementation language is **C++**. Python may appear as launch/smoke/CI glue only—not as the primary node runtime.

---

## In scope (v1 MVP) — shipped

1. **Edge** — ROS2 nodes simulating cell metrics; publish on topics; no real hardware.  
2. **Bridge** — ROS2 → HTTPS JSON batches into cloud ingest with wire contracts.  
3. **Cloud** — Persist time series; rule-based evaluation; alert side effect = **CloudWatch structured log** (`CELL_ALERT`).  
4. **Engineering bar** — Contracts, unit tests, automated smoke (ingest → persist → log alert), shipped-vs-deferred list.

### Definition of done (v1) — met

- Demoable path: cell sim → cloud store → rule alert observable (CW event).  
- Docs state what shipped vs deferred — [SHIPPED.md](SHIPPED.md).  
- Happy-path unit + smoke (`cloud/scripts/smoke_ingest.py`).  
- Emphasis lock respected.

---

## Out of scope (v1)

- Learned / historical baselines and model serving (**P3.1 MLOps**)  
- Full digital twin; multi-robot / multi-cell fleet  
- Operator SPA or rich dashboard  
- Production ROS deploy / real plant hardware  
- Warehouse, infrastructure SHM, or energy-optimization as primary domains  
- Extending P0 or P2 as products; retail/order vocabulary as the story  
- SNS/Slack notify (log event is the v1 alert channel)

---

## Demo surface (v1)

**Default:** Terminal/ROS evidence + automated smoke + structured logs (`CELL_ALERT`).  
**Deferred:** Web UI. P2 already demonstrates an operator console.

---

## Suggested follow-on (P3.1)

After this freeze: small anomaly model on the Dynamo series (e.g. ONNX), deploy, observe, fail gracefully—targets AI/ML Engineering / MLOps without rewriting the robotics tracer. Optional parallel track: deeper cell/robotics fidelity if that is the primary career bet.

---

## Alternatives considered (not primary)

| Option | Why not v1 primary |
|--------|-------------------|
| Extend P2 (learned baselines / console polish) | Weak career-signal change |
| Pure MLOps model-serving demo | Weak C++/ROS2 and industrial edge story |
| Another Java serverless workflow | Repeats P0/P1 |
| Warehouse station as primary metaphor | Strong market, weaker differentiation vs fulfillment-shaped past work |
| Infrastructure / SHM as primary metaphor | Strong personal narrative, weaker ROS2/software-hiring signal |

---

## Relationship to prior portfolio

- **Reuse:** Scope freezes, contracts-first instincts, smoke gates, structured logs, ADRs where decisions matter, honest deferral lists.  
- **Do not reuse:** Order/BOPIS/retail lifecycle as the primary domain story.  
- **Differentiate from P2:** P2 = domain-agnostic monitoring monolith + console. P3 = **industrial cell edge (C++/ROS2) + bridge + thin cloud**.

---

## Locked decisions (v1)

- Bridge transport — **HTTPS POST (JSON batches)** ([ADR 0002](decisions/0002-bridge-transport.md)); MQTT deferred  
- Cloud — **AWS API GW + Python Lambda + DynamoDB + log alerts** ([ADR 0003](decisions/0003-cloud-floor.md))  
- C++ — **C++20** ([ADR 0001](decisions/0001-cpp-standard.md))  
- ROS 2 — **Jazzy** ([ros2-rclcpp.md](spikes/ros2-rclcpp.md))  
- Alert channel — **CloudWatch structured logs**; SNS/Slack deferred  

---

## How to use this document (post-freeze)

1. Prefer [SHIPPED.md](SHIPPED.md) and the root [README](../README.md) for demo and scope.  
2. Do not expand v1 scope without an explicit charter revision.  
3. Keep vocabulary industrial/ops (**cell, line, station, fault, cycle**).  
4. Do not reopen P0/P2 unless explicitly asked.  
5. Next build work should be a **named follow-on** (P3.1 or polish), not silent growth of v1.
