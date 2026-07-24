# P3 suggestion — Industrial cell telemetry bridge

**Status:** Historical (pre-charter; superseded by [CHARTER.md](../CHARTER.md) and v1 freeze)  
**Suggested:** 2026-07-13  
**Context:** Early planning note toward C++/ROS 2 + industrial edge→cloud, after prior portfolio work on serverless middleware, orchestration, and anomaly monitoring. Private career notes are not in this repo.

---

## One-liner

A simulated manufacturing cell (or warehouse station) publishes sensor/state telemetry from a **C++ ROS2** edge; a thin cloud path ingests, stores, and evaluates health. Optional ML is a later slice, not day one.

---

## Why this next

| Already shown (P0–P2) | Still thin for the career arc |
|----------------------|--------------------------------|
| Event-driven serverless middleware (P0) | **C++** / systems programming |
| Step Functions-style orchestration (P1) | **ROS2** / robotics |
| Anomaly monitoring + operator console (P2) | **Industrial / real-economy** domain |
| Ops discipline: contracts, smoke, observability, scope freeze | **MLOps** (defer to P3.1) |

This project bridges **robotics + C++ + cloud/platform + industrial domain** without cloning another order/orchestration middleware demo. Reuse P0–P2 *practices* (testing, observability, ADRs, explicit deferrals); do not reuse retail/order vocabulary as the primary story.

---

## MVP vertical slice (freezeable)

Ship a coherent tracer, then pause (P2-style), rather than an open-ended build.

1. **Edge (C++ / ROS2)** — 1–2 nodes: simulated metrics (e.g. temperature, vibration, cycle time, fault flag); publish on topics; no real hardware required.
2. **Bridge** — ROS2 → MQTT/HTTP or a small agent that forwards batches to AWS (P0-style ingest discipline, new domain).
3. **Cloud** — Persist time series; one API or thin dashboard path; **rule-based** alerts first (patterns already proven in P2).
4. **Engineering bar** — contracts or wire docs, smoke path, structured logs, explicit “not built” list.

**Definition of done for v1:** demoable edge → cloud → alert path; docs state what shipped vs deferred; tests cover the happy path at least at unit + one integration/smoke level.

---

## Intentionally deferred (v1)

- Learned / historical baselines and model serving (candidate **P3.1 MLOps**)
- Full digital twin
- Multi-robot fleet
- Heavy operator SPA (optional thin UI only if needed for demo)
- Production ROS deploy / real plant hardware
- Extending P0 or P2 as products

---

## Suggested follow-on (P3.1)

Once the telemetry path is frozen: add a small **ONNX (or similar) anomaly model** on the series — deploy, observe, fail gracefully. That slice targets AI/ML Engineering / MLOps without blocking the first robotics+cloud demo.

---

## Alternatives considered (not primary)

| Option | Notes |
|--------|--------|
| Extend P2 (F-002 / learned baselines) | Good polish; weak career-signal change. Fine as a short side slice. |
| Pure MLOps model-serving demo | Strong for AI/ML roles; weaker on C++/ROS2 and industrial story. Better as P3.1 or P4. |
| Another Java serverless workflow | Repeats P0/P1; hiring managers see sameness. |

---

## CodeCodex capture (while building)

Distill durable notes into the knowledge map as gaps are closed:

| Topic | Likely home |
|-------|-------------|
| ROS2 node / topic patterns, edge constraints | `architecture/` (or future robotics notes) |
| Ingest, IAM, ops alarms, bridge design | `cloud/`, `engineering/` |
| Modeling / serving when P3.1 starts | `ml/` |
| Agent/prompt habits that worked | `llm/` |

Rule of thumb: capture what unblocked the build; avoid encyclopedia fill.

---

## Decision tradeoff (pick before scaffold)

Prefer one emphasis for the first freeze:

- **More C++ / ROS2 depth** — richer edge nodes, idiomatic modern C++, local sim quality; keep cloud thin.
- **More AWS / platform depth** — richer ingest, IaC observability, stages/CD; keep edge to 1–2 stub nodes.

Default recommendation if undecided: **lean C++/ROS2 on the edge, thin but real AWS path**, so the portfolio visibly advances past Java-only serverless demos.

---

## How agents should use this

1. Treat this as the **proposed P3**, not an active backlog, until the user starts a repo or explicitly scopes milestones.
2. Do not reopen P0/P2 unless asked; cite them only for reusable practices.
3. Keep domain industrial/ops (cell, line, station, fleet node) — not retail/BOPIS/order lifecycle.
4. Prefer scope freezes and deferred lists in the same spirit as P0–P2.
