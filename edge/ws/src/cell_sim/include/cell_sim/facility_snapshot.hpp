#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "cell_msgs/msg/cell_alarm.hpp"
#include "cell_msgs/msg/facility_asset_snapshot.hpp"
#include "rclcpp/time.hpp"

namespace cell_sim {

/** One scripted state. It holds from tick until the next phase. */
struct FacilityPhase {
  std::uint64_t tick{0};
  double reading{0.0};
  std::string status;
  std::string code;
  std::string severity;
  std::string message;
};

/**
 * Empty when the six arrays describe a valid phase list.
 * Otherwise a line that names the parameter that failed.
 * Ticks are positive and strictly increasing. A FAULTED phase has a code.
 * A RUNNING phase has an empty code. A code has severity info, warning, or error.
 */
inline std::string facility_phase_error(
  const std::vector<int64_t> & ticks,
  const std::vector<double> & readings,
  const std::vector<std::string> & statuses,
  const std::vector<std::string> & codes,
  const std::vector<std::string> & severities,
  const std::vector<std::string> & messages)
{
  const auto count = ticks.size();
  if (readings.size() != count) {
    return "phase_reading length does not match phase_tick";
  }
  if (statuses.size() != count) {
    return "phase_status length does not match phase_tick";
  }
  if (codes.size() != count) {
    return "phase_code length does not match phase_tick";
  }
  if (severities.size() != count) {
    return "phase_severity length does not match phase_tick";
  }
  if (messages.size() != count) {
    return "phase_message length does not match phase_tick";
  }

  for (size_t i = 0; i < count; ++i) {
    const auto at = " at index " + std::to_string(i);
    if (ticks[i] <= 0 || (i > 0 && ticks[i] <= ticks[i - 1])) {
      return "phase_tick must be positive and strictly increasing" + at;
    }
    if (statuses[i] != "RUNNING" && statuses[i] != "FAULTED") {
      return "phase_status must be RUNNING or FAULTED" + at;
    }
    const bool faulted = statuses[i] == "FAULTED";
    if (faulted && codes[i].empty()) {
      return "phase_code must be set when phase_status is FAULTED" + at;
    }
    if (!faulted && !codes[i].empty()) {
      return "phase_code must be empty when phase_status is RUNNING" + at;
    }
    if (codes[i].empty()) {
      if (!severities[i].empty()) {
        return "phase_severity must be empty when phase_code is empty" + at;
      }
      if (!messages[i].empty()) {
        return "phase_message must be empty when phase_code is empty" + at;
      }
    } else if (
      severities[i] != "info" && severities[i] != "warning" && severities[i] != "error")
    {
      return "phase_severity must be info, warning, or error when phase_code is set" + at;
    } else if (messages[i].empty()) {
      return "phase_message must be set when phase_code is set" + at;
    }
  }
  return "";
}

/** Build the phase list. Call only after facility_phase_error returns empty. */
inline std::vector<FacilityPhase> facility_phases(
  const std::vector<int64_t> & ticks,
  const std::vector<double> & readings,
  const std::vector<std::string> & statuses,
  const std::vector<std::string> & codes,
  const std::vector<std::string> & severities,
  const std::vector<std::string> & messages)
{
  std::vector<FacilityPhase> phases;
  phases.reserve(ticks.size());
  for (size_t i = 0; i < ticks.size(); ++i) {
    phases.push_back(FacilityPhase{
      static_cast<std::uint64_t>(ticks[i]),
      readings[i],
      statuses[i],
      codes[i],
      severities[i],
      messages[i],
    });
  }
  return phases;
}

/**
 * Build one facility-asset snapshot for a 1-based publish tick.
 * Contract: docs/contracts/facility-asset-snapshot.md
 *
 * The reading before the first phase is healthy_reading, status RUNNING, no alarm.
 * A phase holds from its tick until the next phase. An empty code adds no alarm.
 */
inline cell_msgs::msg::FacilityAssetSnapshot make_facility_snapshot(
  const rclcpp::Time & stamp,
  const std::string & site_id,
  const std::string & hall_id,
  const std::string & asset_id,
  const std::string & asset_type,
  const std::string & unit,
  double healthy_reading,
  std::uint64_t tick,
  const std::vector<FacilityPhase> & phases)
{
  if (tick == 0) {
    tick = 1;
  }

  const FacilityPhase * active = nullptr;
  for (const auto & phase : phases) {
    if (phase.tick > tick) {
      break;
    }
    active = &phase;
  }

  cell_msgs::msg::FacilityAssetSnapshot msg;
  msg.site_id = site_id;
  msg.hall_id = hall_id;
  msg.asset_id = asset_id;
  msg.asset_type = asset_type;
  msg.unit = unit;
  msg.stamp = stamp;
  if (active == nullptr) {
    msg.status = "RUNNING";
    msg.reading = healthy_reading;
    return msg;
  }

  msg.status = active->status;
  msg.reading = active->reading;
  if (!active->code.empty()) {
    cell_msgs::msg::CellAlarm alarm;
    alarm.code = active->code;
    alarm.severity = active->severity;
    alarm.message = active->message;
    msg.alarms.push_back(alarm);
  }
  return msg;
}

}  // namespace cell_sim
