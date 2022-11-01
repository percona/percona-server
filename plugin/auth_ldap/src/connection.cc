/* Copyright (c) 2019 Francisco Miguel Biete Banon. All rights reserved.
   Copyright (c) 2022, Percona Inc. All Rights Reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   51 Franklin Street, Suite 500, Boston, MA 02110-1335 USA */
#include "plugin/auth_ldap/include/connection.h"

#include <iostream>
#include <iterator>
#include <regex>

#include "plugin/auth_ldap/include/plugin_log.h"

namespace {
// example of this callback is in the OpenLDAP's
// servers/slapd/back-meta/bind.c (meta_back_default_urllist)
int cb_urllist_proc(LDAP * /* ld */, LDAPURLDesc **urllist, LDAPURLDesc **url,
                    void * /* params */) {
  if (urllist == url) return LDAP_SUCCESS;

  LDAPURLDesc **urltail;
  for (urltail = &(*url)->lud_next; *urltail; urltail = &(*urltail)->lud_next)
    /* count */;

  // all failed hosts go to the end of list
  *urltail = *urllist;
  // succeeded host becomes first
  *urllist = *url;
  // mark end of list
  *url = nullptr;

  return LDAP_SUCCESS;
}

void cb_log(LDAP_CONST char *data) { log_ldap_dbg(data); }

}  // namespace

namespace mysql {
namespace plugin {
namespace auth_ldap {

void Connection::initialize_global_ldap_parameters(bool enable_debug,
                                                   std::string const &ca_path) {
  int version = LDAP_VERSION3;
  int err = ldap_set_option(nullptr, LDAP_OPT_PROTOCOL_VERSION, &version);
  if (err != LDAP_OPT_SUCCESS) {
    log_error("ldap_set_option(LDAP_OPT_PROTOCOL_VERSION)", err);
  }

  if (ca_path.size() == 0) {
    int reqCert = LDAP_OPT_X_TLS_NEVER;
    err = ldap_set_option(nullptr, LDAP_OPT_X_TLS_REQUIRE_CERT, &reqCert);
    if (err != LDAP_OPT_SUCCESS) {
      log_error("ldap_set_option(LDAP_OPT_X_TLS_REQUIRE_CERT)", err);
    }
  } else {
    char *cca_path = const_cast<char *>(ca_path.c_str());
    err = ldap_set_option(nullptr, LDAP_OPT_X_TLS_CACERTFILE,
                          static_cast<void *>(cca_path));
    if (err != LDAP_OPT_SUCCESS) {
      log_error("ldap_set_option(LDAP_OPT_X_TLS_CACERTFILE)", err);
    }
  }

  err = ldap_set_option(nullptr, LDAP_OPT_X_TLS_NEWCTX, LDAP_OPT_ON);
  if (err != LDAP_OPT_SUCCESS) {
    log_error("ldap_set_option(LDAP_OPT_X_TLS_NEWCTX)", err);
  }

  if (enable_debug) {
    static const unsigned short debug_any = 0xffff;
    err = ldap_set_option(nullptr, LDAP_OPT_DEBUG_LEVEL, &debug_any);
    if (err != LDAP_OPT_SUCCESS) {
      log_error("ldap_set_option(LDAP_OPT_DEBUG_LEVEL)", err);
    }
    ber_set_option(nullptr, LBER_OPT_LOG_PRINT_FN,
                   reinterpret_cast<const void *>(cb_log));
  }
}

Connection::Connection(std::size_t idx, const std::string &ldap_host,
                       std::uint16_t ldap_port,
                       const std::string &fallback_host,
                       std::uint16_t fallback_port, bool use_ssl, bool use_tls)
    : available_(true),
      index_(idx),
      snipped_(false),
      ldap_host_(ldap_host),
      ldap_port_(ldap_port),
      ldap_fallback_host_(fallback_host),
      ldap_fallback_port_(fallback_port),
      use_ssl_(use_ssl),
      use_tls_(use_tls),
      ldap_(nullptr) {}

Connection::~Connection() {
  if (ldap_) {
    ldap_unbind_ext_s(ldap_, nullptr, nullptr);
    ldap_ = nullptr;
  }
}

void Connection::configure(const std::string &ldap_host,
                           std::uint16_t ldap_port,
                           const std::string &fallback_host,
                           std::uint16_t fallback_port, bool use_ssl,
                           bool use_tls) {
  // ldap function use c_strs from these variables
  // changing them during a connect call could lead to a crash
  std::lock_guard<std::mutex> lock(conn_mutex_);
  ldap_host_ = ldap_host;
  ldap_port_ = ldap_port;
  ldap_fallback_host_ = fallback_host;
  ldap_fallback_port_ = fallback_port;
  use_ssl_ = use_ssl;
  use_tls_ = use_tls;
}

Connection::status Connection::connect(const std::string &bind_dn,
                                       const std::string &bind_auth,
                                       std::string &auth_resp,
                                       const std::string &sasl_mech) {
  std::lock_guard<std::mutex> lock(conn_mutex_);

  int version = LDAP_VERSION3;
  ldap_set_option(nullptr, LDAP_OPT_PROTOCOL_VERSION, &version);

  if (bind_auth.empty() && sasl_mech == "") {
    log_srv_error("Empty passwords are disabled with simple auth");
    return status::FAILURE;
  }

  if (!(ldap_host_.empty() || bind_dn.empty())) {
    log_srv_dbg("Connecting to ldap server as " + bind_dn);

    if (ldap_) {
      ldap_unbind_ext_s(ldap_, nullptr, nullptr);
    }

    int err = ldap_initialize(&(ldap_), get_ldap_uri().c_str());
    if (err != LDAP_SUCCESS) {
      log_error("ldap_initialize", err);
      return status::FAILURE;
    }

    // Optional; log warning if the server doesn't support it, but continue
    err = ldap_set_option(ldap_, LDAP_OPT_REFERRALS, LDAP_OPT_OFF);
    if (err != LDAP_OPT_SUCCESS) {
      log_warning("ldap_set_option(LDAP_OPT_REFERRALS, LDAP_OPT_OFF)", err);
    }

    // Optional; log warning if the server doesn't support it, but continue
    err = ldap_set_option(ldap_, LDAP_OPT_RESTART, LDAP_OPT_ON);
    if (err != LDAP_OPT_SUCCESS) {
      log_warning("ldap_set_option(LDAP_OPT_RESTART, LDAP_OPT_ON)", err);
    }

    if (use_tls_) {
      err = ldap_start_tls_s(ldap_, nullptr, nullptr);
      if (err != LDAP_SUCCESS) {
        log_error("ldap_start_tls_s", err);
        return status::FAILURE;
      }
    }

    err = ldap_set_urllist_proc(ldap_, cb_urllist_proc, nullptr);
    if (err != LDAP_OPT_SUCCESS) {
      log_warning("ldap_set_urllist_proc failed", err);
    }

    return connect_step(bind_dn, bind_auth, auth_resp, sasl_mech);
  }

  return status::FAILURE;
}

Connection::status Connection::connect_step(const std::string &bind_dn,
                                            const std::string &bind_auth,
                                            std::string &auth_resp,
                                            const std::string &sasl_mech) {
  struct berval *serverCreds;
  struct berval *userCreds =
      ber_str2bv(strdup(bind_auth.c_str()), 0, 0, nullptr);

  int err = ldap_sasl_bind_s(ldap_, bind_dn.c_str(),
                             sasl_mech.empty() ? nullptr : sasl_mech.c_str(),
                             userCreds, nullptr, nullptr, &serverCreds);

  ber_bvfree(userCreds);
  if (serverCreds && serverCreds->bv_len != 0) {
    auth_resp = std::string(serverCreds->bv_val,
                            serverCreds->bv_val + serverCreds->bv_len);
  }
  ber_bvfree(serverCreds);

  if (err == LDAP_SASL_BIND_IN_PROGRESS) {
    log_srv_dbg("SASL bind in progress: ldap_sasl_bind_s(" + bind_dn + ")");
    return status::IN_PROGRESS;
  }
  if (err != LDAP_SUCCESS) {
    log_warning("Unsuccesful bind: ldap_sasl_bind_s(" + bind_dn + ")", err);
    return status::FAILURE;
  }

  return status::SUCCESS;
}

std::size_t Connection::get_idx_pool() const { return index_; }

bool Connection::is_snipped() const { return snipped_; }

bool Connection::is_zombie() {
  std::lock_guard<std::mutex> lock(conn_mutex_);
  return available_ ? false
                    : (std::time(nullptr) - borrowed_ts_) >
                          Connection::ZombieAfterSeconds;
}

void Connection::mark_as_busy() {
  std::lock_guard<std::mutex> lock(conn_mutex_);
  available_ = false;
  borrowed_ts_ = std::time(nullptr);
}

void Connection::mark_as_free() {
  std::lock_guard<std::mutex> lock(conn_mutex_);
  available_ = true;
}

void Connection::mark_as_snipped() { snipped_ = true; }

std::string Connection::search_dn(const std::string &user_name,
                                  const std::string &user_search_attr,
                                  const std::string &base_dn) {
  std::lock_guard<std::mutex> lock(conn_mutex_);

  std::string str;
  std::ostringstream log_stream;
  std::string filter = user_search_attr + "=" + user_name;

  log_stream << "search_dn(" << base_dn << ", " << filter << ")";
  log_srv_dbg(log_stream.str());
  log_stream.str("");

  LDAPMessage *l_result;
  char *attrs[] = {const_cast<char *>("dn"), nullptr};
  struct timeval search_timeout = {5, 0};
  const int searchlimit = 1;
  int err = ldap_search_ext_s(
      ldap_, base_dn.c_str() /* base */, LDAP_SCOPE_SUBTREE /*scope*/,
      filter.c_str() /*filter*/, attrs /*attrs*/, 0 /*attrsonly*/,
      nullptr /*serverctrls*/, nullptr /*clientctrls*/,
      &search_timeout /*timeout*/, searchlimit /*searchlimit*/,
      &l_result /*ldapmessage*/);
  if (err == LDAP_SUCCESS) {
    // Verify an entry was found
    if (ldap_count_entries(ldap_, l_result) == 0) {
      log_stream << "ldap_search_ext_s(" << base_dn << ", " << filter
                 << ") returned no matching entries";
      log_srv_warn(log_stream.str());
      log_stream.str("");
    } else {
      // entry and dn are pointers to l_result components; don't cleanup them or
      // this segfault when cleaning up l_result
      LDAPMessage *entry = ldap_first_entry(ldap_, l_result);
      char *dn = ldap_get_dn(ldap_, entry);
      log_stream << "ldap_search_ext_s(" << base_dn << ", " << filter
                 << "): " << dn;
      log_srv_dbg(log_stream.str());
      log_stream.str("");
      str = dn;
    }
    ldap_msgfree(l_result);
    l_result = nullptr;
  } else {
    log_stream << "ldap_search_ext_s(" << base_dn << ", " << filter << ") "
               << ldap_err2string(err);
    log_srv_error(log_stream.str());
    log_stream.str("");
  }

  log_stream << "search_dn(" << base_dn << ", " << filter << ") = " << str;
  log_srv_dbg(log_stream.str());
  log_stream.str("");

  return str;
}

groups_t Connection::search_groups(const std::string &user_name,
                                   const std::string &user_dn,
                                   const std::string &group_search_attr,
                                   const std::string &group_search_filter,
                                   const std::string &base_dn) {
  std::lock_guard<std::mutex> lock(conn_mutex_);

  groups_t list;
  std::stringstream log_stream;
  std::string filter = std::regex_replace(group_search_filter,
                                          std::regex("\\{UA\\}"), user_name);
  filter = std::regex_replace(filter, std::regex("\\{UD\\}"), user_dn);

  LDAPMessage *l_result;
  char *attrs[] = {const_cast<char *>(group_search_attr.c_str()), nullptr};
  struct timeval search_timeout = {5, 0};
  int err = ldap_search_ext_s(
      ldap_, base_dn.c_str() /* base */, LDAP_SCOPE_SUBTREE /*scope*/,
      filter.c_str() /*filter*/, attrs /*attrs*/, 0 /*attrsonly*/,
      nullptr /*serverctrls*/, nullptr /*clientctrls*/,
      &search_timeout /*timeout*/, 0 /*searchlimit*/,
      &l_result /*ldapmessage*/);
  if (err == LDAP_SUCCESS) {
    // Verify an entry was found
    if (ldap_count_entries(ldap_, l_result) == 0) {
      log_stream << "ldap_search_ext_s(" << base_dn << ", " << filter
                 << ") returned no matching entries";
      log_srv_warn(log_stream.str());
      log_stream.str("");
    } else {
      char *attribute;
      BerElement *ber;
      BerValue **vals;
      LDAPMessage *entry = ldap_first_entry(ldap_, l_result);
      while (entry) {
        // Don't cleanup entry, attribute, ber or vals because they will be
        // cleaned up with l_result
        attribute = ldap_first_attribute(ldap_, entry, &ber);
        while (attribute) {
          vals = ldap_get_values_len(ldap_, entry, attribute);
          for (int pos = 0; pos < ldap_count_values_len(vals); pos++) {
            list.push_back(std::string(vals[pos]->bv_val));
          }
          attribute = ldap_next_attribute(ldap_, entry, ber);
        }
        entry = ldap_next_entry(ldap_, entry);
      }
    }
    ldap_msgfree(l_result);
    l_result = nullptr;
  } else {
    log_stream << "ldap_search_ext_s('" << base_dn << "', '" << filter << "') "
               << ldap_err2string(err);
    log_srv_error(log_stream.str());
    log_stream.str("");
  }

  log_stream << "search_groups() = ";
  std::copy(list.begin(), list.end(),
            std::ostream_iterator<std::string>(log_stream, ","));
  log_srv_dbg(log_stream.str());
  log_stream.str("");

  return list;
}

std::string Connection::get_ldap_uri() {
  std::ostringstream str_stream;
  str_stream << (use_ssl_ ? "ldaps://" : "ldap://") << ldap_host_ << ":"
             << ldap_port_;
  if (!ldap_fallback_host_.empty()) {
    // We allow 2 formats for fallback:
    // * only fallback_host is specified (port is 0): in this case, we add it to
    // the connection string as is
    // * port is also specified: in this case we add the protocal prefix and
    // port to it
    str_stream << ",";
    if (ldap_fallback_port_ != 0) {
      str_stream << (use_ssl_ ? "ldaps://" : "ldap://");
    }
    str_stream << ldap_fallback_host_;
    if (ldap_fallback_port_ != 0) {
      str_stream << ":" << ldap_fallback_port_;
    }
  }
  return str_stream.str();
}

void Connection::log_error(const std::string &str, int ldap_err) {
  std::stringstream log_stream;
  log_stream << str << " " << ldap_err2string(ldap_err);
  log_srv_error(log_stream.str());
}
void Connection::log_warning(const std::string &str, int ldap_err) {
  std::stringstream log_stream;
  log_stream << str << " " << ldap_err2string(ldap_err);
  log_srv_warn(log_stream.str());
}

}  // namespace auth_ldap
}  // namespace plugin
}  // namespace mysql
