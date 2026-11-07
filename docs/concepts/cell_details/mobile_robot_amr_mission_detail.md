# Mobile Robot: AMR on a Mission

An **AMR (Autonomous Mobile Robot) on a mission** is a mobile robot actively executing an assigned task, usually in a warehouse, factory, or logistics environment. Unlike a fixed manufacturing cell, the AMR moves freely through the facility using onboard sensors and software to navigate, avoid obstacles, and complete its job without fixed paths (unlike traditional AGVs).

## What “On a Mission” Means

A mission is a discrete job the fleet management system assigns to the robot. Common examples include:

- **Goods-to-person picking**: Travel to a storage location, pick up a tote or shelf, and deliver it to a packing or picking station.
- **Material transport**: Move parts, raw materials, or finished goods between production cells or storage areas.
- **Replenishment**: Bring inventory to a workstation that is running low.
- **Returns / put-away**: Take empty containers or returned items back to storage.

While on a mission the AMR is in a dynamic state: navigating, updating its position, monitoring battery, handling payload, and coordinating with other robots and infrastructure.

## Typical Mission Flow

1. Fleet manager assigns a mission (e.g., “Pick tote TOTE-99381 from location A and deliver to Station 14”).
2. AMR plans a path using its map and current traffic conditions.
3. It navigates autonomously, continuously localizing itself (usually with LiDAR, cameras, or both) and avoiding people and other robots.
4. It arrives, interacts with the payload (lifts, docks, or confirms pickup), then proceeds to the destination.
5. It completes the delivery and reports success (or a failure reason) back to the fleet manager, which then assigns the next mission or sends it to charge.

## Key Telemetry While on a Mission

Typical live data includes:

- **Pose** (`x`, `y`, `theta`) — current location and heading
- **Speed** and remaining **ETA**
- **Battery level** and voltage
- **Mission ID** and destination
- **Payload status** (present / ID of the tote or pallet)
- **Obstacles detected** and any temporary blocks
- **Status** (`NAVIGATING`, `LOADING`, `UNLOADING`, `BLOCKED`, etc.)
- Alarms (localization drift, low battery, hardware faults)

This stream lets the fleet manager and higher-level systems monitor progress, re-route robots, predict delays, and intervene if needed.

## How It Differs from a Fixed Cell

| Aspect              | Fixed Welding Cell              | AMR on a Mission                     |
|---------------------|---------------------------------|--------------------------------------|
| Location            | Stationary                      | Continuously moving                  |
| Primary goal        | Process quality (welds)         | Transport / material flow            |
| Key data            | Weld params, joint temps        | Pose, battery, ETA, payload          |
| Coordination        | Within one station              | Across the whole facility & other robots |
| Typical duration    | Tens of seconds per cycle       | Minutes per mission                  |

## Real-World Context

In modern warehouses and factories, dozens or hundreds of AMRs may be on missions simultaneously. The fleet management system continuously optimizes assignments so that material arrives at production cells just in time, minimizing the “waiting for parts” states seen in machine-tending cells.

## Example Telemetry Snapshot

```json
{
  "timestamp": "2026-11-06T17:06:45Z",
  "robot_id": "AMR-042",
  "fleet_id": "WAREHOUSE_FLEET_A",
  "robot_type": "amr",
  "status": "NAVIGATING",
  "mission_id": "PICK_MISSION_7812",
  "battery_percent": 67,
  "battery_voltage": 48.3,
  "pose": {
    "x": 42.7,
    "y": 18.3,
    "theta": 1.57
  },
  "speed_m_s": 1.2,
  "destination": "STATION_14",
  "eta_seconds": 48,
  "obstacles_detected": 0,
  "payload_present": true,
  "payload_id": "TOTE-99381",
  "alarms": []
}
```
