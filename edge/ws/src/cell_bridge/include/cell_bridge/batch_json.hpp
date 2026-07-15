#pragma once

#include <string>
#include <vector>

#include "cell_msgs/msg/cell_telemetry.hpp"

namespace cell_bridge {

struct BatchConfig {
  std::string cell_id{"cell-001"};
  int schema_version{1};
};

/** Serialize samples to the M2 HTTP batch JSON contract. */
std::string samples_to_batch_json(
  const BatchConfig & config,
  const std::vector<cell_msgs::msg::CellTelemetry> & samples);

}  // namespace cell_bridge
