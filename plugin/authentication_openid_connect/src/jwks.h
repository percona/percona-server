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

#include <string>

class Jwks {
private:
  std::string url;
 public:
  const std::string &get_url() const { return url; }

  Jwks() = delete;
  explicit Jwks(const std::string &url)
      : url(url) {}
  static std::string http_get(const std::string &url);

 private:
  static std::size_t write_callback(const char *received, std::size_t element_size,
                                    std::size_t no_elements, void *user_data);
};
#endif  // MYSQL_JWKS_H
