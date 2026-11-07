#pragma once

#include <stdexcept>
#include <string>
#include <unordered_map>

namespace cell_bridge {

/**
 * One reduction policy for every welding cell.
 * Count or last-forward time is tracked per cell_id.
 * Contract: docs/contracts/weld-bridge-reduction.md
 */
class WeldReducePolicy {
 public:
  WeldReducePolicy(std::string reduce, int every_n, double heartbeat_sec)
  : reduce_(std::move(reduce)), every_n_(every_n), heartbeat_sec_(heartbeat_sec)
  {
    if (reduce_ != "all" && reduce_ != "every_n" && reduce_ != "heartbeat") {
      throw std::invalid_argument("reduce must be all, every_n, or heartbeat");
    }
    if (every_n_ < 1) {
      throw std::invalid_argument("every_n must be >= 1");
    }
    if (heartbeat_sec_ <= 0.0) {
      throw std::invalid_argument("heartbeat_sec must be > 0");
    }
  }

  const std::string & reduce() const { return reduce_; }

  /** now_sec is the bridge clock when the snapshot is handled. */
  bool should_forward(const std::string & cell_id, bool has_alarm, double now_sec)
  {
    if (reduce_ == "all") {
      return true;
    }
    if (reduce_ == "every_n") {
      int & count = counts_[cell_id];
      ++count;
      return has_alarm || (count % every_n_ == 0);
    }

    const auto previous = last_forward_sec_.find(cell_id);
    const bool due = previous == last_forward_sec_.end() ||
      (now_sec - previous->second) >= heartbeat_sec_;
    if (has_alarm || due) {
      last_forward_sec_[cell_id] = now_sec;
      return true;
    }
    return false;
  }

 private:
  std::string reduce_;
  int every_n_;
  double heartbeat_sec_;
  std::unordered_map<std::string, int> counts_;
  std::unordered_map<std::string, double> last_forward_sec_;
};

}  // namespace cell_bridge
