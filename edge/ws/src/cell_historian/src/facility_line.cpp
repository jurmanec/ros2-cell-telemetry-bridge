#include "cell_historian/facility_line.hpp"

#include <cstdint>
#include <sstream>
#include <string>

namespace cell_historian {

namespace {

std::string escape_tag(const std::string & in)
{
  std::string out;
  out.reserve(in.size());
  for (char c : in) {
    if (c == ' ' || c == ',' || c == '=' || c == '\\') {
      out += '\\';
    }
    out += c;
  }
  return out;
}

std::string escape_field_string(const std::string & in)
{
  std::string out;
  out.reserve(in.size());
  for (char c : in) {
    if (c == '"' || c == '\\') {
      out += '\\';
    }
    out += c;
  }
  return out;
}

}  // namespace

std::string facility_snapshot_to_line(const cell_msgs::msg::FacilityAssetSnapshot & snapshot)
{
  std::ostringstream oss;
  oss << "facility_asset"
      << ",site_id=" << escape_tag(snapshot.site_id)
      << ",hall_id=" << escape_tag(snapshot.hall_id)
      << ",asset_id=" << escape_tag(snapshot.asset_id)
      << ",asset_type=" << escape_tag(snapshot.asset_type)
      << ",status=" << escape_tag(snapshot.status)
      << ' '
      << "reading=" << snapshot.reading
      << ",alarm_count=" << snapshot.alarms.size() << 'i';

  for (size_t i = 0; i < snapshot.alarms.size(); ++i) {
    const auto & alarm = snapshot.alarms[i];
    oss << ",a" << i << "_code=\"" << escape_field_string(alarm.code) << '"'
        << ",a" << i << "_severity=\"" << escape_field_string(alarm.severity) << '"'
        << ",a" << i << "_message=\"" << escape_field_string(alarm.message) << '"';
  }

  const auto nanosec = static_cast<std::int64_t>(snapshot.stamp.sec) * 1000000000LL +
    static_cast<std::int64_t>(snapshot.stamp.nanosec);
  oss << ' ' << nanosec;
  return oss.str();
}

}  // namespace cell_historian
