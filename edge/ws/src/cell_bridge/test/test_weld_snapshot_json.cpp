#include <cstdint>

#include <gtest/gtest.h>

#include "cell_bridge/weld_snapshot_json.hpp"
#include "cell_msgs/msg/cell_alarm.hpp"
#include "cell_msgs/msg/weld_cell_snapshot.hpp"
#include "cell_msgs/msg/weld_joint.hpp"

namespace {

cell_msgs::msg::WeldCellSnapshot sample_snapshot()
{
  cell_msgs::msg::WeldCellSnapshot msg;
  msg.cell_id = "WELD_CELL_07";
  msg.cell_type = "robotic_welding";
  msg.status = "RUNNING";
  msg.program = "DOOR_FRAME_WELD_V4";
  msg.stamp.sec = 0;
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
    joint.temp_c = 49.0;
    joint.current_a = 4.0;
    msg.joints.push_back(joint);
  }
  return msg;
}

}  // namespace

TEST(WeldSnapshotJson, SerializesEnvelopeAndUtcTimestamp)
{
  const std::string json = cell_bridge::weld_snapshot_to_json(sample_snapshot());

  EXPECT_NE(json.find("\"schema_version\":2"), std::string::npos);
  EXPECT_NE(json.find("\"timestamp\":\"1970-01-01T00:00:00.111192632Z\""), std::string::npos);
  EXPECT_NE(json.find("\"cell_id\":\"WELD_CELL_07\""), std::string::npos);
  EXPECT_NE(json.find("\"cell_type\":\"robotic_welding\""), std::string::npos);
  EXPECT_NE(json.find("\"arc_on_s\":19.6"), std::string::npos);
  EXPECT_NE(json.find("\"alarms\":[]"), std::string::npos);
  EXPECT_EQ(json.find("\"samples\""), std::string::npos);
}

TEST(WeldSnapshotJson, SerializesAlarmAndEscapesMessage)
{
  auto msg = sample_snapshot();
  msg.status = "FAULTED";
  cell_msgs::msg::CellAlarm alarm;
  alarm.code = "GAS_FLOW_LOW";
  alarm.severity = "warning";
  alarm.message = "gas \"low\"";
  msg.alarms.push_back(alarm);

  const std::string json = cell_bridge::weld_snapshot_to_json(msg);

  EXPECT_NE(json.find("\"status\":\"FAULTED\""), std::string::npos);
  EXPECT_NE(json.find("\"code\":\"GAS_FLOW_LOW\""), std::string::npos);
  EXPECT_NE(json.find("\"message\":\"gas \\\"low\\\"\""), std::string::npos);
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
