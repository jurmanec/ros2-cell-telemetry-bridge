#pragma once

#include <cstdint>
#include <string>

#include "cell_msgs/msg/cell_alarm.hpp"
#include "cell_msgs/msg/weld_cell_snapshot.hpp"
#include "cell_msgs/msg/weld_joint.hpp"
#include "rclcpp/time.hpp"

namespace cell_sim {

/** One simulated weld cycle, in publish ticks. About 42 s at 1 Hz. */
inline constexpr std::uint64_t kWeldCycleTicks = 42;
/** Arc is on for the first 20 ticks of each cycle. */
inline constexpr std::uint64_t kWeldArcTicks = 20;
/** Fault a completed cycle when the part count is a multiple of this value. */
inline constexpr int kWeldAlarmEveryCycles = 15;

/**
 * Build one welding-cell snapshot for a 1-based publish tick.
 * Contract: docs/contracts/weld-cell-snapshot.md
 * alarm_every_cycles <= 0 disables the occasional GAS_FLOW_LOW sample.
 */
inline cell_msgs::msg::WeldCellSnapshot make_weld_snapshot(
  const rclcpp::Time & stamp,
  const std::string & cell_id,
  std::uint64_t tick,
  int alarm_every_cycles = kWeldAlarmEveryCycles)
{
  if (tick == 0) {
    tick = 1;
  }

  const std::uint64_t index = (tick - 1) % kWeldCycleTicks;
  const bool arc_on = index < kWeldArcTicks;
  const bool cycle_complete = index + 1 == kWeldCycleTicks;
  const std::uint32_t parts = static_cast<std::uint32_t>(tick / kWeldCycleTicks);
  const std::uint32_t scrap = parts / 25;
  const bool alarm = cycle_complete && alarm_every_cycles > 0 &&
    parts > 0 && (parts % static_cast<std::uint32_t>(alarm_every_cycles) == 0);
  const double drift = static_cast<double>(tick % 8) * 0.2;

  cell_msgs::msg::WeldCellSnapshot msg;
  msg.cell_id = cell_id;
  msg.cell_type = "robotic_welding";
  msg.status = alarm ? "FAULTED" : "RUNNING";
  msg.program = "DOOR_FRAME_WELD_V4";
  msg.stamp = stamp;
  msg.cycle_time_last_s = parts == 0 ? 0.0 : static_cast<double>(kWeldCycleTicks);
  msg.parts_total_shift = parts;
  msg.parts_scrap_shift = scrap;
  msg.oee_availability = parts == 0
    ? 1.0
    : static_cast<double>(parts - scrap) / static_cast<double>(parts);

  const double currents[3] = {4.1, 6.3, 4.7};
  const double temps[3] = {48.0, 55.0, 50.0};
  msg.joints.reserve(3);
  for (std::uint8_t axis = 1; axis <= 3; ++axis) {
    cell_msgs::msg::WeldJoint joint;
    joint.axis = axis;
    joint.temp_c = temps[axis - 1] + drift;
    joint.current_a = arc_on ? currents[axis - 1] : 0.2;
    msg.joints.push_back(joint);
  }

  msg.process.weld_current_a = arc_on ? 210.0 + static_cast<double>(tick % 3) * 0.1 : 0.0;
  msg.process.weld_voltage_v = arc_on ? 23.1 : 0.0;
  msg.process.wire_feed_m_min = arc_on ? 7.4 : 0.0;
  msg.process.arc_on_s = arc_on ? static_cast<double>(index + 1) : static_cast<double>(kWeldArcTicks);
  msg.process.gas_flow_l_min = alarm ? 8.0 : 15.0;

  if (alarm) {
    cell_msgs::msg::CellAlarm item;
    item.code = "GAS_FLOW_LOW";
    item.severity = "warning";
    item.message = "Shielding gas flow below setpoint";
    msg.alarms.push_back(item);
  }
  return msg;
}

}  // namespace cell_sim
