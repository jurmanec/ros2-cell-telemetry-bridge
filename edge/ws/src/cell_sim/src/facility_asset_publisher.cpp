#include <chrono>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

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
    unit_ = declare_parameter<std::string>("unit", "");
    healthy_reading_ = declare_parameter<double>("healthy_reading", 0.0);

    const std::vector<int64_t> no_ticks;
    const std::vector<double> no_readings;
    const std::vector<std::string> no_text;
    const auto phase_tick = declare_parameter<std::vector<int64_t>>("phase_tick", no_ticks);
    const auto phase_reading = declare_parameter<std::vector<double>>("phase_reading", no_readings);
    const auto phase_status = declare_parameter<std::vector<std::string>>("phase_status", no_text);
    const auto phase_code = declare_parameter<std::vector<std::string>>("phase_code", no_text);
    const auto phase_severity =
      declare_parameter<std::vector<std::string>>("phase_severity", no_text);
    const auto phase_message =
      declare_parameter<std::vector<std::string>>("phase_message", no_text);

    const auto error = cell_sim::facility_phase_error(
      phase_tick, phase_reading, phase_status, phase_code, phase_severity, phase_message);
    if (!error.empty()) {
      throw std::runtime_error(error);
    }
    phases_ = cell_sim::facility_phases(
      phase_tick, phase_reading, phase_status, phase_code, phase_severity, phase_message);

    publisher_ = create_publisher<cell_msgs::msg::FacilityAssetSnapshot>(
      "facility/asset_snapshot", 10);
    const auto period = std::chrono::duration<double>(period_sec_);
    timer_ = create_wall_timer(
      std::chrono::duration_cast<std::chrono::milliseconds>(period),
      [this]() { on_timer(); });

    RCLCPP_INFO(
      get_logger(),
      "facility_asset_publisher started asset_id=%s asset_type=%s period_sec=%.2f phases=%zu",
      asset_id_.c_str(), asset_type_.c_str(), period_sec_, phases_.size());
  }

 private:
  void on_timer() {
    ++tick_;
    const auto message = cell_sim::make_facility_snapshot(
      now(), site_id_, hall_id_, asset_id_, asset_type_, unit_, healthy_reading_, tick_, phases_);

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
  std::string unit_;
  double period_sec_{1.0};
  double healthy_reading_{0.0};
  std::vector<cell_sim::FacilityPhase> phases_;
  rclcpp::Publisher<cell_msgs::msg::FacilityAssetSnapshot>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
  std::uint64_t tick_;
};

int main(int argc, char * argv[]) {
  rclcpp::init(argc, argv);
  try {
    rclcpp::spin(std::make_shared<FacilityAssetPublisher>());
  } catch (const std::exception & ex) {
    RCLCPP_FATAL(rclcpp::get_logger("facility_asset_publisher"), "%s", ex.what());
    rclcpp::shutdown();
    return 1;
  }
  rclcpp::shutdown();
  return 0;
}
