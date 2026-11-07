#include "cell_historian/weld_line.hpp"

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

std::string weld_snapshot_to_line(const cell_msgs::msg::WeldCellSnapshot & snapshot)
{
  std::ostringstream oss;
  oss << "weld_snapshot"
      << ",cell_id=" << escape_tag(snapshot.cell_id)
      << ",cell_type=" << escape_tag(snapshot.cell_type)
      << ",status=" << escape_tag(snapshot.status)
      << ",program=" << escape_tag(snapshot.program)
      << ' '
      << "cycle_time_last_s=" << snapshot.cycle_time_last_s
      << ",parts_total_shift=" << snapshot.parts_total_shift << 'i'
      << ",parts_scrap_shift=" << snapshot.parts_scrap_shift << 'i'
      << ",oee_availability=" << snapshot.oee_availability;

  for (const auto & joint : snapshot.joints) {
    const auto axis = static_cast<unsigned>(joint.axis);
    oss << ",j" << axis << "_temp_c=" << joint.temp_c
        << ",j" << axis << "_current_a=" << joint.current_a;
  }

  const auto & process = snapshot.process;
  oss << ",weld_current_a=" << process.weld_current_a
      << ",weld_voltage_v=" << process.weld_voltage_v
      << ",wire_feed_m_min=" << process.wire_feed_m_min
      << ",arc_on_s=" << process.arc_on_s
      << ",gas_flow_l_min=" << process.gas_flow_l_min
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
