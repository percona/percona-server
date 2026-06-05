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

#ifndef MYSQL_JWKS_H
#define MYSQL_JWKS_H

#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>

/**
 * @class Jwks
 * @brief Manages JSON Web Key Set (JWKS) retrieval via HTTPS.
 *
 * This class handles fetching JWKS from a remote HTTPS endpoint
 * using libcurl. It provides methods for HTTP GET requests and handles
 * response buffering.
 */
class Jwks {
 private:
  std::string url;  ///< The JWKS endpoint URL.

 public:
  /**
   * @brief Gets the JWKS URL.
   * @return The URL string.
   */
  const std::string &get_url() const { return url; }

  Jwks() = delete;

  /**
   * @brief Constructs a Jwks object with a given URL.
   * @param url The URL of the JWKS endpoint.
   */
  explicit Jwks(const std::string_view url) {
    if (!url.empty() && url.find("http://") != 0 && url.find("https://") != 0) {
      throw std::runtime_error("JWKS URL is not valid");
    }
    this->url = url;
  }

  /**
   * @brief Performs an HTTP GET request to the given URL.
   *
   * @return The response body as a string.
   * @throws std::runtime_error if curl initialization fails, HTTP request
   *                            fails, or HTTP status code indicates an error.
   *
   * @note This method does NOT enforce HTTPS, but logs a warning if HTTP is
   * used.
   */
  std::string http_get() const;

 private:
  /**
   * @brief Callback function for writing HTTP response data.
   *
   * @param received Pointer to the received data buffer.
   * @param element_size Size of each element.
   * @param no_elements Number of elements received.
   * @param user_data Pointer to the output std::string* buffer.
   * @return The number of bytes processed (element_size * no_elements).
   */
  static std::size_t write_callback(const char *received,
                                    std::size_t element_size,
                                    std::size_t no_elements, void *user_data);
};
#endif  // MYSQL_JWKS_H
