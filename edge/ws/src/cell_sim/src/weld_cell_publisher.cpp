#include <chrono>
#include <cstdint>
#include <memory>
#include <string>

#include "cell_msgs/msg/weld_cell_snapshot.hpp"
#include "cell_sim/weld_snapshot.hpp"
#include "rclcpp/rclcpp.hpp"

using namespace std::chrono_literals;

/**
 * Robotic welding cell simulator: publishes WeldCellSnapshot on cell/weld_snapshot.
 * Contract: docs/contracts/weld-cell-snapshot.md
 */
class WeldCellPublisher : public rclcpp::Node {
 public:
  WeldCellPublisher() : Node("weld_cell_publisher"), tick_(0) {
    cell_id_ = declare_parameter<std::string>("cell_id", "WELD_CELL_07");
    period_sec_ = declare_parameter<double>("period_sec", 1.0);
    alarm_every_cycles_ = declare_parameter<int>("alarm_every_cycles", cell_sim::kWeldAlarmEveryCycles);

    publisher_ = create_publisher<cell_msgs::msg::WeldCellSnapshot>("cell/weld_snapshot", 10);
    const auto period = std::chrono::duration<double>(period_sec_);
    timer_ = create_wall_timer(
      std::chrono::duration_cast<std::chrono::milliseconds>(period),
      [this]() { on_timer(); });

    RCLCPP_INFO(
      get_logger(),
      "weld_cell_publisher started cell_id=%s period_sec=%.2f",
      cell_id_.c_str(), period_sec_);
  }

 private:
  void on_timer() {
    ++tick_;
    const auto message = cell_sim::make_weld_snapshot(
      now(), cell_id_, tick_, alarm_every_cycles_);

    RCLCPP_INFO(
      get_logger(),
      "publishing: cell_id=%s status=%s arc_on_s=%.1f parts=%u alarms=%zu",
      message.cell_id.c_str(),
      message.status.c_str(),
      message.process.arc_on_s,
      message.parts_total_shift,
      message.alarms.size());
    publisher_->publish(message);
  }

  std::string cell_id_;
  double period_sec_{1.0};
  int alarm_every_cycles_{cell_sim::kWeldAlarmEveryCycles};
  rclcpp::Publisher<cell_msgs::msg::WeldCellSnapshot>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
  std::uint64_t tick_;
};

int main(int argc, char * argv[]) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<WeldCellPublisher>());
  rclcpp::shutdown();
  return 0;
}
