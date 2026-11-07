

Example
```json
{
 "pk": "WELD_CELL_07",
 "sk": "2026-11-07T18:50:40.664385588Z#03112945",
 "alarms": [
 ],
 "cell_id": "WELD_CELL_07",
 "cell_type": "robotic_welding",
 "cycle_time_last_s": 0,
 "ingested_at": 1794077442,
 "joints": [
  {
   "axis": 1,
   "current_a": 4.1,
   "temp_c": 49
  },
  {
   "axis": 2,
   "current_a": 6.3,
   "temp_c": 56
  },
  {
   "axis": 3,
   "current_a": 4.7,
   "temp_c": 51
  }
 ],
 "oee_availability": 1,
 "parts_scrap_shift": 0,
 "parts_total_shift": 0,
 "process": {
  "arc_on_s": 5,
  "gas_flow_l_min": 15,
  "weld_current_a": 210.2,
  "weld_voltage_v": 23.1,
  "wire_feed_m_min": 7.4
 },
 "program": "DOOR_FRAME_WELD_V4",
 "schema_version": 2,
 "status": "RUNNING",
 "timestamp": "2026-11-07T18:50:40.664385588Z"
}
```

Faulted example 
```json
{
 "pk": "WELD_CELL_07",
 "sk": "2026-11-07T19:01:05.644843461Z#db946422",
 "alarms": [
  {
   "code": "GAS_FLOW_LOW",
   "message": "Shielding gas flow below setpoint",
   "severity": "warning"
  }
 ],
 "cell_id": "WELD_CELL_07",
 "cell_type": "robotic_welding",
 "cycle_time_last_s": 42,
 "ingested_at": 1794078065,
 "joints": [
  {
   "axis": 1,
   "current_a": 0.2,
   "temp_c": 49.2
  },
  {
   "axis": 2,
   "current_a": 0.2,
   "temp_c": 56.2
  },
  {
   "axis": 3,
   "current_a": 0.2,
   "temp_c": 51.2
  }
 ],
 "oee_availability": 1,
 "parts_scrap_shift": 0,
 "parts_total_shift": 15,
 "process": {
  "arc_on_s": 20,
  "gas_flow_l_min": 8,
  "weld_current_a": 0,
  "weld_voltage_v": 0,
  "wire_feed_m_min": 0
 },
 "program": "DOOR_FRAME_WELD_V4",
 "schema_version": 2,
 "status": "FAULTED",
 "timestamp": "2026-11-07T19:01:05.644843461Z"
}

```