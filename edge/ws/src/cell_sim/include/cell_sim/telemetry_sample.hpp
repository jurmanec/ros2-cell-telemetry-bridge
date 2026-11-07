#pragma once

#include <cstdint>
#include <string>

#include "cell_msgs/msg/cell_telemetry.hpp"
#include "rclcpp/time.hpp"

namespace cell_sim {

/** Build a CellTelemetry sample (contract: docs/contracts/cell-telemetry.md). */
inline cell_msgs::msg::CellTelemetry make_telemetry_sample(
  const rclcpp::Time & stamp,
  const std::string & cell_id,
  std::uint64_t cycle,
  double temp_c,
  double vibration,
  bool fault)
{
  cell_msgs::msg::CellTelemetry msg;
  msg.cell_id = cell_id;
  msg.stamp = stamp;
  msg.cycle = cycle;
  msg.temp_c = temp_c;
  msg.vibration = vibration;
  msg.fault = fault;
  return msg;
}

}  // namespace cell_sim
