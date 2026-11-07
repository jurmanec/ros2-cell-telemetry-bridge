#pragma once

#include <string>

#include "cell_msgs/msg/weld_cell_snapshot.hpp"

namespace cell_bridge {

/**
 * Serialize one WeldCellSnapshot to the schema_version 2 HTTP body.
 * cell_id is copied from the message. stamp becomes one UTC timestamp.
 * Contract: docs/contracts/weld-cell-snapshot.md
 */
std::string weld_snapshot_to_json(const cell_msgs::msg::WeldCellSnapshot & snapshot);

}  // namespace cell_bridge
