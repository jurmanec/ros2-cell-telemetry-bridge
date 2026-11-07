#pragma once

#include <string>

namespace cell_historian {

struct WriteResult {
  bool ok{false};
  long status{0};
  std::string error;
};

/** POST one line-protocol body. token is sent as Authorization: Token. */
WriteResult influx_write(
  const std::string & base_url,
  const std::string & org,
  const std::string & bucket,
  const std::string & token,
  const std::string & line,
  long timeout_sec = 2);

}  // namespace cell_historian
