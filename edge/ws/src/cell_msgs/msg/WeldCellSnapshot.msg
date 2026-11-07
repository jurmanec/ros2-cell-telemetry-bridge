# Status snapshot for a simulated robotic welding cell.
# Contract: docs/contracts/weld-cell-snapshot.md

string cell_id
string cell_type
string status
string program
builtin_interfaces/Time stamp
float64 cycle_time_last_s
uint32 parts_total_shift
uint32 parts_scrap_shift
float64 oee_availability
WeldJoint[] joints
WeldProcess process
CellAlarm[] alarms
