#include <chrono>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

using namespace std::chrono_literals;

/**
 * Disposable spike publisher: emits a simple cell telemetry line on
 * /cell/telemetry. Not production code — proves rclcpp + Docker path.
 */
class CellPublisher : public rclcpp::Node {
 public:
  CellPublisher() : Node("cell_publisher"), cycle_(0) {
    publisher_ = create_publisher<std_msgs::msg::String>("cell/telemetry", 10);
    timer_ = create_wall_timer(500ms, [this]() { on_timer(); });
    RCLCPP_INFO(get_logger(), "cell_publisher started (spike)");
  }

 private:
  void on_timer() {
    ++cycle_;
    // Toy metrics: temperature drifts, vibration, cycle count, fault flag
    const double temperature_c = 40.0 + (cycle_ % 20) * 0.5;
    const double vibration = 0.1 + (cycle_ % 7) * 0.02;
    const int fault = (cycle_ % 50 == 0) ? 1 : 0;

    auto message = std_msgs::msg::String();
    message.data = "cycle=" + std::to_string(cycle_) +
                   " temp_c=" + std::to_string(temperature_c) +
                   " vibration=" + std::to_string(vibration) +
                   " fault=" + std::to_string(fault);

    RCLCPP_INFO(get_logger(), "publishing: %s", message.data.c_str());
    publisher_->publish(message);
  }

  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
  int cycle_;
};

int main(int argc, char * argv[]) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<CellPublisher>());
  rclcpp::shutdown();
  return 0;
}
