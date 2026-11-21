# Proposed v2 Architecture

## Fixed cells (welding, assembly, machine tending, etc.)

```text
Cell (publishes at 1 Hz)
    ├──► Local Historian          (receives full 1 Hz stream)
    └──► Bridge node              (also receives full 1 Hz stream)
              │
              ▼
         Bridge decides what to forward
              │
              ▼
         AWS API Gateway → DynamoDB
```

## Typical Fleet Architecture

```text
AMRs / AGVs
    │
    ▼
On-site Fleet Manager          ← primary local system of record
    ├── Local database / logs  (mission history, pose trails, faults, battery)
    ├── Traffic & charging logic
    └── Optional local historian or time-series store
            │
            ▼
    Bridge / edge gateway (optional filtering)
            │
            ▼
    Cloud (AWS, Azure, vendor cloud, etc.)
```

## How They Usually Come Together

```text
Fixed cells ──► Local historian ──┐
                                  ├──► Edge / cloud gateway ──► Unified cloud monitoring
Mobile fleet ─► Fleet manager ────┘                              (DynamoDB, Timestream,
                                                                   dashboards, alerts, etc.)
```

## Responsibilities

| Component | Responsibility |
|-----------|----------------|
| Cell | Publishes the full telemetry message at 1 Hz to both the local historian and the bridge (or to a local broker that both subscribe to). |
| Local Historian | Stores every message for on-site, high-resolution history. |
| Bridge node | Receives the full stream and decides what continues to the cloud. It may:<br>• Forward every message<br>• Downsample (e.g., every 5th message)<br>• Aggregate (averages over a window)<br>• Forward only on change or alarm<br>• Send a heartbeat + exceptions |
| AWS side | Receives and stores whatever the bridge sends. |
