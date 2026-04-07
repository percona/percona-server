/*
(C) 2026 Percona LLC and/or its affiliates

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
*/
#include "jwks.h"

#include <curl/curl.h>

#include <memory>
#include <stdexcept>
#include <string>

#include "jwk.h"

std::size_t Jwks::write_callback(const char *received,
                                 const std::size_t element_size,
                                 const std::size_t no_elements,
                                 void *user_data) {
  const std::size_t total = element_size * no_elements;
  std::string *out = static_cast<std::string *>(user_data);
  out->append(received, total);
  return total;
}

std::string Jwks::http_get(const std::string &url) {
  const std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl(
      curl_easy_init(), curl_easy_cleanup);
  if (curl == nullptr) throw std::runtime_error("JWST: curl_easy_init failed");

  std::string response;

  curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, &Jwks::write_callback);
  curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &response);
  curl_easy_setopt(curl.get(), CURLOPT_USERAGENT, "Jwst/1.0");
  curl_easy_setopt(curl.get(), CURLOPT_NOPROGRESS, 1L);
  curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT, 30L);
  curl_easy_setopt(curl.get(), CURLOPT_MAXREDIRS, 5L);
  curl_easy_setopt(curl.get(), CURLOPT_BUFFERSIZE, 102400L);  // Max 100K

  // SECURITY: the constructor ensures the URL starts with HTTP or HTTPS.
  // HTTP case: no security verification is done, assume
  // the administrator deliberately uses unsafe config (e.g. for testing).
  // HTTPS case: the JWKS endpoint must use a valid certificate.
  if (url.find("https://") == 0) {
    curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYHOST, 2L);
  }
  CURLcode curl_code = curl_easy_perform(curl.get());
  if (curl_code != CURLE_OK) {
    const std::string msg = std::string("JWST: HTTP GET from ") + url +
                            " failed: " + curl_easy_strerror(curl_code);
    throw std::runtime_error(msg);
  }

  long http_code = 0;
  curl_code = curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &http_code);
  if (curl_code != CURLE_OK) {
    const std::string msg = std::string("JWST: CURL get info from ") + url +
                            "failed: " + curl_easy_strerror(curl_code);
    throw std::runtime_error(msg);
  }

  if (http_code < 200 || http_code >= 300) {
    throw std::runtime_error("JWST: unexpected HTTP status from " + url + ": " +
                             std::to_string(http_code));
  }

  return response;
}
