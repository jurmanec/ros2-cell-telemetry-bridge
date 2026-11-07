# Fleet Management Systems for Mobile Robots

A **fleet management system (FMS)** is the central software that coordinates a group of mobile robots (AMRs, AGVs, or mixed fleets) so they work together efficiently instead of colliding, idling, or working against each other.

It is the “air traffic control” layer for robots on the floor.

## Core Responsibilities

- **Mission assignment** — Decides which robot should do which job based on location, battery level, payload capability, and priority.
- **Traffic management** — Prevents deadlocks and collisions by controlling movement through shared areas, intersections, and narrow aisles.
- **Battery / charging management** — Sends robots to charge at the right time so the fleet stays available without unnecessary downtime.
- **Real-time monitoring** — Shows the position, status, and health of every robot on a live map.
- **Integration** — Receives work orders from higher systems (WMS, MES, ERP) and reports completion back.
- **Exception handling** — Detects blocked robots, localization problems, or faults and either reassigns the mission or alerts operators.

## How It Fits in the Stack

```
WMS / MES / ERP
        │
        ▼
Fleet Management System  ←→  Operator dashboard / alerts
        │
        ▼
Individual robots (AMRs / AGVs)
```

The robots handle local navigation and safety. The fleet manager handles coordination across the whole facility.

## Key Features

| Feature                      | Purpose                                      |
|-----------------------------|----------------------------------------------|
| Live map visualization      | See every robot’s position and status        |
| Intelligent scheduling      | Assign the best robot for each mission       |
| Traffic control             | Avoid congestion and deadlocks               |
| Charging strategy           | Keep battery levels optimal                  |
| Multi-brand support         | Manage robots from different manufacturers   |
| Simulation                  | Test layouts and fleet size before deployment|
| APIs / standards            | Integrate with WMS, MES, doors, elevators    |

A widely used standard for communication between the fleet manager and vehicles is **VDA 5050**, which defines how orders and status are exchanged (usually over MQTT).

## Relationship to Telemetry

The fleet management system is both a **consumer** and a **producer** of robot telemetry:

- It continuously receives pose, battery, status, and payload data from each robot.
- It uses that data to make assignment and traffic decisions.
- It can also publish higher-level fleet summaries (robots active, charging, faulted, average battery, missions in progress) to a cloud monitoring system or MES.

## Typical Deployment

Most fleet managers run **on-site or at the edge** so traffic control continues even if the cloud connection drops. Longer-term analytics, dashboards, and multi-site views are often pushed to the cloud.

## Examples of Commercial Systems

- OTTO Fleet Manager (Rockwell)
- KUKA.AMR Fleet Manager
- BlueBotics ANT server
- Various vendor-specific and multi-vendor platforms that support VDA 5050

## Summary

The fleet management system turns a collection of individual mobile robots into a coordinated, efficient material-handling system that can feed fixed manufacturing cells and keep production flowing.
