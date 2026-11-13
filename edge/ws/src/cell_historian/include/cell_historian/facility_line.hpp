#pragma once

#include <string>

#include "cell_msgs/msg/facility_asset_snapshot.hpp"

namespace cell_historian {

/**
 * One InfluxDB line for a facility asset snapshot.
 * Tags include asset_id so a query can select one asset over a time range.
 * The timestamp is the ROS stamp in nanoseconds.
 * Measurement: facility_asset. Bucket: facility.
 */
std::string facility_snapshot_to_line(const cell_msgs::msg::FacilityAssetSnapshot & snapshot);

}  // namespace cell_historian
