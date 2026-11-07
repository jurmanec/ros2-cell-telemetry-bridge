#include <cstdint>
#include <string>

#include "cell_historian/weld_line.hpp"
#include "cell_msgs/msg/cell_alarm.hpp"
#include "cell_msgs/msg/weld_cell_snapshot.hpp"
#include "cell_msgs/msg/weld_joint.hpp"
#include "gtest/gtest.h"

namespace {

cell_msgs::msg::WeldCellSnapshot sample()
{
  cell_msgs::msg::WeldCellSnapshot msg;
  msg.cell_id = "WELD_CELL_07";
  msg.cell_type = "robotic_welding";
  msg.status = "RUNNING";
  msg.program = "DOOR FRAME";
  msg.stamp.sec = 10;
  msg.stamp.nanosec = 111192632;
  msg.cycle_time_last_s = 42.1;
  msg.parts_total_shift = 156;
  msg.parts_scrap_shift = 3;
  msg.oee_availability = 0.91;
  msg.process.weld_current_a = 210.0;
  msg.process.weld_voltage_v = 23.1;
  msg.process.wire_feed_m_min = 7.4;
  msg.process.arc_on_s = 19.6;
  msg.process.gas_flow_l_min = 15.0;
  for (uint8_t axis = 1; axis <= 3; ++axis) {
    cell_msgs::msg::WeldJoint joint;
    joint.axis = axis;
    joint.temp_c = 50.0 + axis;
    joint.current_a = 4.0;
    msg.joints.push_back(joint);
  }
  return msg;
}

}  // namespace

TEST(WeldLine, RecordsIdentityJointsProcessAndStamp)
{
  const std::string line = cell_historian::weld_snapshot_to_line(sample());

  EXPECT_NE(line.find("weld_snapshot,cell_id=WELD_CELL_07"), std::string::npos);
  EXPECT_NE(line.find("program=DOOR\\ FRAME"), std::string::npos);
  EXPECT_NE(line.find("parts_total_shift=156i"), std::string::npos);
  EXPECT_NE(line.find("j2_temp_c="), std::string::npos);
  EXPECT_NE(line.find("weld_current_a=210"), std::string::npos);
  EXPECT_NE(line.find("gas_flow_l_min=15"), std::string::npos);
  EXPECT_NE(line.find("alarm_count=0i"), std::string::npos);
  EXPECT_NE(line.find(" 10111192632"), std::string::npos);
}

TEST(WeldLine, RecordsAlarmText)
{
  auto msg = sample();
  msg.status = "FAULTED";
  cell_msgs::msg::CellAlarm alarm;
  alarm.code = "GAS_FLOW_LOW";
  alarm.severity = "warning";
  alarm.message = "gas \"low\"";
  msg.alarms.push_back(alarm);

  const std::string line = cell_historian::weld_snapshot_to_line(msg);

  EXPECT_NE(line.find("status=FAULTED"), std::string::npos);
  EXPECT_NE(line.find("alarm_count=1i"), std::string::npos);
  EXPECT_NE(line.find("a0_code=\"GAS_FLOW_LOW\""), std::string::npos);
  EXPECT_NE(line.find("a0_message=\"gas \\\"low\\\"\""), std::string::npos);
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
