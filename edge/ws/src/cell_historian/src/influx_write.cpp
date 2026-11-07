#include "cell_historian/influx_write.hpp"

#include <curl/curl.h>

#include <string>

namespace cell_historian {

namespace {

size_t capture_write(char * ptr, size_t size, size_t nmemb, void * userdata)
{
  auto * body = static_cast<std::string *>(userdata);
  body->append(ptr, size * nmemb);
  if (body->size() > 512) {
    body->resize(512);
  }
  return size * nmemb;
}

}  // namespace

WriteResult influx_write(
  const std::string & base_url,
  const std::string & org,
  const std::string & bucket,
  const std::string & token,
  const std::string & line,
  long timeout_sec)
{
  WriteResult result;
  CURL * curl = curl_easy_init();
  if (!curl) {
    result.error = "curl_easy_init failed";
    return result;
  }

  std::string url = base_url;
  if (!url.empty() && url.back() == '/') {
    url.pop_back();
  }
  char * org_escaped = curl_easy_escape(curl, org.c_str(), 0);
  char * bucket_escaped = curl_easy_escape(curl, bucket.c_str(), 0);
  url += "/api/v2/write?org=";
  url += org_escaped ? org_escaped : org;
  url += "&bucket=";
  url += bucket_escaped ? bucket_escaped : bucket;
  url += "&precision=ns";
  curl_free(org_escaped);
  curl_free(bucket_escaped);

  struct curl_slist * headers = nullptr;
  headers = curl_slist_append(headers, "Content-Type: text/plain; charset=utf-8");
  const std::string auth = "Authorization: Token " + token;
  headers = curl_slist_append(headers, auth.c_str());

  std::string response;
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, line.c_str());
  curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(line.size()));
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout_sec);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, capture_write);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

  const CURLcode code = curl_easy_perform(curl);
  if (code != CURLE_OK) {
    result.error = curl_easy_strerror(code);
  } else {
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &result.status);
    result.ok = (result.status >= 200 && result.status < 300);
    if (!result.ok) {
      result.error = "HTTP status " + std::to_string(result.status);
      if (!response.empty()) {
        result.error += " " + response;
      }
    }
  }

  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);
  return result;
}

}  // namespace cell_historian
