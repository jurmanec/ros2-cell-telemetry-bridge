#pragma once

#include <string>

namespace cell_bridge {

struct HttpResult {
  bool ok{false};
  long status{0};
  std::string error;
};

/**
 * POST JSON body to url. timeout_sec applies to the whole request.
 * If api_key is non-empty, sends header x-api-key (M3 cloud auth).
 */
HttpResult http_post_json(
  const std::string & url,
  const std::string & json_body,
  long timeout_sec = 2,
  const std::string & api_key = "");

}  // namespace cell_bridge
