# One facility asset, as published by the ROS 2 stand-in for a building-management system.
# Contract: docs/contracts/facility-asset-snapshot.md

string site_id
string hall_id
string asset_id
string asset_type
string status
float64 reading
string unit
builtin_interfaces/Time stamp
CellAlarm[] alarms
