/* Copyright (c) 2026, Percona LLC and/or its affiliates.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/**
  @file plugin/authentication_appkey/authentication_appkey.cc

  Percona authentication_appkey plugin.

  Authenticates a connection by verifying:
    1. The client presents a valid peer TLS certificate.
    2. The certificate chain is trusted (SSL_get_verify_result == X509_V_OK).
    3. The certificate's SAN contains both an IP Address and a URI of the form
       "appkey:<value>".
    4. The appkey value in the SAN matches the account host field (which must
       start with the "appkey:" prefix, e.g. CREATE USER 'app'@'appkey:myapp').
    5. The IP Address in the SAN matches the client's actual connection IP.
    6. The client sends a password packet that is accepted as credential proof.
*/

#include "plugin/authentication_appkey/san_parser.h"

#include <openssl/ssl.h>
#include <openssl/x509.h>

#include <cstring>
#include <string>

#include "mysql/plugin.h"
#include "mysql/plugin_auth.h"
#include "mysql/plugin_auth_common.h"
#include "mysql/service_my_plugin_log.h"
#include "sql/auth/sql_authentication.h"  /* MPVIO_EXT, acl_user */
#include "sql/auth/sql_auth_cache.h"      /* ACL_USER */
#include "sql/current_thd.h"              /* current_thd */
#include "sql/protocol_classic.h"         /* Protocol_classic */
#include "sql/sql_class.h"                /* THD */
#include "violite.h"                      /* Vio */

/* -------------------------------------------------------------------------
 * Global plugin handle (saved in init callback for safe logging)
 * ---------------------------------------------------------------------- */

static MYSQL_PLUGIN appkey_plugin_info = nullptr;

/* -------------------------------------------------------------------------
 * System variables
 * ---------------------------------------------------------------------- */

static unsigned int appkey_auth_error_verbosity = 1;
static bool appkey_auth_log_success = false;

static MYSQL_SYSVAR_UINT(error_verbosity, appkey_auth_error_verbosity,
                         PLUGIN_VAR_RQCMDARG,
                         "Error detail level: 0=silent, 1=access denied "
                         "(default), 2=detailed",
                         nullptr, nullptr, 1, 0, 2, 0);

static MYSQL_SYSVAR_BOOL(
    log_success, appkey_auth_log_success, PLUGIN_VAR_RQCMDARG,
    "Log successful authentications (default OFF)", nullptr, nullptr, false);

static SYS_VAR *appkey_sysvars[] = {MYSQL_SYSVAR(error_verbosity),
                                    MYSQL_SYSVAR(log_success), nullptr};

/* -------------------------------------------------------------------------
 * Logging helpers
 * ---------------------------------------------------------------------- */

static const char kAppkeyHostPrefix[] = "appkey:";
static const size_t kAppkeyHostPrefixLen = sizeof(kAppkeyHostPrefix) - 1;

/**
  Log an authentication failure using my_plugin_log_message.

  @param reason      Short identifier for the failure cause.
  @param user        MySQL user name attempted.
  @param appkey      AppKey value extracted from SAN (may be empty).
  @param cert_ip     IP from the client certificate's SAN (may be empty).
  @param conn_ip     Actual connection IP.
*/
static void log_failure(const char *reason, const char *user,
                        const char *appkey, const char *cert_ip,
                        const char *conn_ip) {
  if (appkey_auth_error_verbosity == 0) return;

  my_plugin_log_message(&appkey_plugin_info, MY_ERROR_LEVEL,
                        "[auth_appkey] FAILED reason=%s user=%s appkey=%s "
                        "cert_ip=%s conn_ip=%s",
                        reason ? reason : "", user ? user : "",
                        appkey ? appkey : "", cert_ip ? cert_ip : "",
                        conn_ip ? conn_ip : "");
}

/**
  Log a successful authentication.

  @param user    MySQL user name.
  @param appkey  AppKey value.
  @param ip      Connection IP.
*/
static void log_success(const char *user, const char *appkey, const char *ip) {
  if (!appkey_auth_log_success) return;

  my_plugin_log_message(&appkey_plugin_info, MY_INFORMATION_LEVEL,
                        "[auth_appkey] SUCCESS user=%s appkey=%s ip=%s",
                        user ? user : "", appkey ? appkey : "", ip ? ip : "");
}

/* -------------------------------------------------------------------------
 * Core authentication function
 * ---------------------------------------------------------------------- */

static int appkey_authenticate(MYSQL_PLUGIN_VIO *vio,
                               MYSQL_SERVER_AUTH_INFO *info) {
  /* ------------------------------------------------------------------ *
   * Gather context information up front.                                *
   * ------------------------------------------------------------------ */
  const char *user = info->user_name ? info->user_name : "";

  /* The MYSQL_PLUGIN_VIO passed here is always the embedded MPVIO_EXT.
     This cast is the same pattern used by sha2_password.cc and other
     built-in authentication plugins. */
  MPVIO_EXT *mpvio = reinterpret_cast<MPVIO_EXT *>(vio);

  /* Use mpvio->ip (the raw connection IP) rather than info->host_or_ip,
     which may contain the resolved hostname when reverse DNS is available.
     info->host_or_ip is retained only for diagnostic logging. */
  const char *conn_ip = mpvio->ip ? mpvio->ip : "";

  /* Determine the expected appkey.
     Primary source: info->auth_string (stored via IDENTIFIED BY 'appkey:...')
     Fallback source: acl_user->host.get_host() for the user@appkey:... format
     which requires the find_mpvio_user() fix in sql_authentication.cc. */
  const char *auth_str = (info->auth_string && info->auth_string_length > 0)
                             ? info->auth_string
                             : "";
  const char *account_host = "";
  if (strncmp(auth_str, kAppkeyHostPrefix, kAppkeyHostPrefixLen) == 0) {
    /* Auth string stores the appkey directly (e.g. "appkey:com.example.app") */
    account_host = auth_str;
  } else if (mpvio->acl_user && mpvio->acl_user->host.get_host()) {
    /* Fall back to host field for user@appkey:... account format */
    account_host = mpvio->acl_user->host.get_host();
  }

  /* ------------------------------------------------------------------ *
   * Step 1: Obtain the peer certificate.                                *
   * ------------------------------------------------------------------ */
  THD *thd = current_thd;
  if (!thd) {
    log_failure("no_thd", user, "", "", conn_ip);
    return CR_ERROR;
  }

  Protocol_classic *proto = thd->get_protocol_classic();
  if (!proto) {
    log_failure("no_proto", user, "", "", conn_ip);
    return CR_ERROR;
  }

  Vio *net_vio = proto->get_vio();
  if (!net_vio) {
    log_failure("no_vio", user, "", "", conn_ip);
    return CR_ERROR;
  }

  /* Log diagnostic info */
  if (appkey_auth_error_verbosity >= 2) {
    my_plugin_log_message(&appkey_plugin_info, MY_INFORMATION_LEVEL,
                          "[auth_appkey] DEBUG thd=%p proto=%p vio=%p "
                          "vio_type=%d ssl_arg=%p vio_is_enc=%d",
                          (void *)thd, (void *)proto, (void *)net_vio,
                          (int)net_vio->type, net_vio->ssl_arg,
                          mpvio->vio_is_encrypted);
  }

  SSL *ssl = reinterpret_cast<SSL *>(net_vio->ssl_arg);
  if (!ssl) {
    log_failure("no_ssl", user, "", "", conn_ip);
    return CR_ERROR;
  }

  X509 *cert = SSL_get_peer_certificate(ssl);
  if (!cert) {
    log_failure("no_cert", user, "", "", conn_ip);
    return CR_ERROR;
  }

  /* ------------------------------------------------------------------ *
   * Step 2: Verify the certificate chain.                               *
   * ------------------------------------------------------------------ */
  if (SSL_get_verify_result(ssl) != X509_V_OK) {
    X509_free(cert);
    log_failure("cert_invalid", user, "", "", conn_ip);
    return CR_ERROR;
  }

  /* ------------------------------------------------------------------ *
   * Step 3: Parse the Subject Alternative Name extension.               *
   * ------------------------------------------------------------------ */
  auth_appkey::SanFields san;
  if (!auth_appkey::parse_san(cert, &san)) {
    X509_free(cert);
    log_failure("san_parse_error", user, "", "", conn_ip);
    return CR_ERROR;
  }
  X509_free(cert);
  cert = nullptr;

  /* ------------------------------------------------------------------ *
   * Step 4: Verify AppKey.                                              *
   *                                                                     *
   * The account was created with a host of the form "appkey:<value>",   *
   * e.g.:  CREATE USER 'app'@'appkey:com.example.app' ...              *
   * acl_user->host.get_host() returns that raw host string.             *
   * We strip the "appkey:" prefix and compare against the value in the  *
   * certificate's SAN URI.                                              *
   * ------------------------------------------------------------------ */
  const char *expected_appkey = "";
  if (strncmp(account_host, kAppkeyHostPrefix, kAppkeyHostPrefixLen) == 0) {
    expected_appkey = account_host + kAppkeyHostPrefixLen;
  }

  if (*expected_appkey == '\0') {
    log_failure("bad_account_config", user, "", san.ip.c_str(), conn_ip);
    return CR_ERROR;
  }

  if (san.appkey.empty() || strcmp(san.appkey.c_str(), expected_appkey) != 0) {
    log_failure("appkey_mismatch", user, san.appkey.c_str(), san.ip.c_str(),
                conn_ip);
    return CR_ERROR;
  }

  /* ------------------------------------------------------------------ *
   * Step 5: Verify client IP matches certificate SAN IP.               *
   * ------------------------------------------------------------------ */
  if (san.ip.empty() || strcmp(san.ip.c_str(), conn_ip) != 0) {
    log_failure("ip_mismatch", user, san.appkey.c_str(), san.ip.c_str(),
                conn_ip);
    return CR_ERROR;
  }

  /* ------------------------------------------------------------------ *
   * Step 6: Read the password packet from the client.                   *
   * ------------------------------------------------------------------ */
  unsigned char *pkt = nullptr;
  int pkt_len = vio->read_packet(vio, &pkt);
  /* caching_sha2_password sends a single \x00 byte for an empty password.
     A real password produces a 32-byte SHA2 hash. Reject empty passwords. */
  if (pkt_len <= 1) {
    log_failure("password_failed", user, san.appkey.c_str(), san.ip.c_str(),
                conn_ip);
    return CR_ERROR;
  }

  // Step 6: 密码包非空性校验（liveness check）
  // 设计说明：本插件的主要凭据是 SSL 证书（AppKey + IP 双重绑定）。
  // 密码在此仅作为"用户知道密码"的存活性证明，不做服务端哈希比对。
  // 服务端哈希比对需要插件实现 generate_authentication_string/compare_password_with_hash，
  // 这超出了当前版本的设计范围（v1.0）。
  // 如需完整密码哈希校验，后续版本可集成 caching_sha2_password 的校验逻辑。
  info->password_used = PASSWORD_USED_YES;
  info->auth_string = reinterpret_cast<const char *>(pkt);
  info->auth_string_length = static_cast<unsigned long>(pkt_len);

  /* ------------------------------------------------------------------ *
   * All checks passed.                                                   *
   * ------------------------------------------------------------------ */
  log_success(user, san.appkey.c_str(), conn_ip);
  return CR_OK;
}

/* -------------------------------------------------------------------------
 * Password storage callbacks
 *
 * generate_authentication_string: called by CREATE USER ... BY 'password'.
 * We store the password as-is (plaintext) since the primary credential is
 * the SSL certificate; the password is a liveness proof only.
 *
 * compare_password_with_hash: called during authentication to compare the
 * stored hash against the client-supplied password. We check non-empty only.
 * ---------------------------------------------------------------------- */

static int appkey_generate_auth_string(char *outbuf, unsigned int *outbuflen,
                                       const char *inbuf,
                                       unsigned int inbuflen) {
  if (inbuflen == 0 || inbuf == nullptr) {
    /* Reject empty password at CREATE USER time */
    *outbuflen = 0;
    return 1;
  }
  if (inbuflen > *outbuflen) inbuflen = *outbuflen;
  memcpy(outbuf, inbuf, inbuflen);
  *outbuflen = inbuflen;
  return 0;
}

static int appkey_compare_password(const char *hash, unsigned long hash_length,
                                   const char *cleartext,
                                   unsigned long cleartext_length,
                                   int *is_error) {
  *is_error = 0;
  if (hash_length == 0 || cleartext_length == 0) return 1; /* mismatch */
  /* Simple non-empty check: cert is the real credential */
  (void)hash;
  (void)cleartext;
  return 0; /* match */
}

static int appkey_validate_auth_string(char * /*inbuf*/,
                                       unsigned int /*buflen*/) {
  /* Any stored auth string is valid for this plugin */
  return 0;
}

static int appkey_set_salt(const char * /*password*/,
                           unsigned int /*password_len*/,
                           unsigned char * /*salt*/,
                           unsigned char *salt_len) {
  /* No salt needed — plaintext storage */
  if (salt_len) *salt_len = 0;
  return 0;
}

/* -------------------------------------------------------------------------
 * Plugin lifecycle callbacks
 * ---------------------------------------------------------------------- */

static int appkey_plugin_init(MYSQL_PLUGIN plugin_info) {
  appkey_plugin_info = plugin_info;
  return 0;
}

static int appkey_plugin_deinit(MYSQL_PLUGIN /*plugin_info*/) { return 0; }

/* -------------------------------------------------------------------------
 * Plugin descriptor
 * ---------------------------------------------------------------------- */

static struct st_mysql_auth appkey_auth_handler = {
    MYSQL_AUTHENTICATION_INTERFACE_VERSION,
    "caching_sha2_password", /* client-side plugin name — matches MySQL 8.0 default */
    appkey_authenticate,
    appkey_generate_auth_string,
    appkey_validate_auth_string,
    appkey_set_salt,
    AUTH_FLAG_PRIVILEGED_USER_FOR_PASSWORD_CHANGE,
    appkey_compare_password
};

mysql_declare_plugin(authentication_appkey){
    MYSQL_AUTHENTICATION_PLUGIN,
    &appkey_auth_handler,
    "authentication_appkey",
    "Percona LLC",
    "AppKey certificate-based authentication plugin",
    PLUGIN_LICENSE_GPL,
    appkey_plugin_init,
    appkey_plugin_deinit,
    nullptr,
    0x0100, /* version 1.0 */
    nullptr,
    appkey_sysvars,
    nullptr
#if MYSQL_PLUGIN_INTERFACE_VERSION >= 0x103
    ,
    0
#endif
} mysql_declare_plugin_end;
