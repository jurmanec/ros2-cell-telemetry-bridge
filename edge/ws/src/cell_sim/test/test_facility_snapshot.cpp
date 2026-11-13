#include <gtest/gtest.h>

#include "cell_sim/facility_snapshot.hpp"
#include "rclcpp/rclcpp.hpp"

TEST(FacilitySnapshot, HealthyCrah) {
  const rclcpp::Time stamp(10, 0, RCL_ROS_TIME);
  const auto msg = cell_sim::make_facility_snapshot(
    stamp, "SITE-01", "HALL-01", "CRAH-01", "crah", 1, cell_sim::kCrahFaultTick);

  EXPECT_EQ(msg.site_id, "SITE-01");
  EXPECT_EQ(msg.hall_id, "HALL-01");
  EXPECT_EQ(msg.asset_id, "CRAH-01");
  EXPECT_EQ(msg.asset_type, "crah");
  EXPECT_EQ(msg.status, "RUNNING");
  EXPECT_DOUBLE_EQ(msg.reading, 16.0);
  EXPECT_EQ(msg.unit, "C");
  EXPECT_EQ(msg.stamp.sec, 10);
  EXPECT_TRUE(msg.alarms.empty());
}

TEST(FacilitySnapshot, CrahStaysRunningUntilFaultTick) {
  const rclcpp::Time stamp(0, 0, RCL_ROS_TIME);
  const auto msg = cell_sim::make_facility_snapshot(
    stamp, "SITE-01", "HALL-01", "CRAH-01", "crah", cell_sim::kCrahFaultTick - 1,
    cell_sim::kCrahFaultTick);

  EXPECT_EQ(msg.status, "RUNNING");
  EXPECT_DOUBLE_EQ(msg.reading, 16.0);
  EXPECT_TRUE(msg.alarms.empty());
}

TEST(FacilitySnapshot, CrahFaultsAtTick180AndLatches) {
  const rclcpp::Time stamp(0, 0, RCL_ROS_TIME);
  const auto at_fault = cell_sim::make_facility_snapshot(
    stamp, "SITE-01", "HALL-01", "CRAH-01", "crah", cell_sim::kCrahFaultTick,
    cell_sim::kCrahFaultTick);
  const auto later = cell_sim::make_facility_snapshot(
    stamp, "SITE-01", "HALL-01", "CRAH-01", "crah", cell_sim::kCrahFaultTick + 1,
    cell_sim::kCrahFaultTick);

  EXPECT_EQ(at_fault.status, "FAULTED");
  EXPECT_DOUBLE_EQ(at_fault.reading, 28.0);
  ASSERT_EQ(at_fault.alarms.size(), 1u);
  EXPECT_EQ(at_fault.alarms[0].code, "COOLING_SUPPLY_HIGH");
  EXPECT_EQ(at_fault.alarms[0].severity, "warning");
  EXPECT_EQ(at_fault.alarms[0].message, "Supply air above setpoint");

  EXPECT_EQ(later.status, "FAULTED");
  EXPECT_DOUBLE_EQ(later.reading, 28.0);
  ASSERT_EQ(later.alarms.size(), 1u);
}

TEST(FacilitySnapshot, RoomFaultsAtTick360) {
  const rclcpp::Time stamp(0, 0, RCL_ROS_TIME);
  const auto before = cell_sim::make_facility_snapshot(
    stamp, "SITE-01", "HALL-01", "ROOM-01", "room", cell_sim::kRoomFaultTick - 1,
    cell_sim::kRoomFaultTick);
  const auto at_fault = cell_sim::make_facility_snapshot(
    stamp, "SITE-01", "HALL-01", "ROOM-01", "room", cell_sim::kRoomFaultTick,
    cell_sim::kRoomFaultTick);

  EXPECT_EQ(before.status, "RUNNING");
  EXPECT_DOUBLE_EQ(before.reading, 22.0);
  EXPECT_EQ(before.unit, "C");
  EXPECT_TRUE(before.alarms.empty());

  EXPECT_EQ(at_fault.status, "FAULTED");
  EXPECT_DOUBLE_EQ(at_fault.reading, 32.0);
  ASSERT_EQ(at_fault.alarms.size(), 1u);
  EXPECT_EQ(at_fault.alarms[0].code, "ROOM_TEMP_HIGH");
  EXPECT_EQ(at_fault.alarms[0].message, "Hall temperature above limit");
}

TEST(FacilitySnapshot, UpsNeverFaultsWhenFaultTickIsZero) {
  const rclcpp::Time stamp(0, 0, RCL_ROS_TIME);
  const auto msg = cell_sim::make_facility_snapshot(
    stamp, "SITE-01", "HALL-01", "UPS-01", "ups", 1000, 0);

  EXPECT_EQ(msg.asset_id, "UPS-01");
  EXPECT_EQ(msg.asset_type, "ups");
  EXPECT_EQ(msg.status, "RUNNING");
  EXPECT_DOUBLE_EQ(msg.reading, 40.0);
  EXPECT_EQ(msg.unit, "pct");
  EXPECT_TRUE(msg.alarms.empty());
}

int main(int argc, char ** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
