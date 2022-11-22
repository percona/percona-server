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

#include <array>
#include <condition_variable>
#include <mutex>

#include "base64.h"
#include "m_string.h"
#include "my_dbug.h"
#include "my_sys.h"
#include "mysql/components/my_service.h"
#include "mysql/components/service_implementation.h"
#include "mysql/components/services/mysql_authentication_registration.h"
#include "mysql/plugin_auth.h"
#include "mysqld_error.h"
#include "sql/current_thd.h"
#include "sql/sql_class.h"

#include <fido.h>
#include <fido/es256.h>

#include <openssl/ec.h>
#include <openssl/rand.h>

static char *rpid;
static MYSQL_SYSVAR_STR(
    rp_id, rpid, PLUGIN_VAR_RQCMDARG | PLUGIN_VAR_MEMALLOC,
    "The relying party ID used for FIDO device registration and authentication",
    nullptr, nullptr, "MySQL");

struct SYS_VAR;
static SYS_VAR *fido_sysvars[] = {MYSQL_SYSVAR(rp_id), nullptr};

struct SYS_VAR {
  MYSQL_PLUGIN_VAR_HEADER;
};

MYSQL_PLUGIN authentication_fido_pluogin_info;

using fido_pubkey_t = std::array<unsigned char, 64>;
bool reconstruct_fido_cred(fido_cred_t *cred, const char *data,
                           std::size_t data_len, fido_pubkey_t &pubkey);

namespace authentication_fido_reg {
// Methods for the registration component interface
DEFINE_BOOL_METHOD(init, (unsigned char **outbuf, unsigned int)) {
  unsigned char data[512];

  unsigned char *pos = data;
  pos = net_store_length(pos, 32);
  RAND_bytes(pos, 32);
  pos += 32;

  // relying party ID
  pos = net_store_length(pos, strlen(rpid));
  memcpy(pos, rpid, strlen(rpid));
  pos += strlen(rpid);

  // last is username.
  // Caller code is supposed to add this, but that doesn't work
  // with the client plugin, as it wouldn't be part of the base64
  // encoding.

  size_t user_name_len = current_thd->security_context()->user().length +
                         current_thd->security_context()->host().length;

  pos = net_store_length(pos, user_name_len);

  memcpy(pos, current_thd->security_context()->user().str,
         current_thd->security_context()->user().length);
  pos += current_thd->security_context()->user().length;
  memcpy(pos, current_thd->security_context()->host().str,
         current_thd->security_context()->host().length);
  pos += current_thd->security_context()->host().length;

  pos += user_name_len;

  assert(pos - data < 512);

  // client expects base64 encoded data
  (*outbuf) = new unsigned char[512];
  base64_encode(data, pos - data, (char *)*outbuf, false);

  return false;
}

DEFINE_BOOL_METHOD(finish,
                   (unsigned char *buf, unsigned int buflen,
                    const unsigned char *challenge __attribute__((unused)),
                    unsigned int challenge_length __attribute__((unused)),
                    unsigned char *challenge_response,
                    unsigned int *challenge_response_length)) {
  // we receive the following data in buf from the client:
  // * length + fido authenticator data
  // * length + signature
  // * length + certificate
  // * length + rpid
  //
  // For authentication to work, we need the signature, the public key
  // part of the X509 certificate and the credential id.
  // Cred ID can be reconstructed by FIDO, and we can also extract the
  // public key from the certificate with it.

  // FIRST: reconstruct cred id using fido
  fido_cred_t *c = fido_cred_new();
  fido_pubkey_t pubkey;
  if (reconstruct_fido_cred(c, (char *)buf, buflen, pubkey)) {
    return true;
  }

  // SECOND: decode challenge (base64) which is the first 2 + username
  unsigned char data[512];

  // first 64 bytes will strore the public key
  memcpy(data, pubkey.data(), 64);

  unsigned char *pos = data + 64;

  // Then the credential id in the format we'll use to
  // send to the client
  auto c_len = fido_cred_id_len(c);
  pos = net_store_length(pos, c_len);
  memcpy(pos, fido_cred_id_ptr(c), c_len);
  pos += c_len;
  fido_cred_free(&c);

  // Base64 encode everything so we can store it in a varchar
  base64_encode(data, pos - data, (char *)challenge_response, false);
  *challenge_response_length = strlen((char *)challenge_response);

  return false;
}

void get_challange_length(unsigned int *outbuflen) {
  // Length is always 200 + username + base64 encoding overhead
  // 512 bytes should be enough to store that
  *outbuflen = 512;
}
};  // namespace authentication_fido_reg

BEGIN_SERVICE_IMPLEMENTATION(authentication_fido,
                             mysql_authentication_registration)
authentication_fido_reg::init, authentication_fido_reg::finish,
    authentication_fido_reg::get_challange_length, END_SERVICE_IMPLEMENTATION();

static int pfido_init(MYSQL_PLUGIN plugin_info __attribute__((unused))) {
  fido_init(0);  // add FIDO_DEBUG for debugging

  // Register the registration service
  SERVICE_TYPE(registry) *plugin_registry = mysql_plugin_registry_acquire();
  my_service<SERVICE_TYPE(registry_registration)> reg("registry_registration",
                                                      plugin_registry);
  using mysql_authentication_registration_t =
      SERVICE_TYPE_NO_CONST(mysql_authentication_registration);

  bool result = reg->register_service(
      "mysql_authentication_registration.authentication_fido",
      reinterpret_cast<my_h_service>(
          const_cast<mysql_authentication_registration_t *>(
              &SERVICE_IMPLEMENTATION(authentication_fido,
                                      mysql_authentication_registration))));

  mysql_plugin_registry_release(plugin_registry);
  return result;
}

static int fido_deinit(MYSQL_PLUGIN plugin_info __attribute__((unused))) {
  // Unregister the registration service
  SERVICE_TYPE(registry) *plugin_registry = mysql_plugin_registry_acquire();
  my_service<SERVICE_TYPE(registry_registration)> reg("registry_registration",
                                                      plugin_registry);
  bool result =
      reg->unregister("mysql_authentication_registration.authentication_fido");

  mysql_plugin_registry_release(plugin_registry);
  return result;
}

bool reconstruct_fido_cred(fido_cred_t *cred, const char *data,
                           std::size_t data_len, fido_pubkey_t &pubkey) {
  // set fixed parameters
  if (fido_cred_set_type(cred, COSE_ES256) != FIDO_OK) {
    return true;
  }
  // clientdata hash: skip, we don't need it
  if (fido_cred_set_rp(cred, rpid, nullptr) != FIDO_OK) {
    return true;
  }
  // authdata: in data, set later
  if (fido_cred_set_rk(cred, FIDO_OPT_FALSE) != FIDO_OK) {
    return true;
  }
  if (fido_cred_set_uv(cred, FIDO_OPT_FALSE) != FIDO_OK) {
    return true;
  }
  // x509: in data, later
  // sig: in data,later
  if (fido_cred_set_fmt(cred, "packed") != FIDO_OK) {
    return true;
  }

  std::vector<unsigned char> decoded(data_len);
  // * length + fido authenticator data
  // * length + signature
  // * length + certificate
  // * length + rpid
  base64_decode(data, data_len, decoded.data(), nullptr, 0);

  unsigned char *to = decoded.data();
  unsigned long len = net_field_length_ll(&to);
  if (len != 0) {
    auto ret = fido_cred_set_authdata(cred, to, len);
    if (ret != FIDO_OK) {
      return true;
    }
  }
  to += len;

  len = net_field_length_ll(&to);
  if (len != 0) {
    if (fido_cred_set_sig(cred, to, len) != FIDO_OK) {
      return true;
    }
  }
  to += len;

  len = net_field_length_ll(&to);
  if (len != 0) {
    if (fido_cred_set_x509(cred, to, len) != FIDO_OK) {
      return true;
    }
  }

  auto data2 = fido_cred_pubkey_ptr(cred);
  memcpy(pubkey.data(), static_cast<const void *>(data2),
         fido_cred_pubkey_len(cred));
  to += len;

  // Last is RP in data, ignore, that's fixed
  return false;
}

int fido_authenticate(MYSQL_PLUGIN_VIO *vio, MYSQL_SERVER_AUTH_INFO *info) {
  unsigned char *response;

  if (info == nullptr) {
    return CR_ERROR;  // PS-8454
  }

  if (info->multi_factor_auth_info &&
      info->multi_factor_auth_info[info->current_auth_factor]
          .is_registration_required) {
    // send empty challange, that's what the client expects
    if (vio->write_packet(
            vio,
            static_cast<const unsigned char *>(static_cast<const void *>("")),
            0)) {
      return CR_ERROR;
    }
    // allow login to proceed to finish registration
    return CR_OK_AUTH_IN_SANDBOX_MODE;
  }

  // Challange:
  // 1. scramble_length (8 byte)
  // 2. scramble
  // 3. RPID length
  // 4. RPID
  // 5. CRED ID length
  // 6. CRED ID

  std::vector<unsigned char> decoded(512);
  auto decoded_len = base64_decode(info->auth_string, info->auth_string_length,
                                   decoded.data(), nullptr, 0);

  static auto const data_offset = 64;

  if (decoded_len < data_offset + 1) {
    return CR_ERROR;  // PS-8454
  }
  unsigned char data[512];
  unsigned char *pos = data;

  // add random scramble
  pos = net_store_length(pos, 32);
  unsigned char scramble[32];
  RAND_bytes(scramble, 32);
  memcpy(pos, scramble, 32);
  pos += 32;

  // relying party ID
  pos = net_store_length(pos, strlen(rpid));
  memcpy(pos, rpid, strlen(rpid));
  pos += strlen(rpid);

  // remaining is copied from auth_string
  // first 64 bytes is the public key the data we'll need is after that
  memcpy(pos, decoded.data() + data_offset, decoded_len - data_offset);
  pos += decoded_len - data_offset;

  if (vio->write_packet(
          vio,
          static_cast<const unsigned char *>(static_cast<const void *>(data)),
          pos - data)) {
    return CR_ERROR;
  }

  // First 64 bytes of the decoded auth_string are the public key
  // in FIDO internal format
  const es256_pk_t *es256_pk = reinterpret_cast<es256_pk_t *>(decoded.data());

  // Handle the response
  // len + authenticator data
  // len + signature
  // we also need the fixed rpid, and the random scramble we sent earlier

  if ((vio->read_packet(vio, &response)) < 0) {
    return CR_ERROR;
  }

  unsigned char *from = response;
  std::size_t adata_len = net_field_length_ll(&from);
  fido_assert_t *assert = fido_assert_new();

  auto r = fido_assert_set_rp(assert, rpid);
  if (r != FIDO_OK) {
    fido_assert_free(&assert);
    return CR_ERROR;
  }

  r = fido_assert_set_clientdata_hash(assert, (const unsigned char *)scramble,
                                      32);
  if (r != FIDO_OK) {
    fido_assert_free(&assert);
    return CR_ERROR;
  }

  r = fido_assert_set_count(assert, 1);
  if (r != FIDO_OK) {
    fido_assert_free(&assert);
    return CR_ERROR;
  }

  r = fido_assert_set_authdata(assert, 0, from, adata_len);
  if (r != FIDO_OK) {
    fido_assert_free(&assert);
    return CR_ERROR;
  }
  from += adata_len;

  std::size_t signature_len = net_field_length_ll(&from);
  r = fido_assert_set_sig(assert, 0, from, signature_len);
  if (r != FIDO_OK) {
    fido_assert_free(&assert);
    return CR_ERROR;
  }

  r = fido_assert_verify(assert, 0, COSE_ES256, es256_pk);
  fido_assert_free(&assert);

  if (r != FIDO_OK) {
    return CR_ERROR;
  }

  return CR_OK;
}

int fido_generate_auth_string_hash(char *outbuf, unsigned int *buflen,
                                   const char *inbuf, unsigned int inbuflen) {
  /*
    fail if buffer specified by server cannot be copied to output buffer
  */
  if (*buflen < inbuflen) return 1; /* error */
  strncpy(outbuf, inbuf, inbuflen);
  *buflen = strnlen(inbuf, inbuflen);
  return 0; /* success */
}

int fido_validate_auth_string_hash(char *const, unsigned int) {
  return 0; /* success */
}

int fido_set_salt(const char *password __attribute__((unused)),
                  unsigned int password_len __attribute__((unused)),
                  unsigned char *salt __attribute__((unused)),
                  unsigned char *salt_len) {
  *salt_len = 0;
  return 0; /* success */
}

// Plugin declaration
struct st_mysql_auth plugin_fido_handler = {
    MYSQL_AUTHENTICATION_INTERFACE_VERSION,  // int interface_version
    "authentication_fido_client",            // const char *client_auth_plugin
    &fido_authenticate,                      // authentication function
    &fido_generate_auth_string_hash,         // generate_authentication_string,
    &fido_validate_auth_string_hash,         // validate_authentication_string,
    &fido_set_salt,                          // set_salt,
    AUTH_FLAG_REQUIRES_REGISTRATION,         // authentication_flags
    nullptr};

mysql_declare_plugin(authentication_fido) {
  MYSQL_AUTHENTICATION_PLUGIN,      /* plugin type */
      &plugin_fido_handler,         /* type-specific descriptor */
      "authentication_fido",        /* plugin name */
      "Percona",                    /* author */
      "FIDO authentication plugin", /* description */
      PLUGIN_LICENSE_GPL,           /* license type */
      &pfido_init,                  /* init function */
      &fido_deinit,                 /* deinit function */
      nullptr,                      /* no check function */
      0x0100,                       /* version = 1.0 */
      nullptr,                      /* no status variables */
      fido_sysvars,                 /* system variables */
      nullptr                       /* no reserved information */
#if MYSQL_PLUGIN_INTERFACE_VERSION >= 0x103
      ,
      0 /* no flags */
#endif
}
mysql_declare_plugin_end;
