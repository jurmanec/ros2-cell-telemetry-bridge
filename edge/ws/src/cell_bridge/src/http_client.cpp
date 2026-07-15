#include "cell_bridge/http_client.hpp"

#include <curl/curl.h>

namespace cell_bridge {

namespace {

size_t discard_write(char * /*ptr*/, size_t size, size_t nmemb, void * /*userdata*/)
{
  return size * nmemb;
}

}  // namespace

HttpResult http_post_json(
  const std::string & url,
  const std::string & json_body,
  long timeout_sec,
  const std::string & api_key)
{
  HttpResult result;
  CURL * curl = curl_easy_init();
  if (!curl) {
    result.error = "curl_easy_init failed";
    return result;
  }

  struct curl_slist * headers = nullptr;
  headers = curl_slist_append(headers, "Content-Type: application/json");
  if (!api_key.empty()) {
    const std::string line = "x-api-key: " + api_key;
    headers = curl_slist_append(headers, line.c_str());
  }

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_body.c_str());
  curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(json_body.size()));
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout_sec);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, discard_write);

  const CURLcode code = curl_easy_perform(curl);
  if (code != CURLE_OK) {
    result.error = curl_easy_strerror(code);
  } else {
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &result.status);
    result.ok = (result.status >= 200 && result.status < 300);
    if (!result.ok) {
      result.error = "HTTP status " + std::to_string(result.status);
    }
  }

  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);
  return result;
}

}  // namespace cell_bridge
