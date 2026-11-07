#pragma once

#include <string>
#include <vector>

#include "cell_msgs/msg/cell_telemetry.hpp"

namespace cell_bridge {

struct BatchConfig {
  int schema_version{1};
};

/**
 * Serialize one cell's samples to the HTTP batch JSON contract.
 * cell_id is the publisher's identity (every sample in the vector belongs to it).
 */
std::string samples_to_batch_json(
  const BatchConfig & config,
  const std::string & cell_id,
  const std::vector<cell_msgs::msg::CellTelemetry> & samples);

}  // namespace cell_bridge
