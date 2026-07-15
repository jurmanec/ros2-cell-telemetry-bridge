#include "cell_bridge/batch_json.hpp"

#include <sstream>

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

}  // namespace

std::string samples_to_batch_json(
  const BatchConfig & config,
  const std::vector<cell_msgs::msg::CellTelemetry> & samples)
{
  std::ostringstream oss;
  oss << '{'
      << "\"schema_version\":" << config.schema_version << ','
      << "\"cell_id\":\"" << escape_json_string(config.cell_id) << "\","
      << "\"samples\":[";

  for (size_t i = 0; i < samples.size(); ++i) {
    const auto & s = samples[i];
    if (i > 0) {
      oss << ',';
    }
    oss << '{'
        << "\"stamp_sec\":" << s.stamp.sec << ','
        << "\"stamp_nanosec\":" << s.stamp.nanosec << ','
        << "\"cycle\":" << s.cycle << ','
        << "\"temp_c\":" << s.temp_c << ','
        << "\"vibration\":" << s.vibration << ','
        << "\"fault\":" << (s.fault ? "true" : "false")
        << '}';
  }

  oss << "]}";
  return oss.str();
}

}  // namespace cell_bridge
