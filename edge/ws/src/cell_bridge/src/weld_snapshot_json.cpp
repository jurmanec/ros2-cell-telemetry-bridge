#include "cell_bridge/weld_snapshot_json.hpp"

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

std::string weld_snapshot_to_json(const cell_msgs::msg::WeldCellSnapshot & snapshot)
{
  std::ostringstream oss;
  oss << '{'
      << "\"schema_version\":2,"
      << "\"timestamp\":\""
      << format_utc_timestamp(snapshot.stamp.sec, snapshot.stamp.nanosec) << "\","
      << "\"cell_id\":\"" << escape_json_string(snapshot.cell_id) << "\","
      << "\"cell_type\":\"" << escape_json_string(snapshot.cell_type) << "\","
      << "\"status\":\"" << escape_json_string(snapshot.status) << "\","
      << "\"program\":\"" << escape_json_string(snapshot.program) << "\","
      << "\"cycle_time_last_s\":" << snapshot.cycle_time_last_s << ','
      << "\"parts_total_shift\":" << snapshot.parts_total_shift << ','
      << "\"parts_scrap_shift\":" << snapshot.parts_scrap_shift << ','
      << "\"oee_availability\":" << snapshot.oee_availability << ','
      << "\"joints\":[";

  for (size_t i = 0; i < snapshot.joints.size(); ++i) {
    const auto & joint = snapshot.joints[i];
    if (i > 0) {
      oss << ',';
    }
    oss << '{'
        << "\"axis\":" << static_cast<unsigned>(joint.axis) << ','
        << "\"temp_c\":" << joint.temp_c << ','
        << "\"current_a\":" << joint.current_a
        << '}';
  }

  const auto & process = snapshot.process;
  oss << "],\"process\":{"
      << "\"weld_current_a\":" << process.weld_current_a << ','
      << "\"weld_voltage_v\":" << process.weld_voltage_v << ','
      << "\"wire_feed_m_min\":" << process.wire_feed_m_min << ','
      << "\"arc_on_s\":" << process.arc_on_s << ','
      << "\"gas_flow_l_min\":" << process.gas_flow_l_min
      << "},\"alarms\":[";

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
