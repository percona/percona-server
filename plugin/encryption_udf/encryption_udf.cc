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

#include <boost/lexical_cast/try_lexical_convert.hpp>

#include <mysql/plugin.h>

#include <mysqlpp/udf_wrappers.hpp>

#include <opensslpp/core_error.hpp>
#include <opensslpp/dh_compute_operations.hpp>
#include <opensslpp/dh_key.hpp>
#include <opensslpp/dh_padding.hpp>
#include <opensslpp/digest_operations.hpp>
#include <opensslpp/dsa_key.hpp>
#include <opensslpp/dsa_sign_verify_operations.hpp>
#include <opensslpp/rsa_encrypt_decrypt_operations.hpp>
#include <opensslpp/rsa_key.hpp>
#include <opensslpp/rsa_padding.hpp>
#include <opensslpp/rsa_sign_verify_operations.hpp>

namespace {

// CREATE_ASYMMETRIC_PRIV_KEY(@algorithm, {@key_len|@dh_parameters})
// This functions generates a private key using the given algorithm
// (@algorithm) and key length (@key_len) or Diffie-Hellman
// secret(@dh_parameters), and returns the key as a binary string in PEM format.
// If key generation fails, the result is NULL.
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

    // arg1 - @key_len|@dh_parameters
    ctx.mark_arg_nullable(1, false);
    ctx.set_arg_type(1, STRING_RESULT);
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
  if (algorithm != "RSA" && algorithm != "DSA" && algorithm != "DH")
    throw std::invalid_argument("Invalid algorithm specified");
  auto length_or_dh_parameters = ctx.get_arg<STRING_RESULT>(1);

  std::string pem;
  if (algorithm == "DH") {
    if (length_or_dh_parameters.data() == nullptr)
      throw std::invalid_argument("DH parameters cannot be NULL");
    auto dh_parameters_pem = static_cast<std::string>(length_or_dh_parameters);

    auto key = opensslpp::dh_key::import_parameters_pem(dh_parameters_pem);
    key.promote_to_key();
    pem = opensslpp::dh_key::export_private_pem(key);
  } else {
    if (length_or_dh_parameters.data() == nullptr)
      throw std::invalid_argument("Key length cannot be NULL");

    std::uint32_t length = 0;
    if (!boost::conversion::try_lexical_convert(length_or_dh_parameters,
                                                length))
      throw std::invalid_argument("Key length is not a numeric value");

    if (algorithm == "RSA") {
      if (length < 1024 || length > 16384)
        throw std::invalid_argument("Invalid RSA key length specified");
      auto key = opensslpp::rsa_key::generate(length);
      pem = opensslpp::rsa_key::export_private_pem(key);
    } else if (algorithm == "DSA") {
      // DSA max key length must be <= OPENSSL_DSA_MAX_MODULUS_BITS (10000)
      // and be a multiple of 64
      if (length < 1024 || length > 9984)
        throw std::invalid_argument("Invalid DSA key length specified");
      auto key = opensslpp::dsa_key::generate_parameters(length);
      key.promote_to_key();
      pem = opensslpp::dsa_key::export_private_pem(key);
    }
  }

  return {std::move(pem)};
}

// CREATE_ASYMMETRIC_PUB_KEY(@algorithm, @priv_key_str)
// Derives a public key from the given private key using the given algorithm,
// and returns the key as a binary string in PEM format.
// If key derivation fails, the result is NULL.
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
  if (algorithm != "RSA" && algorithm != "DSA" && algorithm != "DH")
    throw std::invalid_argument("Invalid algorithm specified");
  auto priv_key_pem = static_cast<std::string>(ctx.get_arg<STRING_RESULT>(1));
  if (priv_key_pem.data() == nullptr)
    throw std::invalid_argument("Private key cannot be NULL");

  std::string pem;
  if (algorithm == "RSA") {
    auto priv_key = opensslpp::rsa_key::import_private_pem(priv_key_pem);
    pem = opensslpp::rsa_key::export_public_pem(priv_key);
  } else if (algorithm == "DSA") {
    auto priv_key = opensslpp::dsa_key::import_private_pem(priv_key_pem);
    pem = opensslpp::dsa_key::export_public_pem(priv_key);
  } else if (algorithm == "DH") {
    auto priv_key = opensslpp::dh_key::import_private_pem(priv_key_pem);
    pem = opensslpp::dh_key::export_public_pem(priv_key);
  }
  return {std::move(pem)};
}

// ASYMMETRIC_ENCRYPT(@algorithm, @str, @key_str)
// Encrypts a string using the given algorithm and key string, and returns
// the resulting ciphertext as a binary string.
// If encryption fails, the result is NULL.
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
    throw std::invalid_argument("Key cannot be NULL");
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

// ASYMMETRIC_DECRYPT(@algorithm, @crypt_str, @key_str)
// Decrypts an encrypted string using the given algorithm and key string, and
// returns the resulting plaintext as a binary string. If decryption fails, the
// result is NULL.
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
    throw std::invalid_argument("Key cannot be NULL");
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

// CREATE_DIGEST(@digest_type, @str)
// Creates a digest from the given string using the given digest type, and
// returns the digest as a binary string.
// If digest generation fails, the result is NULL.
// Supported @digest_type values: 'SHA224', 'SHA256', 'SHA384', 'SHA512'
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

// ASYMMETRIC_SIGN(@algorithm, @digest_str, @priv_key_str, @digest_type)
// Signs a digest string using a private key string, and returns the signature
// as a binary string.
// If signing fails, the result is NULL.
// @digest_str is the digest string. It can be generated by calling
// CREATE_DIGEST().
// @digest_type indicates the digest algorithm used to generate the digest
// string.
// @priv_key_str is the private key string to use for signing the digest string.
// It must be a valid key string in PEM format.
// @algorithm indicates the encryption algorithm used to create the key.
// Supported @algorithm values: 'RSA', 'DSA'
// Supported @digest_type values: 'SHA224', 'SHA256', 'SHA384', 'SHA512'
class asymmetric_sign_impl {
 public:
  asymmetric_sign_impl(mysqlpp::udf_context &ctx) {
    DBUG_TRACE;

    if (ctx.get_number_of_args() != 4)
      throw std::invalid_argument("Function requires exactly four arguments");

    // result
    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);

    // arg0 - @algorithm
    ctx.mark_arg_nullable(0, false);
    ctx.set_arg_type(0, STRING_RESULT);

    // arg1 - @digest_str
    ctx.mark_arg_nullable(1, false);
    ctx.set_arg_type(1, STRING_RESULT);

    // arg2 - @priv_key_str
    ctx.mark_arg_nullable(2, false);
    ctx.set_arg_type(2, STRING_RESULT);

    // arg3 - @digest_type
    ctx.mark_arg_nullable(3, false);
    ctx.set_arg_type(3, STRING_RESULT);
  }
  ~asymmetric_sign_impl() { DBUG_TRACE; }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &args);
};

mysqlpp::udf_result_t<STRING_RESULT> asymmetric_sign_impl::calculate(
    const mysqlpp::udf_context &ctx) {
  DBUG_TRACE;

  auto algorithm = ctx.get_arg<STRING_RESULT>(0);
  if (algorithm.data() == nullptr)
    throw std::invalid_argument("Algorithm cannot be NULL");
  if (algorithm != "RSA" && algorithm != "DSA")
    throw std::invalid_argument("Invalid algorithm specified");

  auto message_digest_sv = ctx.get_arg<STRING_RESULT>(1);
  if (message_digest_sv.data() == nullptr)
    throw std::invalid_argument("Message digest cannot be NULL");
  auto message_digest = static_cast<std::string>(message_digest_sv);

  auto private_key_pem_sv = ctx.get_arg<STRING_RESULT>(2);
  if (private_key_pem_sv.data() == nullptr)
    throw std::invalid_argument("Private key cannot be NULL");
  auto private_key_pem = static_cast<std::string>(private_key_pem_sv);

  auto digest_type_sv = ctx.get_arg<STRING_RESULT>(3);
  if (digest_type_sv.data() == nullptr)
    throw std::invalid_argument("Digest type cannot be NULL");
  auto digest_type = static_cast<std::string>(digest_type_sv);

  std::string signature;
  if (algorithm == "RSA") {
    auto private_key = opensslpp::rsa_key::import_private_pem(private_key_pem);
    signature = opensslpp::sign_with_rsa_private_key(
        digest_type, message_digest, private_key);
  } else if (algorithm == "DSA") {
    auto private_key = opensslpp::dsa_key::import_private_pem(private_key_pem);
    signature = opensslpp::sign_with_dsa_private_key(
        digest_type, message_digest, private_key);
  }
  return {std::move(signature)};
}

// ASYMMETRIC_VERIFY(@algorithm, @digest_str, @sig_str, @pub_key_str,
// @digest_type) Verifies whether the signature string matches the digest
// string, and returns 1 or 0 to indicate whether verification succeeded or
// failed.
// @digest_str is the digest string. It can be generated by calling
// CREATE_DIGEST().
// @digest_type indicates the digest algorithm used to generate the digest
// string.
// @sig_str is the signature string. It can be generated by calling
// ASYMMETRIC_SIGN().
// @pub_key_str is the public key string of the signer. It corresponds to the
// private key passed to ASYMMETRIC_SIGN() to generate the signature string
// and must be a valid key string in PEM format.
// @algorithm indicates the encryption algorithm used to create the key.
// Supported algorithm values: 'RSA', 'DSA'
// Supported digest_type values: 'SHA224', 'SHA256', 'SHA384', 'SHA512'
class asymmetric_verify_impl {
 public:
  asymmetric_verify_impl(mysqlpp::udf_context &ctx) {
    DBUG_TRACE;

    if (ctx.get_number_of_args() != 5)
      throw std::invalid_argument("Function requires exactly five arguments");

    // result
    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);

    // arg0 - @algorithm
    ctx.mark_arg_nullable(0, false);
    ctx.set_arg_type(0, STRING_RESULT);

    // arg1 - @digest_str
    ctx.mark_arg_nullable(1, false);
    ctx.set_arg_type(1, STRING_RESULT);

    // arg2 - @sig_str
    ctx.mark_arg_nullable(2, false);
    ctx.set_arg_type(2, STRING_RESULT);

    // arg3 - @pub_key_str
    ctx.mark_arg_nullable(3, false);
    ctx.set_arg_type(3, STRING_RESULT);

    // arg4 - @digest_type
    ctx.mark_arg_nullable(4, false);
    ctx.set_arg_type(4, STRING_RESULT);
  }
  ~asymmetric_verify_impl() { DBUG_TRACE; }

  mysqlpp::udf_result_t<INT_RESULT> calculate(const mysqlpp::udf_context &args);
};

mysqlpp::udf_result_t<INT_RESULT> asymmetric_verify_impl::calculate(
    const mysqlpp::udf_context &ctx) {
  DBUG_TRACE;

  auto algorithm = ctx.get_arg<STRING_RESULT>(0);
  if (algorithm.data() == nullptr)
    throw std::invalid_argument("Algorithm cannot be NULL");
  if (algorithm != "RSA" && algorithm != "DSA")
    throw std::invalid_argument("Invalid algorithm specified");

  auto message_digest_sv = ctx.get_arg<STRING_RESULT>(1);
  if (message_digest_sv.data() == nullptr)
    throw std::invalid_argument("Message digest cannot be NULL");
  auto message_digest = static_cast<std::string>(message_digest_sv);

  auto signature_sv = ctx.get_arg<STRING_RESULT>(2);
  if (signature_sv.data() == nullptr)
    throw std::invalid_argument("Signature cannot be NULL");
  auto signature = static_cast<std::string>(signature_sv);

  auto public_key_pem_sv = ctx.get_arg<STRING_RESULT>(3);
  if (public_key_pem_sv.data() == nullptr)
    throw std::invalid_argument("Public key cannot be NULL");
  auto public_key_pem = static_cast<std::string>(public_key_pem_sv);

  auto digest_type_sv = ctx.get_arg<STRING_RESULT>(4);
  if (digest_type_sv.data() == nullptr)
    throw std::invalid_argument("Digest type cannot be NULL");
  auto digest_type = static_cast<std::string>(digest_type_sv);

  bool verification_result = false;
  if (algorithm == "RSA") {
    auto public_key = opensslpp::rsa_key::import_public_pem(public_key_pem);
    verification_result = opensslpp::verify_with_rsa_public_key(
        digest_type, message_digest, signature, public_key);
  } else if (algorithm == "DSA") {
    auto public_key = opensslpp::dsa_key::import_public_pem(public_key_pem);
    verification_result = opensslpp::verify_with_dsa_public_key(
        digest_type, message_digest, signature, public_key);
  }
  return {verification_result ? 1LL : 0LL};
}

// CREATE_DH_PARAMETERS(@key_len)
// Creates parameters for generating a DH private/public key pair and returns
// them in PEM format. The parameters can be passed to
// CREATE_ASYMMETRIC_PRIV_KEY(). If secret generation fails, the result is null.
// Supported @key_len values: The minimum and maximum key lengths in bits are
// 1,024 and 10,000. These key-length limits are constraints imposed by OpenSSL.
class create_dh_parameters_impl {
 public:
  create_dh_parameters_impl(mysqlpp::udf_context &ctx) {
    DBUG_TRACE;

    if (ctx.get_number_of_args() != 1)
      throw std::invalid_argument("Function requires exactly one argument");

    // result
    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);

    // arg0 - @key_len
    ctx.mark_arg_nullable(0, false);
    ctx.set_arg_type(0, INT_RESULT);
  }
  ~create_dh_parameters_impl() { DBUG_TRACE; }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &args);
};

mysqlpp::udf_result_t<STRING_RESULT> create_dh_parameters_impl::calculate(
    const mysqlpp::udf_context &ctx) {
  DBUG_TRACE;

  auto optional_length = ctx.get_arg<INT_RESULT>(0);
  if (!optional_length)
    throw std::invalid_argument("Parameters length cannot be NULL");
  auto length = optional_length.get();

  if (length < 1024 || length > 10000)
    throw std::invalid_argument("Invalid DH parameters length specified");

  auto key = opensslpp::dh_key::generate_parameters(length);
  key.promote_to_key();

  return {opensslpp::dh_key::export_parameters_pem(key)};
}

// ASYMMETRIC_DERIVE(@pub_key_str, @priv_key_str)
// Derives a symmetric key using the private key of one party and the public
// key of another, and returns the resulting key as a binary string.
// If key derivation fails, the result is NULL.
// @pub_key_str and @priv_key_str must be valid key strings in PEM format.
// They must be created using the DH algorithm.
class asymmetric_derive_impl {
 public:
  asymmetric_derive_impl(mysqlpp::udf_context &ctx) {
    DBUG_TRACE;

    if (ctx.get_number_of_args() != 2)
      throw std::invalid_argument("Function requires exactly two arguments");

    // result
    ctx.mark_result_const(false);
    ctx.mark_result_nullable(true);

    // arg0 - @pub_key_str
    ctx.mark_arg_nullable(0, false);
    ctx.set_arg_type(0, STRING_RESULT);

    // arg1 - @priv_key_str
    ctx.mark_arg_nullable(1, false);
    ctx.set_arg_type(1, STRING_RESULT);
  }
  ~asymmetric_derive_impl() { DBUG_TRACE; }

  mysqlpp::udf_result_t<STRING_RESULT> calculate(
      const mysqlpp::udf_context &args);
};

mysqlpp::udf_result_t<STRING_RESULT> asymmetric_derive_impl::calculate(
    const mysqlpp::udf_context &ctx) {
  DBUG_TRACE;

  auto public_key_pem_sv = ctx.get_arg<STRING_RESULT>(0);
  if (public_key_pem_sv.data() == nullptr)
    throw std::invalid_argument("Public key cannot be NULL");
  auto public_key_pem = static_cast<std::string>(public_key_pem_sv);
  auto public_key = opensslpp::dh_key::import_public_pem(public_key_pem);

  auto private_key_pem_sv = ctx.get_arg<STRING_RESULT>(1);
  if (private_key_pem_sv.data() == nullptr)
    throw std::invalid_argument("Private key cannot be NULL");
  auto private_key_pem = static_cast<std::string>(private_key_pem_sv);
  auto private_key = opensslpp::dh_key::import_private_pem(private_key_pem);

  return {opensslpp::compute_dh_key(public_key, private_key,
                                    opensslpp::dh_padding::nist_sp800_56a)};
}

}  // end of anonymous namespace

DECLARE_STRING_UDF(create_asymmetric_priv_key_impl, create_asymmetric_priv_key)
DECLARE_STRING_UDF(create_asymmetric_pub_key_impl, create_asymmetric_pub_key)
DECLARE_STRING_UDF(asymmetric_encrypt_impl, asymmetric_encrypt)
DECLARE_STRING_UDF(asymmetric_decrypt_impl, asymmetric_decrypt)
DECLARE_STRING_UDF(create_digest_impl, create_digest)
DECLARE_STRING_UDF(asymmetric_sign_impl, asymmetric_sign)
DECLARE_INT_UDF(asymmetric_verify_impl, asymmetric_verify)
DECLARE_STRING_UDF(create_dh_parameters_impl, create_dh_parameters)
DECLARE_STRING_UDF(asymmetric_derive_impl, asymmetric_derive)
