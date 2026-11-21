#include <cstdint>
#include <vector>

#include <gtest/gtest.h>

#include "cell_sim/facility_snapshot.hpp"
#include "rclcpp/rclcpp.hpp"

namespace {

const std::vector<cell_sim::FacilityPhase> & phases()
{
  static const std::vector<cell_sim::FacilityPhase> list = {
    {10, 27.0, "FAULTED", "ROOM_TEMP_HIGH", "warning",
      "Hall temperature above recommended limit."},
    {20, 32.0, "FAULTED", "ROOM_TEMP_HIGH", "error",
      "Hall temperature at allowable limit."},
    {30, 22.0, "RUNNING", "", "", ""},
  };
  return list;
}

cell_msgs::msg::FacilityAssetSnapshot at(std::uint64_t tick)
{
  const rclcpp::Time stamp(10, 0, RCL_ROS_TIME);
  return cell_sim::make_facility_snapshot(
    stamp, "SITE-01", "HALL-01", "ROOM-01", "room", "C", 22.0, tick, phases());
}

}  // namespace

TEST(FacilitySnapshot, TickBeforeAPhaseIsHealthy) {
  const auto msg = at(9);

  EXPECT_EQ(msg.site_id, "SITE-01");
  EXPECT_EQ(msg.hall_id, "HALL-01");
  EXPECT_EQ(msg.asset_id, "ROOM-01");
  EXPECT_EQ(msg.asset_type, "room");
  EXPECT_EQ(msg.unit, "C");
  EXPECT_EQ(msg.stamp.sec, 10);
  EXPECT_EQ(msg.status, "RUNNING");
  EXPECT_DOUBLE_EQ(msg.reading, 22.0);
  EXPECT_TRUE(msg.alarms.empty());
}

TEST(FacilitySnapshot, FirstTickOfAPhaseCarriesTheAlarm) {
  const auto msg = at(10);

  EXPECT_EQ(msg.status, "FAULTED");
  EXPECT_DOUBLE_EQ(msg.reading, 27.0);
  ASSERT_EQ(msg.alarms.size(), 1u);
  EXPECT_EQ(msg.alarms[0].code, "ROOM_TEMP_HIGH");
  EXPECT_EQ(msg.alarms[0].severity, "warning");
  EXPECT_EQ(msg.alarms[0].message, "Hall temperature above recommended limit.");

  const auto still = at(19);
  EXPECT_EQ(still.status, "FAULTED");
  EXPECT_DOUBLE_EQ(still.reading, 27.0);
  ASSERT_EQ(still.alarms.size(), 1u);
  EXPECT_EQ(still.alarms[0].severity, "warning");
}

TEST(FacilitySnapshot, LaterPhaseChangesSeverity) {
  const auto msg = at(20);

  EXPECT_EQ(msg.status, "FAULTED");
  EXPECT_DOUBLE_EQ(msg.reading, 32.0);
  ASSERT_EQ(msg.alarms.size(), 1u);
  EXPECT_EQ(msg.alarms[0].code, "ROOM_TEMP_HIGH");
  EXPECT_EQ(msg.alarms[0].severity, "error");
  EXPECT_EQ(msg.alarms[0].message, "Hall temperature at allowable limit.");
}

TEST(FacilitySnapshot, LaterPhaseReturnsToRunning) {
  const auto msg = at(30);

  EXPECT_EQ(msg.status, "RUNNING");
  EXPECT_DOUBLE_EQ(msg.reading, 22.0);
  EXPECT_TRUE(msg.alarms.empty());

  const auto still = at(31);
  EXPECT_EQ(still.status, "RUNNING");
  EXPECT_DOUBLE_EQ(still.reading, 22.0);
  EXPECT_TRUE(still.alarms.empty());
}

TEST(FacilitySnapshot, EmptyPhaseListStaysHealthy) {
  const rclcpp::Time stamp(0, 0, RCL_ROS_TIME);
  const auto msg = cell_sim::make_facility_snapshot(
    stamp, "SITE-01", "HALL-01", "UPS-01", "ups", "pct", 40.0, 5, {});

  EXPECT_EQ(msg.status, "RUNNING");
  EXPECT_DOUBLE_EQ(msg.reading, 40.0);
  EXPECT_EQ(msg.unit, "pct");
  EXPECT_TRUE(msg.alarms.empty());
}

int main(int argc, char ** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
