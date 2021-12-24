/* Copyright (c) 2022 Percona LLC and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; version 2 of
   the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#include <stdexcept>
#include <string>

#include <mysql/plugin.h>

#include <mysqlpp/udf_wrappers.hpp>

#include <opensslpp/core_error.hpp>
#include <opensslpp/digest_operations.hpp>
#include <opensslpp/rsa_encrypt_decrypt_operations.hpp>
#include <opensslpp/rsa_key.hpp>
#include <opensslpp/rsa_padding.hpp>

namespace {

//
// CREATE_ASYMMETRIC_PRIV_KEY(@algorithm, {@key_len|@dh_secret})
// This functions generates a private key using the given algorithm
// (@algorithm) and key length (@key_len) or Diffie-Hellman secret(@dh_secret),
// and returns the key as a binary string in PEM format.
// If key generation fails, the result is NULL.
//
class create_asymmetric_priv_key_impl {
 public:
  create_asymmetric_priv_key_impl(mysqlpp::udf_context &ctx) {
    DBUG_TRACE;

    if (ctx.get_number_of_args() != 2)
      throw std::invalid_argument("Function requires exactly two arguments");

    // result
    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);

    // arg0 - @algorithm
    ctx.mark_arg_nullable(0, false);
    ctx.set_arg_type(0, STRING_RESULT);

    // arg1 - @key_len
    ctx.mark_arg_nullable(1, false);
    ctx.set_arg_type(1, INT_RESULT);
  }
  ~create_asymmetric_priv_key_impl() { DBUG_TRACE; }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &args);
};

mysqlpp::udf_result_t<STRING_RESULT> create_asymmetric_priv_key_impl::calculate(
    const mysqlpp::udf_context &ctx) {
  DBUG_TRACE;

  auto algorithm = ctx.get_arg<STRING_RESULT>(0);
  if (algorithm.data() == nullptr)
    throw std::invalid_argument("Algorithm cannot be NULL");
  if (algorithm != "RSA")
    throw std::invalid_argument("Invalid algorithm specified");
  auto optional_length = ctx.get_arg<INT_RESULT>(1);
  if (!optional_length)
    throw std::invalid_argument("Key length cannot be NULL");
  auto length = optional_length.get();
  if (length < 1024 || length > 16384)
    throw std::invalid_argument("Invalid key length specified");

  auto key = opensslpp::rsa_key::generate(length);
  return {opensslpp::rsa_key::export_private_pem(key)};
}

//
// CREATE_ASYMMETRIC_PUB_KEY(@algorithm, @priv_key_str)
// Derives a public key from the given private key using the given algorithm,
// and returns the key as a binary string in PEM format.
// If key derivation fails, the result is NULL.
//
class create_asymmetric_pub_key_impl {
 public:
  create_asymmetric_pub_key_impl(mysqlpp::udf_context &ctx) {
    DBUG_TRACE;

    if (ctx.get_number_of_args() != 2)
      throw std::invalid_argument("Function requires exactly two arguments");

    // result
    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);

    // arg0 - @algorithm
    ctx.mark_arg_nullable(0, false);
    ctx.set_arg_type(0, STRING_RESULT);

    // arg1 - @priv_key_str
    ctx.mark_arg_nullable(1, false);
    ctx.set_arg_type(1, STRING_RESULT);
  }
  ~create_asymmetric_pub_key_impl() { DBUG_TRACE; }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &args);
};

mysqlpp::udf_result_t<STRING_RESULT> create_asymmetric_pub_key_impl::calculate(
    const mysqlpp::udf_context &ctx) {
  DBUG_TRACE;

  auto algorithm = ctx.get_arg<STRING_RESULT>(0);
  if (algorithm.data() == nullptr)
    throw std::invalid_argument("Algorithm cannot be NULL");
  if (algorithm != "RSA")
    throw std::invalid_argument("Invalid algorithm specified");
  auto priv_key_pem = static_cast<std::string>(ctx.get_arg<STRING_RESULT>(1));
  if (priv_key_pem.data() == nullptr)
    throw std::invalid_argument("priv_key_str cannot be NULL");
  auto priv_key = opensslpp::rsa_key::import_private_pem(priv_key_pem);
  return {opensslpp::rsa_key::export_public_pem(priv_key)};
}

//
// ASYMMETRIC_ENCRYPT(@algorithm, @str, @key_str)
// Encrypts a string using the given algorithm and key string, and returns
// the resulting ciphertext as a binary string.
// If encryption fails, the result is NULL.
//
class asymmetric_encrypt_impl {
 public:
  asymmetric_encrypt_impl(mysqlpp::udf_context &ctx) {
    DBUG_TRACE;

    if (ctx.get_number_of_args() != 3)
      throw std::invalid_argument("Function requires exactly three arguments");

    // result
    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);

    // arg0 - @algorithm
    ctx.mark_arg_nullable(0, false);
    ctx.set_arg_type(0, STRING_RESULT);

    // arg1 - @str
    ctx.mark_arg_nullable(1, false);
    ctx.set_arg_type(1, STRING_RESULT);

    // arg2 - @key_str
    ctx.mark_arg_nullable(2, false);
    ctx.set_arg_type(2, STRING_RESULT);
  }
  ~asymmetric_encrypt_impl() { DBUG_TRACE; }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &args);
};

mysqlpp::udf_result_t<STRING_RESULT> asymmetric_encrypt_impl::calculate(
    const mysqlpp::udf_context &ctx) {
  DBUG_TRACE;

  auto algorithm = ctx.get_arg<STRING_RESULT>(0);
  if (algorithm.data() == nullptr)
    throw std::invalid_argument("Algorithm cannot be NULL");
  if (algorithm != "RSA")
    throw std::invalid_argument("Invalid algorithm specified");

  auto message_sv = ctx.get_arg<STRING_RESULT>(1);
  if (message_sv.data() == nullptr)
    throw std::invalid_argument("Message cannot be NULL");
  auto message = static_cast<std::string>(message_sv);

  auto key_pem_sv = ctx.get_arg<STRING_RESULT>(2);
  if (key_pem_sv.data() == nullptr)
    throw std::invalid_argument("key_str cannot be NULL");
  auto key_pem = static_cast<std::string>(key_pem_sv);

  opensslpp::rsa_key key;
  try {
    key = opensslpp::rsa_key::import_private_pem(key_pem);
  } catch (const opensslpp::core_error &) {
    key = opensslpp::rsa_key::import_public_pem(key_pem);
  }

  return {key.is_private() ? opensslpp::encrypt_with_rsa_private_key(
                                 message, key, opensslpp::rsa_padding::pkcs1)
                           : opensslpp::encrypt_with_rsa_public_key(
                                 message, key, opensslpp::rsa_padding::pkcs1)};
}

//
// ASYMMETRIC_DECRYPT(@algorithm, @crypt_str, @key_str)
// Decrypts an encrypted string using the given algorithm and key string, and
// returns the resulting plaintext as a binary string. If decryption fails, the
// result is NULL.
//
class asymmetric_decrypt_impl {
 public:
  asymmetric_decrypt_impl(mysqlpp::udf_context &ctx) {
    DBUG_TRACE;

    if (ctx.get_number_of_args() != 3)
      throw std::invalid_argument("Function requires exactly three arguments");

    // result
    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);

    // arg0 - @algorithm
    ctx.mark_arg_nullable(0, false);
    ctx.set_arg_type(0, STRING_RESULT);

    // arg1 - @crypt_str
    ctx.mark_arg_nullable(1, false);
    ctx.set_arg_type(1, STRING_RESULT);

    // arg2 - @key_str
    ctx.mark_arg_nullable(2, false);
    ctx.set_arg_type(2, STRING_RESULT);
  }
  ~asymmetric_decrypt_impl() { DBUG_TRACE; }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &args);
};

mysqlpp::udf_result_t<STRING_RESULT> asymmetric_decrypt_impl::calculate(
    const mysqlpp::udf_context &ctx) {
  DBUG_TRACE;

  auto algorithm = ctx.get_arg<STRING_RESULT>(0);
  if (algorithm.data() == nullptr)
    throw std::invalid_argument("Algorithm cannot be NULL");
  if (algorithm != "RSA")
    throw std::invalid_argument("Invalid algorithm specified");

  auto message_sv = ctx.get_arg<STRING_RESULT>(1);
  if (message_sv.data() == nullptr)
    throw std::invalid_argument("Message cannot be NULL");
  auto message = static_cast<std::string>(message_sv);

  auto key_pem_sv = ctx.get_arg<STRING_RESULT>(2);
  if (key_pem_sv.data() == nullptr)
    throw std::invalid_argument("key_str cannot be NULL");
  auto key_pem = static_cast<std::string>(key_pem_sv);

  opensslpp::rsa_key key;
  try {
    key = opensslpp::rsa_key::import_private_pem(key_pem);
  } catch (const opensslpp::core_error &) {
    key = opensslpp::rsa_key::import_public_pem(key_pem);
  }

  return {key.is_private() ? opensslpp::decrypt_with_rsa_private_key(
                                 message, key, opensslpp::rsa_padding::pkcs1)
                           : opensslpp::decrypt_with_rsa_public_key(
                                 message, key, opensslpp::rsa_padding::pkcs1)};
}

//
// CREATE_DIGEST(@digest_type, @str)
// Creates a digest from the given string using the given digest type, and
// returns the digest as a binary string.
// If digest generation fails, the result is NULL.
// Supported @digest_type values: 'SHA224', 'SHA256', 'SHA384', 'SHA512'
//
class create_digest_impl {
 public:
  create_digest_impl(mysqlpp::udf_context &ctx) {
    DBUG_TRACE;

    if (ctx.get_number_of_args() != 2)
      throw std::invalid_argument("Function requires exactly two arguments");

    // result
    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);

    // arg0 - @digest_type
    ctx.mark_arg_nullable(0, false);
    ctx.set_arg_type(0, STRING_RESULT);

    // arg1 - @str
    ctx.mark_arg_nullable(1, false);
    ctx.set_arg_type(1, STRING_RESULT);
  }
  ~create_digest_impl() { DBUG_TRACE; }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &args);
};

mysqlpp::udf_result_t<STRING_RESULT> create_digest_impl::calculate(
    const mysqlpp::udf_context &ctx) {
  DBUG_TRACE;

  auto digest_type_sv = ctx.get_arg<STRING_RESULT>(0);
  if (digest_type_sv.data() == nullptr)
    throw std::invalid_argument("Digest type cannot be NULL");
  auto digest_type = static_cast<std::string>(digest_type_sv);

  auto message_sv = ctx.get_arg<STRING_RESULT>(1);
  if (message_sv.data() == nullptr)
    throw std::invalid_argument("Message cannot be NULL");
  auto message = static_cast<std::string>(message_sv);

  return {opensslpp::calculate_digest(digest_type, message)};
}

}  // end of anonymous namespace

DECLARE_STRING_UDF(create_asymmetric_priv_key_impl, create_asymmetric_priv_key)
DECLARE_STRING_UDF(create_asymmetric_pub_key_impl, create_asymmetric_pub_key)
DECLARE_STRING_UDF(asymmetric_encrypt_impl, asymmetric_encrypt)
DECLARE_STRING_UDF(asymmetric_decrypt_impl, asymmetric_decrypt)
DECLARE_STRING_UDF(create_digest_impl, create_digest)
