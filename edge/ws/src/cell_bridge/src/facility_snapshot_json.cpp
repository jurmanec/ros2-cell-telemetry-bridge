#include "cell_bridge/facility_snapshot_json.hpp"

#include <cstdint>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

namespace cell_bridge {

namespace {

std::string escape_json_string(const std::string & in)
{
  std::string out;
  out.reserve(in.size() + 8);
  for (char c : in) {
    switch (c) {
      case '\\': out += "\\\\"; break;
      case '"': out += "\\\""; break;
      case '\n': out += "\\n"; break;
      case '\r': out += "\\r"; break;
      case '\t': out += "\\t"; break;
      default: out += c; break;
    }
  }
  return out;
}

std::string format_utc_timestamp(int32_t sec, uint32_t nanosec)
{
  const std::time_t unix_sec = static_cast<std::time_t>(sec);
  std::tm utc{};
  gmtime_r(&unix_sec, &utc);
  char date[32];
  std::strftime(date, sizeof(date), "%Y-%m-%dT%H:%M:%S", &utc);
  std::ostringstream oss;
  oss << date << '.' << std::setw(9) << std::setfill('0') << nanosec << 'Z';
  return oss.str();
}

}  // namespace

std::string facility_snapshot_to_json(const cell_msgs::msg::FacilityAssetSnapshot & snapshot)
{
  std::ostringstream oss;
  oss << '{'
      << "\"schema_version\":3,"
      << "\"timestamp\":\""
      << format_utc_timestamp(snapshot.stamp.sec, snapshot.stamp.nanosec) << "\","
      << "\"site_id\":\"" << escape_json_string(snapshot.site_id) << "\","
      << "\"hall_id\":\"" << escape_json_string(snapshot.hall_id) << "\","
      << "\"asset_id\":\"" << escape_json_string(snapshot.asset_id) << "\","
      << "\"asset_type\":\"" << escape_json_string(snapshot.asset_type) << "\","
      << "\"status\":\"" << escape_json_string(snapshot.status) << "\","
      << "\"reading\":" << snapshot.reading << ','
      << "\"unit\":\"" << escape_json_string(snapshot.unit) << "\","
      << "\"alarms\":[";

  for (size_t i = 0; i < snapshot.alarms.size(); ++i) {
    const auto & alarm = snapshot.alarms[i];
    if (i > 0) {
      oss << ',';
    }
    oss << '{'
        << "\"code\":\"" << escape_json_string(alarm.code) << "\","
        << "\"severity\":\"" << escape_json_string(alarm.severity) << "\","
        << "\"message\":\"" << escape_json_string(alarm.message) << "\""
        << '}';
  }

  oss << "]}";
  return oss.str();
}

}  // namespace cell_bridge
