# Cell and Fleet Telemetry Samples

This document collects sample telemetry payloads from both **fixed manufacturing cells** and **mobile robot fleets**. It also describes how these diverse streams can be published to a single cloud-based storage and monitoring system.

---

## 1. Fixed Manufacturing Cell Telemetry

These samples represent stationary robotic work cells (welding, assembly, machine tending, palletizing, etc.).

### 1.1 Welding Cell — Normal Running

```json
{
  "timestamp": "2026-11-06T17:05:22Z",
  "cell_id": "WELD_CELL_07",
  "cell_type": "robotic_welding",
  "status": "RUNNING",
  "program": "DOOR_FRAME_WELD_V4",
  "cycle_time_last_s": 42.1,
  "parts_total_shift": 156,
  "parts_scrap_shift": 3,
  "oee_availability": 0.91,
  "joints": [
    {"axis": 1, "temp_c": 49.2, "current_a": 4.1},
    {"axis": 2, "temp_c": 56.8, "current_a": 6.3},
    {"axis": 3, "temp_c": 51.4, "current_a": 4.7}
  ],
  "process": {
    "weld_current_a": 210,
    "weld_voltage_v": 23.1,
    "wire_feed_m_min": 7.4,
    "arc_on_s": 19.6,
    "gas_flow_l_min": 15.0
  },
  "alarms": []
}
```

### 1.2 Machine-Tending Cell — Waiting for Part

```json
{
  "timestamp": "2026-11-06T17:05:45Z",
  "cell_id": "CNC_TEND_02",
  "cell_type": "machine_tending",
  "status": "WAITING",
  "wait_reason": "upstream_part_not_present",
  "program": "LATHE_LOAD_UNLOAD",
  "cycle_time_last_s": 68.4,
  "parts_total_shift": 89,
  "parts_scrap_shift": 1,
  "robot_status": "IDLE",
  "joints": [
    {"axis": 1, "temp_c": 38.1, "current_a": 0.4},
    {"axis": 2, "temp_c": 41.5, "current_a": 0.3},
    {"axis": 3, "temp_c": 39.7, "current_a": 0.5}
  ],
  "process": {
    "gripper_state": "open",
    "part_present_infeed": false,
    "cnc_status": "running",
    "cnc_cycle_remaining_s": 142
  },
  "alarms": []
}
```

### 1.3 Assembly Cell — Minor Fault

```json
{
  "timestamp": "2026-11-06T17:06:01Z",
  "cell_id": "ASM_CELL_14",
  "cell_type": "assembly",
  "status": "FAULTED",
  "program": "BRACKET_ASSEMBLY_V8",
  "cycle_time_last_s": null,
  "parts_total_shift": 203,
  "parts_scrap_shift": 11,
  "robot_status": "STOPPED",
  "joints": [
    {"axis": 1, "temp_c": 44.6, "current_a": 0.0},
    {"axis": 2, "temp_c": 48.9, "current_a": 0.0},
    {"axis": 3, "temp_c": 46.2, "current_a": 0.0}
  ],
  "process": {
    "screwdriver_torque_nm": 0.0,
    "vision_result": "failed",
    "part_present": true
  },
  "alarms": [
    {
      "code": "VISION_MISMATCH",
      "severity": "warning",
      "message": "Part orientation out of tolerance",
      "timestamp": "2026-11-06T17:05:58Z"
    }
  ]
}
```

### 1.4 Palletizing Cell — High Throughput

```json
{
  "timestamp": "2026-11-06T17:06:18Z",
  "cell_id": "PAL_CELL_01",
  "cell_type": "palletizing",
  "status": "RUNNING",
  "program": "CASE_PALLETIZE_LAYER_5",
  "cycle_time_last_s": 4.8,
  "cases_total_shift": 1842,
  "pallets_completed_shift": 17,
  "robot_status": "RUNNING",
  "joints": [
    {"axis": 1, "temp_c": 52.3, "current_a": 5.8},
    {"axis": 2, "temp_c": 58.1, "current_a": 7.4},
    {"axis": 3, "temp_c": 55.6, "current_a": 6.1}
  ],
  "process": {
    "gripper_vacuum_bar": -0.81,
    "cases_per_layer": 12,
    "current_layer": 5,
    "conveyor_speed_m_min": 24.0
  },
  "energy_kwh_shift": 38.4,
  "alarms": []
}
```

---

## 2. Mobile Robot Fleet Telemetry

These samples represent AMRs, AGVs, and other mobile robots, plus fleet-level summaries.

### 2.1 AMR — On a Mission (Warehouse)

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

### 2.2 AMR — Charging

```json
{
  "timestamp": "2026-11-06T17:07:02Z",
  "robot_id": "AMR-017",
  "fleet_id": "WAREHOUSE_FLEET_A",
  "robot_type": "amr",
  "status": "CHARGING",
  "mission_id": null,
  "battery_percent": 28,
  "battery_voltage": 46.1,
  "charge_rate_w": 820,
  "estimated_full_minutes": 42,
  "pose": {
    "x": 5.2,
    "y": 2.1,
    "theta": 0.0
  },
  "dock_id": "CHARGER_03",
  "alarms": []
}
```

### 2.3 AGV — Blocked / Traffic

```json
{
  "timestamp": "2026-11-06T17:07:18Z",
  "robot_id": "AGV-008",
  "fleet_id": "FACTORY_LOGISTICS",
  "robot_type": "agv",
  "status": "BLOCKED",
  "mission_id": "TRANSPORT_4451",
  "battery_percent": 81,
  "pose": {
    "x": 112.4,
    "y": 67.9,
    "theta": 3.14
  },
  "speed_m_s": 0.0,
  "block_reason": "path_occupied_by_AMR-029",
  "wait_time_s": 37,
  "payload_id": "PALLET-2204",
  "alarms": [
    {
      "code": "TRAFFIC_HOLD",
      "severity": "info",
      "message": "Waiting for path clearance"
    }
  ]
}
```

### 2.4 Fleet Summary Snapshot

```json
{
  "timestamp": "2026-11-06T17:07:30Z",
  "fleet_id": "WAREHOUSE_FLEET_A",
  "total_robots": 48,
  "robots_active": 31,
  "robots_charging": 9,
  "robots_idle": 6,
  "robots_faulted": 2,
  "average_battery_percent": 62,
  "missions_in_progress": 27,
  "missions_completed_today": 1842,
  "average_mission_time_s": 186,
  "alerts": [
    {
      "robot_id": "AMR-033",
      "code": "LOW_BATTERY",
      "severity": "warning"
    },
    {
      "robot_id": "AMR-051",
      "code": "LOCALIZATION_DRIFT",
      "severity": "error"
    }
  ]
}
```

---

## 3. Publishing Both to One Cloud-Based System

It is practical and increasingly common for both fixed manufacturing cells and mobile robot fleets to publish telemetry to the **same cloud-based storage and monitoring platform**.

### Typical Architecture

```
Fixed Cells (welding, assembly, tending, palletizing)
    │
    ├── PLC / Robot Controller / Edge Gateway
    │
Mobile Robots (AMRs, AGVs)
    │
    ├── Fleet Manager / Edge Gateway
    │
    ▼
Common Protocols (MQTT, OPC UA, REST, etc.)
    │
    ▼
Cloud IoT Platform
(AWS IoT Core, Azure IoT Hub, Google Cloud IoT, PTC ThingWorx, Siemens MindSphere, custom, etc.)
    │
    ├── Time-series database (e.g., InfluxDB, Timescale, AWS Timestream)
    ├── Object storage for raw payloads
    ├── Stream processing / rules engine
    └── Dashboards, alerts, analytics, digital twin
```

### Key Integration Points

- **Protocol unification**: Most systems publish via MQTT topics or OPC UA. Fixed cells and mobile robots can use the same broker.
- **Topic / namespace design**:  
  - `factory/cells/WELD_CELL_07/telemetry`  
  - `factory/fleets/WAREHOUSE_FLEET_A/robots/AMR-042/telemetry`  
  - `factory/fleets/WAREHOUSE_FLEET_A/summary`
- **Schema flexibility**: Cloud platforms accept different JSON schemas. A common envelope (timestamp, source_id, source_type, payload) can be added if desired, or the raw messages can be stored as-is and queried by type.
- **Edge summarization**: High-frequency joint data from fixed cells is often aggregated at the edge before cloud upload; mobile robots typically already send lower-frequency status updates.
- **Correlation value**: A unified system can link events across domains (e.g., an assembly cell waiting because the AMR delivering parts is blocked or low on battery).

### Benefits of a Single Cloud System

- One operational dashboard for production and logistics
- Correlated root-cause analysis across fixed and mobile assets
- Unified alerting and OEE-style metrics
- Single security, retention, and compliance model
- Easier digital twin and predictive maintenance across the whole facility

This architecture is standard in modern Industry 4.0 / smart-factory deployments.
