#include <curl/curl.h>

#include <memory>
#include <string>

#include "cell_historian/influx_write.hpp"
#include "cell_historian/weld_line.hpp"
#include "cell_msgs/msg/weld_cell_snapshot.hpp"
#include "rclcpp/rclcpp.hpp"

/**
 * Subscribes to cell/weld_snapshot and writes every snapshot to InfluxDB.
 * It does not filter and it does not post to the cloud.
 */
class WeldHistorianNode : public rclcpp::Node {
 public:
  WeldHistorianNode()
  : Node("weld_historian")
  {
    influx_url_ = declare_parameter<std::string>("influx_url", "http://influxdb:8086");
    influx_org_ = declare_parameter<std::string>("influx_org", "p3");
    influx_bucket_ = declare_parameter<std::string>("influx_bucket", "weld");
    influx_token_ = declare_parameter<std::string>("influx_token", "p3-historian-dev-token");

    curl_global_init(CURL_GLOBAL_DEFAULT);

    subscription_ = create_subscription<cell_msgs::msg::WeldCellSnapshot>(
      "cell/weld_snapshot", 10,
      [this](const cell_msgs::msg::WeldCellSnapshot & msg) { on_snapshot(msg); });

    RCLCPP_INFO(
      get_logger(),
      "weld_historian started influx_url=%s org=%s bucket=%s",
      influx_url_.c_str(), influx_org_.c_str(), influx_bucket_.c_str());
  }

  ~WeldHistorianNode() override
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
    const auto line = cell_historian::weld_snapshot_to_line(msg);
    const auto result = cell_historian::influx_write(
      influx_url_, influx_org_, influx_bucket_, influx_token_, line, 2);
    if (!result.ok) {
      RCLCPP_ERROR(
        get_logger(),
        "historian write failed status=%ld error=%s cell_id=%s",
        result.status, result.error.c_str(), msg.cell_id.c_str());
      return;
    }
    RCLCPP_INFO(
      get_logger(),
      "historian wrote cell_id=%s status=%s alarms=%zu",
      msg.cell_id.c_str(), msg.status.c_str(), msg.alarms.size());
  }

  std::string influx_url_;
  std::string influx_org_;
  std::string influx_bucket_;
  std::string influx_token_;
  rclcpp::Subscription<cell_msgs::msg::WeldCellSnapshot>::SharedPtr subscription_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<WeldHistorianNode>());
  rclcpp::shutdown();
  return 0;
}
