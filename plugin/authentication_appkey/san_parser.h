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

#pragma once
#include <string>
#include <openssl/x509.h>

namespace auth_appkey {

struct SanFields {
  std::string ip;      // 从 IP Address SAN 条目提取（仅 IPv4）
  std::string appkey;  // 从 URI:appkey: SAN 条目提取（去掉 "appkey:" 前缀）
};

/**
 * 解析 X.509 证书的 SAN 扩展字段。
 * 注意：仅支持 IPv4 地址，IPv6 地址会被忽略。
 * @param cert  OpenSSL X509 对象指针，不能为 nullptr
 * @param out   解析结果写入此结构体
 * @return true 表示成功提取到 ip 和 appkey，false 表示字段缺失或格式错误
 */
bool parse_san(const X509 *cert, SanFields *out);

}  // namespace auth_appkey
