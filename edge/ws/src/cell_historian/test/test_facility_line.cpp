#include <string>

#include "cell_historian/facility_line.hpp"
#include "cell_msgs/msg/cell_alarm.hpp"
#include "cell_msgs/msg/facility_asset_snapshot.hpp"
#include "gtest/gtest.h"

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
  msg.stamp.sec = 10;
  msg.stamp.nanosec = 111192632;
  return msg;
}

}  // namespace

TEST(FacilityLine, RecordsIdentityReadingAndStamp)
{
  const std::string line = cell_historian::facility_snapshot_to_line(healthy_crah());

  EXPECT_NE(line.find("facility_asset,site_id=SITE-01,hall_id=HALL-01,asset_id=CRAH-01"), std::string::npos);
  EXPECT_NE(line.find("asset_type=crah"), std::string::npos);
  EXPECT_NE(line.find("status=RUNNING"), std::string::npos);
  EXPECT_NE(line.find("reading=16"), std::string::npos);
  EXPECT_NE(line.find("alarm_count=0i"), std::string::npos);
  EXPECT_NE(line.find(" 10111192632"), std::string::npos);
}

TEST(FacilityLine, RecordsCoolingAlarmText)
{
  auto msg = healthy_crah();
  msg.status = "FAULTED";
  msg.reading = 28.0;
  cell_msgs::msg::CellAlarm alarm;
  alarm.code = "COOLING_SUPPLY_HIGH";
  alarm.severity = "warning";
  alarm.message = "Supply air above setpoint";
  msg.alarms.push_back(alarm);

  const std::string line = cell_historian::facility_snapshot_to_line(msg);

  EXPECT_NE(line.find("status=FAULTED"), std::string::npos);
  EXPECT_NE(line.find("reading=28"), std::string::npos);
  EXPECT_NE(line.find("alarm_count=1i"), std::string::npos);
  EXPECT_NE(line.find("a0_code=\"COOLING_SUPPLY_HIGH\""), std::string::npos);
  EXPECT_NE(line.find("a0_severity=\"warning\""), std::string::npos);
  EXPECT_NE(line.find("a0_message=\"Supply air above setpoint\""), std::string::npos);
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
