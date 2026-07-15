#include <chrono>
#include <cstdint>
#include <memory>

#include "cell_msgs/msg/cell_telemetry.hpp"
#include "cell_sim/telemetry_sample.hpp"
#include "rclcpp/rclcpp.hpp"

using namespace std::chrono_literals;

/**
 * Manufacturing cell simulator: publishes cell_msgs/CellTelemetry on cell/telemetry.
 * Contract: docs/contracts/cell-telemetry.md
 */
class CellPublisher : public rclcpp::Node {
 public:
  CellPublisher() : Node("cell_publisher"), cycle_(0) {
    publisher_ = create_publisher<cell_msgs::msg::CellTelemetry>("cell/telemetry", 10);
    timer_ = create_wall_timer(500ms, [this]() { on_timer(); });
    RCLCPP_INFO(get_logger(), "cell_publisher started (M1 CellTelemetry)");
  }

 private:
  void on_timer() {
    ++cycle_;
    const double temperature_c = 40.0 + static_cast<double>(cycle_ % 20) * 0.5;
    const double vibration = 0.1 + static_cast<double>(cycle_ % 7) * 0.02;
    const bool fault = (cycle_ % 50 == 0);

    const auto message = cell_sim::make_telemetry_sample(
      now(), cycle_, temperature_c, vibration, fault);

    RCLCPP_INFO(
      get_logger(),
      "publishing: cycle=%llu temp_c=%.2f vibration=%.3f fault=%s",
      static_cast<unsigned long long>(message.cycle),
      message.temp_c,
      message.vibration,
      message.fault ? "true" : "false");
    publisher_->publish(message);
  }

  rclcpp::Publisher<cell_msgs::msg::CellTelemetry>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
  std::uint64_t cycle_;
};

int main(int argc, char * argv[]) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<CellPublisher>());
  rclcpp::shutdown();
  return 0;
}
