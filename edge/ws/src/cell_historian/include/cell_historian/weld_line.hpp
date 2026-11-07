#pragma once

#include <string>

#include "cell_msgs/msg/weld_cell_snapshot.hpp"

namespace cell_historian {

/**
 * One InfluxDB line for a weld snapshot.
 * Tags include cell_id so a query can select one cell over a time range.
 * The timestamp is the ROS stamp in nanoseconds.
 */
std::string weld_snapshot_to_line(const cell_msgs::msg::WeldCellSnapshot & snapshot);

}  // namespace cell_historian
