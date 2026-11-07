# Filtering and Reduction at the Bridge Node

By the time data from these two sources reaches a bridge node, the bridge is the usual place where filtering and reduction happen before anything is sent to the cloud.

## 1. Welding Cell

Local rate is typically ~1 Hz with a full payload (status, joint temps/currents, weld current/voltage, gas flow, cycle metrics, alarms).

### Likely reduction at the bridge

| Technique              | What is kept / changed                                      | Typical result to cloud          |
|------------------------|-------------------------------------------------------------|----------------------------------|
| Downsample             | Forward every 5th message (or every 5 s)                    | 0.2 Hz instead of 1 Hz           |
| On-change + heartbeat  | Send immediately on status/alarm change; otherwise every 5–10 s | Most messages dropped            |
| Aggregate              | Average weld current, voltage, joint temps over a 5 s window | One summary message per window   |
| Field pruning          | Drop or simplify joint array; keep only max temp or averages | Smaller payload                  |
| Exception priority     | Always forward alarms and out-of-tolerance process values   | Full detail only when needed     |

**Common cloud payload from a welding cell:**  
status, averaged process values, max/average joint temperature, parts/scrap counts, active alarms — sent every 5 seconds or on change.

## 2. AMR on a Mission

Local updates are often already 1–5 seconds (pose, battery, status, mission ID, ETA, payload).

### Likely reduction at the bridge (or inside the fleet manager before the bridge)

| Technique              | What is kept / changed                                      | Typical result to cloud          |
|------------------------|-------------------------------------------------------------|----------------------------------|
| Downsample pose        | Send pose every 2–5 s instead of every update               | Lower location update rate       |
| On-change              | Forward only when status, mission, payload, or battery band changes | Many routine updates dropped     |
| Summarize              | Drop raw speed; keep ETA, battery %, destination            | Smaller message                  |
| Fleet-level rollup     | Individual robot messages reduced; periodic fleet summary added | Fewer per-robot messages         |
| Exception priority     | Always forward faults, localization issues, low battery     | Full detail on problems          |

**Common cloud payload from an AMR:**  
robot ID, status, battery %, simplified pose (or zone), mission ID, destination/ETA, payload present, alarms — sent every 5–10 seconds or on meaningful change.

## Side-by-Side Summary

| Source          | Local rate     | Typical cloud rate after bridge | Main reductions                          |
|-----------------|----------------|---------------------------------|------------------------------------------|
| Welding cell    | ~1 Hz full     | 0.1–0.2 Hz (or on-change)       | Averaging, downsampling, field pruning   |
| AMR on mission  | 0.2–1 Hz       | 0.1–0.2 Hz (or on-change)       | Downsampling pose, on-change, summarization |

In both cases the bridge (or the fleet manager acting as the edge layer) is responsible for turning the fuller local stream into a lighter, cloud-appropriate stream.
