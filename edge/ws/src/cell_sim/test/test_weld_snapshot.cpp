#include <gtest/gtest.h>

#include "cell_sim/weld_snapshot.hpp"
#include "rclcpp/rclcpp.hpp"

TEST(WeldSnapshot, RunningSampleHasThreeJointsAndNoAlarm) {
  const rclcpp::Time stamp(10, 0, RCL_ROS_TIME);
  const auto msg = cell_sim::make_weld_snapshot(stamp, "WELD_CELL_07", 1);

  EXPECT_EQ(msg.cell_id, "WELD_CELL_07");
  EXPECT_EQ(msg.cell_type, "robotic_welding");
  EXPECT_EQ(msg.status, "RUNNING");
  EXPECT_EQ(msg.program, "DOOR_FRAME_WELD_V4");
  EXPECT_EQ(msg.joints.size(), 3u);
  EXPECT_LT(msg.joints[0].temp_c, msg.joints[1].temp_c);
  EXPECT_LT(msg.joints[2].temp_c, msg.joints[1].temp_c);
  EXPECT_DOUBLE_EQ(msg.process.arc_on_s, 1.0);
  EXPECT_GT(msg.process.weld_current_a, 200.0);
  EXPECT_TRUE(msg.alarms.empty());
  EXPECT_EQ(msg.parts_total_shift, 0u);
}

TEST(WeldSnapshot, ArcOffDropsJointCurrent) {
  const rclcpp::Time stamp(0, 0, RCL_ROS_TIME);
  const auto msg = cell_sim::make_weld_snapshot(stamp, "WELD_CELL_07", 21);

  EXPECT_EQ(msg.status, "RUNNING");
  EXPECT_DOUBLE_EQ(msg.process.arc_on_s, 20.0);
  EXPECT_DOUBLE_EQ(msg.process.weld_current_a, 0.0);
  EXPECT_DOUBLE_EQ(msg.joints[0].current_a, 0.2);
}

TEST(WeldSnapshot, CompletedCycleCountsAPart) {
  const rclcpp::Time stamp(0, 0, RCL_ROS_TIME);
  const auto msg = cell_sim::make_weld_snapshot(stamp, "WELD_CELL_07", 42);

  EXPECT_EQ(msg.parts_total_shift, 1u);
  EXPECT_DOUBLE_EQ(msg.cycle_time_last_s, 42.0);
  EXPECT_EQ(msg.status, "RUNNING");
  EXPECT_TRUE(msg.alarms.empty());
}

TEST(WeldSnapshot, EveryFifteenthCycleRaisesGasAlarm) {
  const rclcpp::Time stamp(0, 0, RCL_ROS_TIME);
  const auto msg = cell_sim::make_weld_snapshot(stamp, "WELD_CELL_07", 42 * 15);

  EXPECT_EQ(msg.parts_total_shift, 15u);
  EXPECT_EQ(msg.status, "FAULTED");
  ASSERT_EQ(msg.alarms.size(), 1u);
  EXPECT_EQ(msg.alarms[0].code, "GAS_FLOW_LOW");
  EXPECT_EQ(msg.alarms[0].severity, "warning");
  EXPECT_DOUBLE_EQ(msg.process.gas_flow_l_min, 8.0);
}

int main(int argc, char ** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
