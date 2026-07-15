#include <curl/curl.h>

#include <chrono>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "cell_bridge/batch_json.hpp"
#include "cell_bridge/http_client.hpp"
#include "cell_msgs/msg/cell_telemetry.hpp"
#include "rclcpp/rclcpp.hpp"

using namespace std::chrono_literals;

/**
 * Subscribes to cell/telemetry, batches samples, POSTs JSON to ingest.
 * Contract: docs/contracts/telemetry-batch-http.md
 * Retry: docs/bridge-http-retry.md
 */
class CellBridgeNode : public rclcpp::Node {
 public:
  CellBridgeNode()
  : Node("cell_bridge")
  {
    ingest_url_ = declare_parameter<std::string>(
      "ingest_url", "http://ingest:8080/v1/telemetry/batch");
    api_key_ = declare_parameter<std::string>("api_key", "");
    batch_config_.cell_id = declare_parameter<std::string>("cell_id", "cell-001");
    batch_size_ = declare_parameter<int>("batch_size", 5);
    flush_period_sec_ = declare_parameter<double>("flush_period_sec", 1.0);
    max_attempts_ = declare_parameter<int>("max_attempts", 3);

    curl_global_init(CURL_GLOBAL_DEFAULT);

    subscription_ = create_subscription<cell_msgs::msg::CellTelemetry>(
      "cell/telemetry", 10,
      [this](const cell_msgs::msg::CellTelemetry & msg) { on_telemetry(msg); });

    const auto period = std::chrono::duration<double>(flush_period_sec_);
    flush_timer_ = create_wall_timer(
      std::chrono::duration_cast<std::chrono::milliseconds>(period),
      [this]() { flush_due_to_timer(); });

    RCLCPP_INFO(
      get_logger(),
      "cell_bridge started ingest_url=%s cell_id=%s batch_size=%d api_key_set=%s",
      ingest_url_.c_str(), batch_config_.cell_id.c_str(), batch_size_,
      api_key_.empty() ? "false" : "true");
  }

  ~CellBridgeNode() override
  {
    flush_all();
    curl_global_cleanup();
  }

 private:
  void on_telemetry(const cell_msgs::msg::CellTelemetry & msg)
  {
    std::vector<cell_msgs::msg::CellTelemetry> to_send;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      buffer_.push_back(msg);
      if (static_cast<int>(buffer_.size()) >= batch_size_) {
        to_send.swap(buffer_);
      }
    }
    if (!to_send.empty()) {
      deliver(to_send);
    }
  }

  void flush_due_to_timer()
  {
    std::vector<cell_msgs::msg::CellTelemetry> to_send;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (buffer_.empty()) {
        return;
      }
      to_send.swap(buffer_);
    }
    deliver(to_send);
  }

  void flush_all()
  {
    std::vector<cell_msgs::msg::CellTelemetry> to_send;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      to_send.swap(buffer_);
    }
    if (!to_send.empty()) {
      deliver(to_send);
    }
  }

  void deliver(const std::vector<cell_msgs::msg::CellTelemetry> & samples)
  {
    const std::string body = cell_bridge::samples_to_batch_json(batch_config_, samples);
    bool delivered = false;

    for (int attempt = 1; attempt <= max_attempts_; ++attempt) {
      const auto result = cell_bridge::http_post_json(ingest_url_, body, 2, api_key_);
      if (result.ok) {
        RCLCPP_INFO(
          get_logger(),
          "ingest ok attempt=%d status=%ld samples=%zu cell_id=%s",
          attempt, result.status, samples.size(), batch_config_.cell_id.c_str());
        delivered = true;
        break;
      }

      const bool retryable = result.status == 0 || result.status >= 500;
      RCLCPP_WARN(
        get_logger(),
        "ingest fail attempt=%d status=%ld error=%s samples=%zu retryable=%s",
        attempt, result.status, result.error.c_str(), samples.size(),
        retryable ? "true" : "false");

      if (!retryable || attempt == max_attempts_) {
        break;
      }
      std::this_thread::sleep_for(attempt == 1 ? 200ms : 500ms);
    }

    if (!delivered) {
      RCLCPP_ERROR(
        get_logger(),
        "ingest dropped batch samples=%zu cell_id=%s",
        samples.size(), batch_config_.cell_id.c_str());
    }
  }

  cell_bridge::BatchConfig batch_config_;
  std::string ingest_url_;
  std::string api_key_;
  int batch_size_{5};
  double flush_period_sec_{1.0};
  int max_attempts_{3};
  // Protects buffer_. Redundant under SingleThreadedExecutor (rclcpp::spin),
  // but required if we switch to a MultiThreadedExecutor later.
  std::mutex mutex_;
  std::vector<cell_msgs::msg::CellTelemetry> buffer_;

  rclcpp::Subscription<cell_msgs::msg::CellTelemetry>::SharedPtr subscription_;
  rclcpp::TimerBase::SharedPtr flush_timer_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<CellBridgeNode>());
  rclcpp::shutdown();
  return 0;
}
