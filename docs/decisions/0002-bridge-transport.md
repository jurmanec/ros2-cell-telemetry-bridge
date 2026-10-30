# ADR 0002 — Bridge northbound transport

**Status:** Accepted (implemented in M2)  
**Date:** 2026-07-13  
**Milestone:** M2

## Decision

Use **HTTPS POST** with **JSON batches** for the bridge’s **northbound** egress (ROS graph → ingest) in v1.

Defer **MQTT** (and any dual-egress / hybrid) until a later slice if needed. Do **not** build MQTT alongside HTTPS in M2.

## Context

M2 requires a forwarder that subscribes to cell telemetry, batches samples, and delivers them to an ingest endpoint. The charter left transport open (`MQTT` vs `HTTPS`).

The edge already has a local “hot path”: **ROS 2 topics** (`cell_msgs/msg/CellTelemetry` on `cell/telemetry`). The bridge is not that control/sensor loop; it is how samples leave the cell toward cloud health evaluation.

Industry tip often heard: MQTT for hot path, HTTP for cold path (config, history, firmware). That pattern is valid for many full IIoT stacks, but here ROS/DDS already owns the local hot path. Northbound telemetry can still be HTTPS without denying a future MQTT option.

## Options considered

| Option | Pros | Cons |
|--------|------|------|
| **HTTPS POST + JSON batches** | Easy local stub (curl-friendly); maps cleanly to AWS-style API ingest later; batching/retries familiar; fewer moving parts for M2 | Less “classic OT broker” stereotype than MQTT |
| **MQTT** | Strong IIoT/gateway narrative; natural pub/sub egress | Needs broker in the demo loop; extra QoS/session concepts; M3 often still needs a hop into store/rules; higher scope risk for v1 |
| **Hybrid now (MQTT hot + HTTP cold)** | Matches some production end-states | Doubles contracts, stubs, and failure modes in M2; cold-path HTTP is out of v1 scope anyway (no config/firmware/history product yet) |

## Why HTTPS now

1. **Scope** — one northbound mechanism for M2; stub ingest can share the same JSON contract as future cloud ingest.
2. **Edge focus** — keep broker ops out of the critical path while the ROS 2 / C++ edge and cloud floor settle.
3. **AWS fit** — HTTPS/API ingest maps cleanly to API Gateway–style cloud ingest for the thin northbound hop.
4. **Hybrid later remains open** — “Northbound HTTPS now; optional MQTT into the bridge or dual egress later” without painting into a corner.

## Consequences

- Implement `cell_bridge` (or equivalent) as a C++ / `rclcpp` node depending on `cell_msgs`.
- Define a batch JSON wire contract for HTTPS bodies (companion to the ROS msg contract).
- M2 may target a **local stub ingest** (HTTP 200 + log/ack); M3 swaps in real cloud ingest behind the same URL/contract shape where practical.
- Document basic failure/retry (timeouts, non-2xx, simple retries) — not a full reliability platform.
- MQTT, IoT Core–style broker paths, and dual egress are **out of M2**; revisit only with an explicit milestone/ADR update.
- Charter/architecture “bridge transport TBD” → locked to this ADR.
