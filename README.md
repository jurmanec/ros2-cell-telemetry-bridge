# P3 — Industrial Cell Telemetry Bridge

**Status:** v1 frozen (2026-07-15)

Simulated manufacturing-cell telemetry on a **C++ / ROS 2 Jazzy** edge, batched over HTTPS to a thin **AWS** ingest path that persists samples and emits **rule-based** health events as CloudWatch logs.

**What this is:** an edge→cloud **data-path tracer** — ROS 2 / `rclcpp` scaffolding plus known cloud patterns (API Gateway, Lambda, DynamoDB, smoke).  
**What this is not:** real sensors, robot control, actuators, a robot model, or trained ML. Alerts for v1 are **structured log events** (`CELL_ALERT`), not SNS/Slack/Paging.

Source of truth: [docs/CHARTER.md](docs/CHARTER.md) · Architecture: [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) · Milestones: [docs/MILESTONES.md](docs/MILESTONES.md) · [Shipped vs deferred](docs/SHIPPED.md)

---

## How to demo

### A. Cloud happy path (fastest — M4 smoke)

Requires a deployed `dev` stack and AWS credentials:

```bash
cd cloud
cp .env.example .env   # set INGEST_API_KEY; deploy if not already
npm install
npx serverless deploy --stage dev --region us-east-1
.venv/bin/pip install -r requirements.txt   # once
.venv/bin/python scripts/smoke_ingest.py    # expect SMOKE PASSED
```

Details: [cloud/README.md](cloud/README.md). Events: [docs/contracts/cloud-ingest-events.md](docs/contracts/cloud-ingest-events.md).

### B. Local edge + stub (M2)

```bash
cd edge
./scripts/run_m2_stack.sh    # publisher + bridge + stub ingest
```

Details: [edge/README.md](edge/README.md).

### C. Edge → live AWS (M3 path)

Point `cell_bridge` at IngestUrl with `api_key` matching `INGEST_API_KEY` — see [cloud/README.md](cloud/README.md#point-the-edge-bridge-at-cloud).

---

## Repo map

| Path | Role |
|------|------|
| [`edge/`](edge/) | Jazzy packages `cell_msgs`, `cell_sim`, `cell_bridge`; Compose + stub ingest |
| [`cloud/`](cloud/) | Serverless ingest Lambda, DynamoDB, smoke script |
| [`docs/contracts/`](docs/contracts/) | Telemetry + HTTP batch + ingest log events |
| [`docs/decisions/`](docs/decisions/) | ADRs (C++20, HTTPS bridge, AWS cloud floor) |
| [`spike/ros2/`](spike/ros2/) | Historical Humble spike (throwaway) |

---

## Follow-ons (not v1)

- **P3.1 MLOps** — small anomaly model on the Dynamo series (e.g. ONNX), deploy, observe, fail gracefully  
- Optional polish: SNS/Slack notify, pure-JSON Lambda logs, Compose→cloud one-shot, GitHub Actions  
- Deeper robotics: real sensors/control, richer cell models (beyond this tracer)

Pre-charter notes: [docs/archive/](docs/archive/).
