#include <gtest/gtest.h>

#include "cell_bridge/facility_snapshot_json.hpp"
#include "cell_msgs/msg/cell_alarm.hpp"
#include "cell_msgs/msg/facility_asset_snapshot.hpp"

namespace {

cell_msgs::msg::FacilityAssetSnapshot healthy_crah()
{
  cell_msgs::msg::FacilityAssetSnapshot msg;
  msg.site_id = "SITE-01";
  msg.hall_id = "HALL-01";
  msg.asset_id = "CRAH-01";
  msg.asset_type = "crah";
  msg.status = "RUNNING";
  msg.reading = 16.0;
  msg.unit = "C";
  msg.stamp.sec = 0;
  msg.stamp.nanosec = 111192632;
  return msg;
}

}  // namespace

TEST(FacilitySnapshotJson, SerializesHealthyCrah)
{
  const std::string json = cell_bridge::facility_snapshot_to_json(healthy_crah());

  EXPECT_NE(json.find("\"schema_version\":3"), std::string::npos);
  EXPECT_NE(json.find("\"timestamp\":\"1970-01-01T00:00:00.111192632Z\""), std::string::npos);
  EXPECT_NE(json.find("\"site_id\":\"SITE-01\""), std::string::npos);
  EXPECT_NE(json.find("\"hall_id\":\"HALL-01\""), std::string::npos);
  EXPECT_NE(json.find("\"asset_id\":\"CRAH-01\""), std::string::npos);
  EXPECT_NE(json.find("\"asset_type\":\"crah\""), std::string::npos);
  EXPECT_NE(json.find("\"status\":\"RUNNING\""), std::string::npos);
  EXPECT_NE(json.find("\"reading\":16"), std::string::npos);
  EXPECT_NE(json.find("\"unit\":\"C\""), std::string::npos);
  EXPECT_NE(json.find("\"alarms\":[]"), std::string::npos);
}

TEST(FacilitySnapshotJson, SerializesCoolingSupplyHigh)
{
  auto msg = healthy_crah();
  msg.status = "FAULTED";
  msg.reading = 28.0;
  cell_msgs::msg::CellAlarm alarm;
  alarm.code = "COOLING_SUPPLY_HIGH";
  alarm.severity = "warning";
  alarm.message = "Supply air above setpoint";
  msg.alarms.push_back(alarm);

  const std::string json = cell_bridge::facility_snapshot_to_json(msg);

  EXPECT_NE(json.find("\"status\":\"FAULTED\""), std::string::npos);
  EXPECT_NE(json.find("\"reading\":28"), std::string::npos);
  EXPECT_NE(json.find("\"code\":\"COOLING_SUPPLY_HIGH\""), std::string::npos);
  EXPECT_NE(json.find("\"severity\":\"warning\""), std::string::npos);
  EXPECT_NE(json.find("\"message\":\"Supply air above setpoint\""), std::string::npos);
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
