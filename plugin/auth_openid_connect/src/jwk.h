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

#ifndef MYSQL_JWK_H
#define MYSQL_JWK_H

#include <openssl/opensslv.h>
#if OPENSSL_VERSION_NUMBER >= 0x30000000L  // 3.0.0
#define JWK_OPENSSL_3_0
#endif

#ifdef JWK_OPENSSL_3_0
#include <openssl/types.h>
#else
#include <openssl/ossl_typ.h>
#endif
#include <string>
#include <string_view>
#include <utility>
#include <vector>

/**
 * @class Jwk
 * @brief Base class for representing a JSON Web Key.
 */
class Jwk {
 protected:
  std::string alg{};  ///< Algorithm used for the key.
  std::string use{};  ///< Intended use of the key.
  std::string kid{};  ///< Key ID.
  std::string kty{};  ///< Key Type (e.g., "RSA", "EC").

#ifdef JWK_OPENSSL_3_0
  /**
   * @brief Constructs OpenSSL OSSL_PARAM for the key (OpenSSL 3.0).
   * @return A pointer to an array of OSSL_PARAM objects.
   */
  virtual OSSL_PARAM *construct_param() = 0;
#endif
  /**
   * @brief Constructs an EVP_PKEY for the key
   * @return A newly allocated EVP_PKEY; caller takes ownership.
   */
#ifdef JWK_OPENSSL_3_0
  EVP_PKEY *construct_pkey();
#else
  virtual EVP_PKEY *construct_pkey() = 0;
#endif

 public:
  /**
   * @brief Constructs a Jwk object with a given key type.
   * @param kty The key type string.
   */
  explicit Jwk(const char *kty) : kty(kty) {}
  Jwk() = delete;
  Jwk(const Jwk &) = delete;
  Jwk &operator=(const Jwk &) = delete;
  Jwk(Jwk &&) = delete;
  Jwk &operator=(Jwk &&) = delete;
  virtual ~Jwk() = default;

  /**
   * @brief Converts the JWK to PEM format.
   * @return The PEM-encoded public key.
   */
  std::string to_pem();

  /**
   * @brief Decodes a base64url-encoded string.
   * @param input The base64url string to decode.
   * @return A vector containing the decoded bytes.
   */
  static std::vector<unsigned char> base64url_decode(
      const std::string_view input);
};

/**
 * @class Rsa_jwk
 * @brief Represents an RSA public key in JWK format.
 */
class Rsa_jwk : public Jwk {
 private:
  std::string n{};  ///< Modulus.
  std::string e{};  ///< Public exponent.

 protected:
#ifdef JWK_OPENSSL_3_0
  /**
   * @brief Constructs OpenSSL OSSL_PARAM for the RSA key (OpenSSL 3.0).
   * @return A pointer to an array of OSSL_PARAM objects.
   */
  OSSL_PARAM *construct_param() override;
#else
  EVP_PKEY *construct_pkey() override;
#endif

 public:
  Rsa_jwk() = delete;
  /**
   * @brief Constructs an Rsa_jwk object.
   * @param n The RSA modulus in base64url format.
   * @param e The RSA public exponent in base64url format.
   */
  Rsa_jwk(std::string n, std::string e)
      : Jwk("RSA"), n(std::move(n)), e(std::move(e)) {}
};

/**
 * @class Ec_jwk
 * @brief Represents an Elliptic Curve (EC) public key in JWK format.
 */
class Ec_jwk : public Jwk {
 private:
  std::string crv{};  ///< Curve type (e.g., "P-256").
  std::string x{};    ///< X coordinate.
  std::string y{};    ///< Y coordinate.

 protected:
#ifdef JWK_OPENSSL_3_0
  /**
   * @brief Constructs OpenSSL OSSL_PARAM for the EC key (OpenSSL 3.0).
   *
   * Converts the base64url-encoded EC coordinates to OpenSSL OSSL_PARAM format
   * suitable for key creation. Supports curves P-256, P-384, and P-521.
   *
   * @return A pointer to an array of OSSL_PARAM objects.
   * @throws std::runtime_error if crv, x, or y is empty, if the curve is
   *                            unsupported, or if parameter construction fails.
   */
  OSSL_PARAM *construct_param() override;
#else
  EVP_PKEY *construct_pkey() override;
#endif

 public:
  Ec_jwk() = delete;

  /**
   * @brief Constructs an Ec_jwk object.
   * @param crv The curve name.
   * @param x The X coordinate in base64url format.
   * @param y The Y coordinate in base64url format.
   */
  Ec_jwk(std::string crv, std::string x, std::string y)
      : Jwk("EC"), crv(std::move(crv)), x(std::move(x)), y(std::move(y)) {}
};

#endif  // MYSQL_JWK_H
