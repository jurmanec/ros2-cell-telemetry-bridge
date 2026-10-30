# Charter — Industrial Cell Telemetry Bridge

**Status:** v1 complete (2026-07-15)  
**Last updated:** 2026-07-15

This document defines **product scope** for this repository. v1 milestones **M0–M5** are complete. Treat scope as closed unless you explicitly revise this charter for a follow-on slice. What shipped vs deferred: [SHIPPED.md](SHIPPED.md).

---

## Purpose

Connect a **simulated manufacturing cell** on a **C++ / ROS 2** edge to a **minimal but real cloud path** for ingest, persistence, and rule-based health alerts.

**Scope honesty:** v1 is an **edge → cloud telemetry bridge** (ROS 2 / `rclcpp` scaffolding plus cloud-native ingest). It is **not** real sensors, control, actuators, robot models, or trained ML. Deeper robotics and optional ML can be follow-ons.

---

## One-liner

A simulated manufacturing cell publishes sensor/state telemetry from a **C++ ROS 2** edge; a bridge forwards batches to the cloud; the cloud stores time series and evaluates **rule-based** health. Optional ML is a later slice, not v1.

---

## Emphasis lock (v1)

| Layer | Investment |
|-------|------------|
| **Primary** | Modern C++ (`rclcpp`) cell nodes, idiomatic edge design, local sim quality |
| **Cloud** | Minimal deployable path: ingest → durable series → one rule-alert path → structured logs + smoke |
| **Not v1** | Multi-stage CD / rich alarm taxonomy, operator SPA, fleet, digital twin, learned models |

**Language:** Edge implementation language is **C++**. Python may appear as launch/smoke/CI glue only—not as the primary node runtime.

---

## In scope (v1 MVP) — shipped

1. **Edge** — ROS 2 nodes simulating cell metrics; publish on topics; no real hardware.  
2. **Bridge** — ROS 2 → HTTPS JSON batches into cloud ingest with wire contracts.  
3. **Cloud** — Persist time series; rule-based evaluation; alert side effect = **CloudWatch structured log** (`CELL_ALERT`).  
4. **Engineering bar** — Contracts, unit tests, automated smoke (ingest → persist → log alert), shipped-vs-deferred list.

### Definition of done (v1) — met

- Demoable path: cell sim → cloud store → rule alert observable (CW event).  
- Docs state what shipped vs deferred — [SHIPPED.md](SHIPPED.md).  
- Happy-path unit + smoke (`cloud/scripts/smoke_ingest.py`).  
- Emphasis lock respected.

---

## Out of scope (v1)

- Learned / historical baselines and model serving  
- Full digital twin; multi-robot / multi-cell fleet  
- Operator SPA or rich dashboard  
- Production ROS deploy / real plant hardware  
- Warehouse, structural health monitoring, or energy-optimization as primary domains  
- SNS/Slack notify (log event is the v1 alert channel)

---

## Demo surface (v1)

**Default:** Terminal/ROS evidence + automated smoke + structured logs (`CELL_ALERT`).  
**Deferred:** Web UI.

---

## Suggested follow-ons

- Small anomaly model on the Dynamo series (e.g. ONNX): deploy, observe, fail gracefully.  
- Optional: richer cell/robotics fidelity if that becomes the next product focus.  
- Optional polish: notify channel, Compose→cloud one-shot, CI.

---

## Locked decisions (v1)

- Bridge transport — **HTTPS POST (JSON batches)** ([ADR 0002](decisions/0002-bridge-transport.md)); MQTT deferred  
- Cloud — **AWS API GW + Python Lambda + DynamoDB + log alerts** ([ADR 0003](decisions/0003-cloud-floor.md))  
- C++ — **C++20** ([ADR 0001](decisions/0001-cpp-standard.md))  
- ROS 2 — **Jazzy** ([ros2-rclcpp.md](spikes/ros2-rclcpp.md))  
- Alert channel — **CloudWatch structured logs**; SNS/Slack deferred  

---

## How to use this document

1. Prefer [SHIPPED.md](SHIPPED.md) and the root [README](../README.md) for demo and scope.  
2. Do not expand v1 scope without an explicit charter revision.  
3. Keep vocabulary industrial/ops (**cell, line, station, fault, cycle**).  
4. Next build work should be a **named follow-on**, not silent growth of v1.
