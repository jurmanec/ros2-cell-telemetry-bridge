#include <gtest/gtest.h>

#include "cell_bridge/batch_json.hpp"
#include "cell_msgs/msg/cell_telemetry.hpp"

TEST(BatchJson, SerializesContractShape)
{
  cell_bridge::BatchConfig config;
  config.cell_id = "cell-001";
  config.schema_version = 1;

  cell_msgs::msg::CellTelemetry sample;
  sample.stamp.sec = 10;
  sample.stamp.nanosec = 20;
  sample.cycle = 3;
  sample.temp_c = 41.5;
  sample.vibration = 0.16;
  sample.fault = true;

  const std::string json = cell_bridge::samples_to_batch_json(config, {sample});

  EXPECT_NE(json.find("\"schema_version\":1"), std::string::npos);
  EXPECT_NE(json.find("\"cell_id\":\"cell-001\""), std::string::npos);
  EXPECT_NE(json.find("\"cycle\":3"), std::string::npos);
  EXPECT_NE(json.find("\"temp_c\":41.5"), std::string::npos);
  EXPECT_NE(json.find("\"fault\":true"), std::string::npos);
  EXPECT_NE(json.find("\"stamp_sec\":10"), std::string::npos);
}

TEST(BatchJson, EscapesCellId)
{
  cell_bridge::BatchConfig config;
  config.cell_id = "cell\"x";
  const std::string json = cell_bridge::samples_to_batch_json(config, {});
  EXPECT_NE(json.find("cell\\\"x"), std::string::npos);
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
