#include <gtest/gtest.h>

#include "cell_sim/telemetry_sample.hpp"
#include "rclcpp/rclcpp.hpp"

TEST(TelemetrySample, FillsContractFields) {
  const rclcpp::Time stamp(1, 500000000, RCL_ROS_TIME);
  const auto msg = cell_sim::make_telemetry_sample(stamp, 42ULL, 41.5, 0.16, true);

  EXPECT_EQ(msg.stamp.sec, 1);
  EXPECT_EQ(msg.stamp.nanosec, 500000000u);
  EXPECT_EQ(msg.cycle, 42ULL);
  EXPECT_DOUBLE_EQ(msg.temp_c, 41.5);
  EXPECT_DOUBLE_EQ(msg.vibration, 0.16);
  EXPECT_TRUE(msg.fault);
}

TEST(TelemetrySample, FaultFalseByDefaultPath) {
  const rclcpp::Time stamp(0, 0, RCL_ROS_TIME);
  const auto msg = cell_sim::make_telemetry_sample(stamp, 1ULL, 40.5, 0.12, false);
  EXPECT_FALSE(msg.fault);
  EXPECT_EQ(msg.cycle, 1ULL);
}

int main(int argc, char ** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
