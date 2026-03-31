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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/
#ifndef MYSQL_JWK_H
#define MYSQL_JWK_H

#include <string>
#include <vector>
#include <openssl/bn.h>

class Jwk {
 protected:
  std::string alg{};
  std::string use{};
  std::string kid{};
  std::string kty{};
  virtual OSSL_PARAM *construct_param() = 0;

 public:
  explicit Jwk(const char *kty) : kty(kty) {}
  Jwk() = delete;
  virtual ~Jwk() = default;
  std::string to_pem();
  static std::vector<unsigned char> base64url_decode(const std::string &input);
};

class Rsa_jwk : public Jwk {
 private:
  std::string n{};
  std::string e{};

 protected:
  OSSL_PARAM *construct_param() override;

 public:
  Rsa_jwk() = delete;
  Rsa_jwk(std::string n, std::string e)
      : Jwk("RSA"), n(std::move(n)), e(std::move(e)) {}
};

class Ec_jwk : public Jwk {
 private:
  std::string crv{};
  std::string x{};
  std::string y{};

 protected:
  OSSL_PARAM *construct_param() override;

 public:
  Ec_jwk() = delete;

  Ec_jwk(std::string crv, std::string x, std::string y)
      : Jwk("EC"), crv(std::move(crv)), x(std::move(x)), y(std::move(y)) {}
};

#endif //MYSQL_JWK_H
