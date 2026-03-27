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

#include "config.h"

#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/core_names.h>
#include <openssl/evp.h>
#include <openssl/param_build.h>
#include <openssl/params.h>
#include <openssl/pem.h>
#include <picojson/picojson.h>
#include <algorithm>
#include <exception>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include "mysql/my_loglevel.h"
#include "mysql/service_thd_alloc.h"

#include <mysql/components/services/log_builtins.h>
#include <mysqld_error.h>

const Idp_configs *Idp_configs::config(nullptr);
char *Idp_configs::sysvar(nullptr);

template <typename T, T *(*alloc)(), void (*dealloc)(T *)>
class Raii {
 private:
  T *ptr;

 public:
  Raii() : ptr(alloc()) {
    if (ptr == nullptr) throw std::bad_alloc();
  }
  explicit Raii(T *ptr) : ptr(ptr) {}

  ~Raii() {
    if (ptr) dealloc(ptr);
  }

  T *get() const noexcept { return ptr; }
  T &operator*() const noexcept { return *ptr; }
  T *operator->() const noexcept { return ptr; }
};

struct Idp {
  std::string kty{};
  std::string use{};
  std::string kid{};
  std::string alg{};
  std::string key{};
};

class Jwk {
 protected:
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

std::vector<unsigned char> Jwk::base64url_decode(const std::string &input) {
  std::string s = input;
  const size_t pad = s.size() % 4;
  if (pad != 0) s.append(4 - pad, '=');
  std::ranges::replace(s, '-', '+');
  std::ranges::replace(s, '_', '/');

  std::vector<unsigned char> output((s.size() / 4) * 3);
  const int len = EVP_DecodeBlock(
      output.data(), reinterpret_cast<const unsigned char *>(s.data()),
      s.size());
  if (len < 0) throw std::runtime_error("Base64 decode failed");
  output.resize(len - (pad == 0 ? 0 : 4 - pad));
  return output;
}

OSSL_PARAM *Rsa_jwk::construct_param() {
  if (n.empty() || e.empty()) throw std::runtime_error("RSA requires n and e");

  auto n_bytes = base64url_decode(n);
  auto e_bytes = base64url_decode(e);
  Raii<BIGNUM, nullptr, BN_free> bn_n(
      BN_bin2bn(n_bytes.data(), n_bytes.size(), nullptr));
  Raii<BIGNUM, nullptr, BN_free> bn_e(
      BN_bin2bn(e_bytes.data(), e_bytes.size(), nullptr));

  // BN for RSA
  const Raii<OSSL_PARAM_BLD, OSSL_PARAM_BLD_new, OSSL_PARAM_BLD_free> param_bld;
  if ((OSSL_PARAM_BLD_push_BN(param_bld.get(), OSSL_PKEY_PARAM_RSA_N,
                              bn_n.get()) == 0) ||
      (OSSL_PARAM_BLD_push_BN(param_bld.get(), OSSL_PKEY_PARAM_RSA_E,
                              bn_e.get()) == 0))
    throw std::runtime_error("Failed to push BN params for RSA");

  return OSSL_PARAM_BLD_to_param(param_bld.get());
}

OSSL_PARAM *Ec_jwk::construct_param() {
  if (crv.empty() || x.empty() || y.empty())
    throw std::runtime_error("EC requires crv, x, y");

  std::string curve_name;
  auto crv_bytes = base64url_decode(crv);
  if (crv_bytes == base64url_decode("P-256"))
    curve_name = "P-256";  // Bez OSSL_CURVE_P_256
  else if (crv_bytes == base64url_decode("P-384"))
    curve_name = "P-384";
  else if (crv_bytes == base64url_decode("P-521"))
    curve_name = "P-521";
  else
    throw std::runtime_error("Unsupported EC curve: " + crv);

  auto x_bytes = base64url_decode(x);
  auto y_bytes = base64url_decode(y);

  // Uncompressed public key point: 0x04 + X + Y
  std::vector<unsigned char> pub_key_octet = {0x04};
  pub_key_octet.insert(pub_key_octet.end(), x_bytes.begin(), x_bytes.end());
  pub_key_octet.insert(pub_key_octet.end(), y_bytes.begin(), y_bytes.end());

  const Raii<OSSL_PARAM_BLD, OSSL_PARAM_BLD_new, OSSL_PARAM_BLD_free> param_bld;
  if ((OSSL_PARAM_BLD_push_utf8_string(param_bld.get(),
                                       OSSL_PKEY_PARAM_GROUP_NAME,
                                       curve_name.c_str(), 0) == 0) ||
      (OSSL_PARAM_BLD_push_octet_string(
           param_bld.get(), OSSL_PKEY_PARAM_PUB_KEY, pub_key_octet.data(),
           pub_key_octet.size()) == 0))
    throw std::runtime_error("Failed to build EC params");

  return OSSL_PARAM_BLD_to_param(param_bld.get());
}

inline EVP_PKEY *pkey_from_ctx(EVP_PKEY_CTX *ctx, OSSL_PARAM *params) {
  EVP_PKEY *pkey(nullptr);
  if (ctx == nullptr || (EVP_PKEY_fromdata_init(ctx) == 0) ||
      (EVP_PKEY_fromdata(ctx, &pkey, EVP_PKEY_PUBLIC_KEY, params) == 0)) {
    throw std::runtime_error("RSA EVP_PKEY_fromdata failed");
  }
  return pkey;
}

std::string Jwk::to_pem() {
  const Raii<OSSL_PARAM, nullptr, OSSL_PARAM_free> param(construct_param());

  const Raii<EVP_PKEY_CTX, nullptr,
             [](EVP_PKEY_CTX *ctx) { EVP_PKEY_CTX_free(ctx); }>
      ctx(EVP_PKEY_CTX_new_from_name(nullptr, kty.c_str(), nullptr));

  Raii<EVP_PKEY, nullptr, EVP_PKEY_free> pkey(
      pkey_from_ctx(ctx.get(), param.get()));
  const Raii<BIO, []() { return BIO_new(BIO_s_mem()); },
             [](BIO *bio) { BIO_free(bio); }>
      bio;
  std::string pem;

  if (PEM_write_bio_PUBKEY(bio.get(), pkey.get()) == 0)
    throw std::runtime_error("PEM_write_bio_PUBKEY failed");

  BUF_MEM *mem(nullptr);
  BIO_get_mem_ptr(bio.get(), &mem);
  pem.assign(mem->data, mem->length);
  return pem;
}

int Idp_configs::check(MYSQL_THD thd [[maybe_unused]],
                       SYS_VAR *var [[maybe_unused]], void *save,
                       st_mysql_value *value) {
  int value_len(0);
  Idp_configs *new_config(
      Idp_configs::parse_var(value->val_str(value, nullptr, &value_len)));
  if (new_config == nullptr) return 1;
  *static_cast<const Idp_configs **>(save) = new_config;
  // TODO what if sth fails between check and update? -> avoid memory leak
  return 0;
}

void Idp_configs::update(MYSQL_THD thd [[maybe_unused]],
                         SYS_VAR *var [[maybe_unused]],
                         void *var_ptr [[maybe_unused]], const void *save) {
  const Idp_configs *prev_config(config);
  Idp_configs::config = *static_cast<Idp_configs *const *>(save);
  if (prev_config != nullptr) {
    delete prev_config;
  }
}

std::string Idp_configs::get_str(const picojson::object &obj,
                                 const std::string &key,
                                 const std::string &from, bool is_mandatory) {
  auto it = obj.find(key);
  if (it == obj.end() || !it->second.is<std::string>()) {
    if (is_mandatory)
      throw std::runtime_error("missing " + key + " in " + from);
    return "";
  }
  return it->second.get<std::string>();
}

void Idp_configs::parse_json(const std::string &config_json) {
  picojson::value json_obj;
  if (std::string err = picojson::parse(json_obj, config_json); !err.empty())
    throw std::runtime_error(err);

  if (!json_obj.is<picojson::object>())
    throw std::runtime_error("incorrect configuration structure");

  for (const picojson::object &obj = json_obj.get<picojson::object>();
       const auto &[issuer, jwk] : obj) {
    if (!jwk.is<picojson::object>())
      throw std::runtime_error("incorrect JWK definition of " + issuer);

    const picojson::object &jwk_obj = jwk.get<picojson::object>();

    std::string name = get_str(jwk_obj, "name", issuer);
    std::string kty = get_str(jwk_obj, "kty", issuer);
    std::string use = get_str(jwk_obj, "use", issuer, false);
    std::string key;
    if (kty == "RSA") {
      Rsa_jwk rsa_jwk(get_str(jwk_obj, "n", issuer),
                      get_str(jwk_obj, "e", issuer));
      key = rsa_jwk.to_pem();
    } else if (kty == "EC") {
      Ec_jwk ec_jwk(get_str(jwk_obj, "crv", issuer),
                    get_str(jwk_obj, "x", issuer),
                    get_str(jwk_obj, "y", issuer));
      key = ec_jwk.to_pem();
    } else
      throw std::runtime_error(std::string("invalid kty in ") + issuer);

    idp_configs[issuer] = {name, use, key};
  }
}

static constexpr size_t prefix_len{sizeof("FILE://") - 1};

char parse_prefix(const std::string &prefix) {
  static constexpr std::string_view file_prefix{"FILE://"};
  static constexpr std::string_view json_prefix{"JSON://"};

  if (prefix.size() != prefix_len) return 0;

  // Case-insensitive prefix check
  if (prefix[0] == file_prefix[0]) {
    for (size_t i = 0; i < prefix_len; ++i) {
      if (std::toupper(prefix[i]) != file_prefix[i]) return 0;
    }
    return 'F';
  }
  if (prefix[0] == json_prefix[0]) {
    for (size_t i = 0; i < prefix_len; ++i) {
      if (std::toupper(prefix[i]) != json_prefix[i]) return 0;
    }
    return 'J';
  }

  return 0;
}

std::string read_from_file(const std::string &path) {
  std::ifstream file(path, std::ios::binary | std::ios::ate);
  if (!file) {
    throw std::runtime_error("cannot open config file: " + path);
  }

  auto size = file.tellg();
  if (size < 0) {
    throw std::runtime_error("cannot determine size of config file: " + path);
  }

  std::string content(size, '\0');
  file.seekg(0, std::ios::beg);
  if (!file.read(content.data(), size)) {
    throw std::runtime_error("cannot read config file: " + path);
  }

  return content;
}

Idp_configs *Idp_configs::parse_var(const std::string &config_var) {
  // nothing changed
  if (config_var == sysvar) return nullptr;

  try {
    if (config_var.size() < prefix_len)
      throw std::runtime_error("invalid prefix");
    std::string prefix = config_var.substr(0, prefix_len);
    std::string config_json;
    switch (parse_prefix(prefix)) {
      case 'F':
        config_json = read_from_file(config_var.substr(prefix_len - 1));
        break;
      case 'J':
        config_json = config_var.substr(prefix_len);
        break;
      default:
        throw std::runtime_error("invalid prefix");
    }

    return new Idp_configs(config_var, config_json);
  } catch (const std::exception &e) {
    std::string error_msg("configuration error: ");
    error_msg += e.what();
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG, error_msg.c_str());
  } catch (...) {
    LogPluginErr(ERROR_LEVEL, ER_LOG_PRINTF_MSG, "configuration error");
  }
  return nullptr;
}