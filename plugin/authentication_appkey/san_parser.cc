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

#include "san_parser.h"
#include <openssl/x509v3.h>
#include <cstring>
#include <cstdio>

namespace auth_appkey {

static const char kAppkeyPrefix[] = "appkey:";
static const size_t kAppkeyPrefixLen = sizeof(kAppkeyPrefix) - 1;
// Buffer size for IPv4 dotted-decimal notation: "255.255.255.255\0" = 16 bytes
static const int kIPv4AddrStrLen = 16;

bool parse_san(const X509 *cert, SanFields *out) {
  out->ip.clear();
  out->appkey.clear();

  GENERAL_NAMES *sans = static_cast<GENERAL_NAMES *>(
      X509_get_ext_d2i(cert, NID_subject_alt_name, nullptr, nullptr));
  if (!sans) return false;

  bool found_ip = false;
  bool found_appkey = false;

  int count = sk_GENERAL_NAME_num(sans);
  for (int i = 0; i < count; ++i) {
    GENERAL_NAME *entry = sk_GENERAL_NAME_value(sans, i);
    if (!entry) continue;  // 防御性检查：跳过损坏的 SAN 条目

    if (!found_ip && entry->type == GEN_IPADD) {
      ASN1_OCTET_STRING *ip_asn1 = entry->d.iPAddress;
      if (!ip_asn1 || !ip_asn1->data) continue;  // 防御性检查
      // 仅支持 IPv4（4 字节）；IPv6（16 字节）暂不支持，静默跳过
      if (ip_asn1->length == 4) {
        char buf[kIPv4AddrStrLen];
        snprintf(buf, sizeof(buf), "%u.%u.%u.%u",
                 (unsigned)ip_asn1->data[0], (unsigned)ip_asn1->data[1],
                 (unsigned)ip_asn1->data[2], (unsigned)ip_asn1->data[3]);
        out->ip = buf;
        found_ip = true;
      }
    }

    if (!found_appkey && entry->type == GEN_URI) {
      ASN1_IA5STRING *uri_asn1 = entry->d.uniformResourceIdentifier;
      if (!uri_asn1 || !uri_asn1->data) continue;  // 防御性检查
      const char *uri = reinterpret_cast<const char *>(
          ASN1_STRING_get0_data(uri_asn1));
      int uri_len = ASN1_STRING_length(uri_asn1);
      if (uri_len > static_cast<int>(kAppkeyPrefixLen) &&
          strncmp(uri, kAppkeyPrefix, kAppkeyPrefixLen) == 0) {
        size_t value_len = uri_len - kAppkeyPrefixLen;
        const char *value_start = uri + kAppkeyPrefixLen;
        // 拒绝含嵌入 NUL 字节的 appkey（防止认证绕过攻击）
        if (strnlen(value_start, value_len) != value_len) continue;
        out->appkey.assign(value_start, value_len);
        found_appkey = true;
      }
    }
  }

  GENERAL_NAMES_free(sans);
  return found_ip && found_appkey;
}

}  // namespace auth_appkey
