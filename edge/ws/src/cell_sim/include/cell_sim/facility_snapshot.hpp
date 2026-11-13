#pragma once

#include <cstdint>
#include <string>

#include "cell_msgs/msg/cell_alarm.hpp"
#include "cell_msgs/msg/facility_asset_snapshot.hpp"
#include "rclcpp/time.hpp"

namespace cell_sim {

/** Cooling supply-air fault on CRAH-01, in publish ticks. About 3 min at 1 Hz. */
inline constexpr std::uint64_t kCrahFaultTick = 180;
/** Hall-temperature fault on ROOM-01, in publish ticks. About 6 min at 1 Hz. */
inline constexpr std::uint64_t kRoomFaultTick = 360;

/**
 * Build one facility-asset snapshot for a 1-based publish tick.
 * Contract: docs/contracts/facility-asset-snapshot.md
 *
 * Readings and alarm text come from asset_type (crah, room, ups).
 * fault_at_tick == 0 never faults. A positive value latches FAULTED
 * from that tick onward.
 */
inline cell_msgs::msg::FacilityAssetSnapshot make_facility_snapshot(
  const rclcpp::Time & stamp,
  const std::string & site_id,
  const std::string & hall_id,
  const std::string & asset_id,
  const std::string & asset_type,
  std::uint64_t tick,
  std::uint64_t fault_at_tick)
{
  if (tick == 0) {
    tick = 1;
  }

  double healthy = 0.0;
  double faulted_reading = 0.0;
  const char * unit = "";
  const char * alarm_code = "";
  const char * alarm_message = "";
  if (asset_type == "crah") {
    healthy = 16.0;
    faulted_reading = 28.0;
    unit = "C";
    alarm_code = "COOLING_SUPPLY_HIGH";
    alarm_message = "Supply air above setpoint";
  } else if (asset_type == "room") {
    healthy = 22.0;
    faulted_reading = 32.0;
    unit = "C";
    alarm_code = "ROOM_TEMP_HIGH";
    alarm_message = "Hall temperature above limit";
  } else if (asset_type == "ups") {
    healthy = 40.0;
    faulted_reading = 40.0;
    unit = "pct";
  }

  const bool faulted = fault_at_tick > 0 && tick >= fault_at_tick;

  cell_msgs::msg::FacilityAssetSnapshot msg;
  msg.site_id = site_id;
  msg.hall_id = hall_id;
  msg.asset_id = asset_id;
  msg.asset_type = asset_type;
  msg.status = faulted ? "FAULTED" : "RUNNING";
  msg.reading = faulted ? faulted_reading : healthy;
  msg.unit = unit;
  msg.stamp = stamp;
  if (faulted && alarm_code[0] != '\0') {
    cell_msgs::msg::CellAlarm alarm;
    alarm.code = alarm_code;
    alarm.severity = "warning";
    alarm.message = alarm_message;
    msg.alarms.push_back(alarm);
  }
  return msg;
}

}  // namespace cell_sim
