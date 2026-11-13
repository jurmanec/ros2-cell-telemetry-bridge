#include <chrono>
#include <cstdint>
#include <memory>
#include <string>

#include "cell_msgs/msg/facility_asset_snapshot.hpp"
#include "cell_sim/facility_snapshot.hpp"
#include "rclcpp/rclcpp.hpp"

using namespace std::chrono_literals;

/**
 * One facility asset: publishes FacilityAssetSnapshot on facility/asset_snapshot.
 * A second and third process remap the node name and set their own asset parameters.
 * Contract: docs/contracts/facility-asset-snapshot.md
 */
class FacilityAssetPublisher : public rclcpp::Node {
 public:
  FacilityAssetPublisher() : Node("facility_asset_publisher"), tick_(0) {
    site_id_ = declare_parameter<std::string>("site_id", "SITE-01");
    hall_id_ = declare_parameter<std::string>("hall_id", "HALL-01");
    asset_id_ = declare_parameter<std::string>("asset_id", "CRAH-01");
    asset_type_ = declare_parameter<std::string>("asset_type", "crah");
    period_sec_ = declare_parameter<double>("period_sec", 1.0);
    fault_at_tick_ = declare_parameter<int>("fault_at_tick", 0);

    publisher_ = create_publisher<cell_msgs::msg::FacilityAssetSnapshot>(
      "facility/asset_snapshot", 10);
    const auto period = std::chrono::duration<double>(period_sec_);
    timer_ = create_wall_timer(
      std::chrono::duration_cast<std::chrono::milliseconds>(period),
      [this]() { on_timer(); });

    RCLCPP_INFO(
      get_logger(),
      "facility_asset_publisher started asset_id=%s asset_type=%s period_sec=%.2f fault_at_tick=%d",
      asset_id_.c_str(), asset_type_.c_str(), period_sec_, fault_at_tick_);
  }

 private:
  void on_timer() {
    ++tick_;
    const auto fault_at = fault_at_tick_ > 0
      ? static_cast<std::uint64_t>(fault_at_tick_)
      : 0;
    const auto message = cell_sim::make_facility_snapshot(
      now(), site_id_, hall_id_, asset_id_, asset_type_, tick_, fault_at);

    RCLCPP_INFO(
      get_logger(),
      "publishing: asset_id=%s status=%s reading=%.1f alarms=%zu",
      message.asset_id.c_str(),
      message.status.c_str(),
      message.reading,
      message.alarms.size());
    publisher_->publish(message);
  }

  std::string site_id_;
  std::string hall_id_;
  std::string asset_id_;
  std::string asset_type_;
  double period_sec_{1.0};
  int fault_at_tick_{0};
  rclcpp::Publisher<cell_msgs::msg::FacilityAssetSnapshot>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
  std::uint64_t tick_;
};

int main(int argc, char * argv[]) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<FacilityAssetPublisher>());
  rclcpp::shutdown();
  return 0;
}
