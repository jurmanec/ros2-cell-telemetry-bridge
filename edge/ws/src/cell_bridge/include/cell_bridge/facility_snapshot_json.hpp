#pragma once

#include <string>

#include "cell_msgs/msg/facility_asset_snapshot.hpp"

namespace cell_bridge {

/**
 * Serialize one FacilityAssetSnapshot to the schema_version 3 HTTP body.
 * asset_id is copied from the message. stamp becomes one UTC timestamp.
 * Contract: docs/contracts/facility-asset-snapshot.md
 */
std::string facility_snapshot_to_json(const cell_msgs::msg::FacilityAssetSnapshot & snapshot);

}  // namespace cell_bridge
