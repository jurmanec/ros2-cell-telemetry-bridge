#include <stdexcept>
#include <string>
#include <vector>

#include "cell_bridge/weld_reduce.hpp"
#include "gtest/gtest.h"

namespace {

std::vector<bool> forward_n(cell_bridge::WeldReducePolicy & policy, int n, int alarm_at)
{
  std::vector<bool> decisions;
  for (int i = 1; i <= n; ++i) {
    decisions.push_back(policy.should_forward("WELD_CELL_07", i == alarm_at, static_cast<double>(i)));
  }
  return decisions;
}

}  // namespace

TEST(WeldReduce, AllForwardsEverySnapshot)
{
  cell_bridge::WeldReducePolicy policy("all", 5, 5.0);
  const auto decisions = forward_n(policy, 3, 2);
  EXPECT_EQ(decisions, (std::vector<bool>{true, true, true}));
}

TEST(WeldReduce, EveryNDropsUntilMultipleAndKeepsAlarm)
{
  cell_bridge::WeldReducePolicy policy("every_n", 5, 5.0);
  const auto decisions = forward_n(policy, 10, 7);
  EXPECT_EQ(
    decisions,
    (std::vector<bool>{
      false, false, false, false, true,
      false, true, false, false, true}));
}

TEST(WeldReduce, EveryNCountsEachCellSeparately)
{
  cell_bridge::WeldReducePolicy policy("every_n", 5, 5.0);
  EXPECT_FALSE(policy.should_forward("WELD_CELL_07", false, 0.0));
  EXPECT_FALSE(policy.should_forward("WELD_CELL_08", false, 0.0));
  EXPECT_FALSE(policy.should_forward("WELD_CELL_07", false, 1.0));
  EXPECT_FALSE(policy.should_forward("WELD_CELL_07", false, 2.0));
  EXPECT_FALSE(policy.should_forward("WELD_CELL_07", false, 3.0));
  EXPECT_TRUE(policy.should_forward("WELD_CELL_07", false, 4.0));
  EXPECT_FALSE(policy.should_forward("WELD_CELL_08", false, 4.0));
}

TEST(WeldReduce, HeartbeatForwardsOnIntervalAndAlarmResetsIt)
{
  cell_bridge::WeldReducePolicy policy("heartbeat", 5, 5.0);
  EXPECT_TRUE(policy.should_forward("WELD_CELL_07", false, 0.0));
  EXPECT_FALSE(policy.should_forward("WELD_CELL_07", false, 4.9));
  EXPECT_TRUE(policy.should_forward("WELD_CELL_07", false, 5.0));
  EXPECT_TRUE(policy.should_forward("WELD_CELL_07", true, 6.0));
  EXPECT_FALSE(policy.should_forward("WELD_CELL_07", false, 10.9));
  EXPECT_TRUE(policy.should_forward("WELD_CELL_07", false, 11.0));
}

TEST(WeldReduce, RejectsInvalidParameters)
{
  EXPECT_THROW(cell_bridge::WeldReducePolicy("sometimes", 5, 5.0), std::invalid_argument);
  EXPECT_THROW(cell_bridge::WeldReducePolicy("every_n", 0, 5.0), std::invalid_argument);
  EXPECT_THROW(cell_bridge::WeldReducePolicy("heartbeat", 5, 0.0), std::invalid_argument);
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
