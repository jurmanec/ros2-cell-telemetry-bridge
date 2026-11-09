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

Parts of a *datapoint* for InfluxDB:

**Measurement** — high-level name or category (e.g., weld_telemetry).
**Tags** — Metadata that identifies the series (e.g., cell_id: WELD_CELL_07).
Could be composite (e.g., cell_id, machine_id).
**Fields** — The actual measured values (status, joints, process points, other numerical measurements). 
**Timestamp** — When the point was record