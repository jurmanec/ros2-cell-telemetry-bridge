#include <curl/curl.h>

#include <chrono>
#include <memory>
#include <optional>
#include <string>
#include <thread>

#include "cell_bridge/http_client.hpp"
#include "cell_bridge/weld_reduce.hpp"
#include "cell_bridge/weld_snapshot_json.hpp"
#include "cell_msgs/msg/weld_cell_snapshot.hpp"
#include "rclcpp/rclcpp.hpp"

using namespace std::chrono_literals;

/**
 * Subscribes to cell/weld_snapshot and POSTs one schema_version 2 JSON body.
 * cell_id is copied from the snapshot. This node has no cell_id parameter.
 * Contract: docs/contracts/weld-cell-snapshot.md
 * Reduction: docs/contracts/weld-bridge-reduction.md
 * Retry: docs/bridge-http-retry.md
 */
class WeldBridgeNode : public rclcpp::Node {
 public:
  WeldBridgeNode()
  : Node("weld_cell_bridge")
  {
    ingest_url_ = declare_parameter<std::string>(
      "ingest_url", "http://ingest:8080/v1/telemetry/batch");
    api_key_ = declare_parameter<std::string>("api_key", "");
    max_attempts_ = declare_parameter<int>("max_attempts", 3);
    const auto reduce = declare_parameter<std::string>("reduce", "all");
    const auto every_n = declare_parameter<int>("every_n", 5);
    const auto heartbeat_sec = declare_parameter<double>("heartbeat_sec", 5.0);
    reduce_.emplace(reduce, every_n, heartbeat_sec);

    curl_global_init(CURL_GLOBAL_DEFAULT);

    subscription_ = create_subscription<cell_msgs::msg::WeldCellSnapshot>(
      "cell/weld_snapshot", 10,
      [this](const cell_msgs::msg::WeldCellSnapshot & msg) { on_snapshot(msg); });

    RCLCPP_INFO(
      get_logger(),
      "weld_cell_bridge started ingest_url=%s api_key_set=%s reduce=%s every_n=%d heartbeat_sec=%.1f",
      ingest_url_.c_str(), api_key_.empty() ? "false" : "true",
      reduce_->reduce().c_str(), static_cast<int>(every_n), heartbeat_sec);
  }

  ~WeldBridgeNode() override
  {
    curl_global_cleanup();
  }

 private:
  void on_snapshot(const cell_msgs::msg::WeldCellSnapshot & msg)
  {
    if (msg.cell_id.empty()) {
      RCLCPP_ERROR(get_logger(), "dropping weld snapshot with empty cell_id");
      return;
    }
    const bool has_alarm = !msg.alarms.empty();
    if (!reduce_->should_forward(msg.cell_id, has_alarm, now().seconds())) {
      RCLCPP_INFO(
        get_logger(),
        "reduce skip cell_id=%s reduce=%s alarms=%zu",
        msg.cell_id.c_str(), reduce_->reduce().c_str(), msg.alarms.size());
      return;
    }
    post_snapshot(msg);
  }

  void post_snapshot(const cell_msgs::msg::WeldCellSnapshot & snapshot)
  {
    const std::string body = cell_bridge::weld_snapshot_to_json(snapshot);
    bool delivered = false;

    for (int attempt = 1; attempt <= max_attempts_; ++attempt) {
      const auto result = cell_bridge::http_post_json(ingest_url_, body, 2, api_key_);
      if (result.ok) {
        RCLCPP_INFO(
          get_logger(),
          "ingest ok attempt=%d status=%ld cell_id=%s alarms=%zu",
          attempt, result.status, snapshot.cell_id.c_str(), snapshot.alarms.size());
        delivered = true;
        break;
      }

      const bool retryable = result.status == 0 || result.status >= 500;
      RCLCPP_WARN(
        get_logger(),
        "ingest fail attempt=%d status=%ld error=%s cell_id=%s retryable=%s",
        attempt, result.status, result.error.c_str(), snapshot.cell_id.c_str(),
        retryable ? "true" : "false");

      if (!retryable || attempt == max_attempts_) {
        break;
      }
      std::this_thread::sleep_for(attempt == 1 ? 200ms : 500ms);
    }

    if (!delivered) {
      RCLCPP_ERROR(
        get_logger(),
        "ingest dropped snapshot cell_id=%s",
        snapshot.cell_id.c_str());
    }
  }

  std::string ingest_url_;
  std::string api_key_;
  int max_attempts_{3};
  std::optional<cell_bridge::WeldReducePolicy> reduce_;
  rclcpp::Subscription<cell_msgs::msg::WeldCellSnapshot>::SharedPtr subscription_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<WeldBridgeNode>());
  rclcpp::shutdown();
  return 0;
}
