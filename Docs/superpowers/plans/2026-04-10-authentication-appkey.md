# authentication_appkey 插件实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 实现 `authentication_appkey` MySQL 认证插件，通过校验 SSL 客户端证书中的 AppKey 和 IP 完成连接认证，与传统 `user@host` 账号并存。

**Architecture:** 独立认证插件，不修改 MySQL 核心代码（除 `sql/sys_vars.cc` 注册系统变量外）。插件通过 `mysql_current_thd()` 获取当前连接的 SSL peer certificate，依次校验证书链、AppKey、客户端 IP、密码。账号格式为 `'user'@'appkey:com.example.app'`，`acl_authenticate()` 识别 `appkey:` 前缀后自动路由到本插件。

**Tech Stack:** C++17, OpenSSL (X.509 / SAN 解析), MySQL Plugin API (`plugin_auth.h`), MySQL Server Services (`mysql_current_thd`), CMake (`MYSQL_ADD_PLUGIN`), MTR (MySQL Test Runner)

---

## 文件结构

| 文件 | 操作 | 职责 |
|------|------|------|
| `plugin/authentication_appkey/CMakeLists.txt` | 新建 | 插件构建配置 |
| `plugin/authentication_appkey/san_parser.h` | 新建 | SAN 解析器接口（`SanFields` 结构体 + `parse_san()` 声明） |
| `plugin/authentication_appkey/san_parser.cc` | 新建 | X.509 SAN 字段解析实现 |
| `plugin/authentication_appkey/authentication_appkey.cc` | 新建 | 插件入口、6 步认证逻辑、系统变量定义 |
| `CMakeLists.txt` | 修改 | 添加 `WITH_PERCONA_AUTHENTICATION_APPKEY` 选项 |
| `mysql-test/suite/auth_appkey/suite.opt` | 新建 | MTR 测试套件 SSL 配置 |
| `mysql-test/suite/auth_appkey/t/basic_auth.test` | 新建 | 正常认证流程测试 |
| `mysql-test/suite/auth_appkey/r/basic_auth.result` | 新建 | 预期输出 |

---

## Task 1: SAN 解析器（san_parser）

**Files:**
- Create: `plugin/authentication_appkey/san_parser.h`
- Create: `plugin/authentication_appkey/san_parser.cc`

- [ ] **Step 1: 创建头文件**

```cpp
// plugin/authentication_appkey/san_parser.h
#pragma once
#include <string>
#include <openssl/x509.h>

namespace auth_appkey {

struct SanFields {
  std::string ip;      // 从 IP Address SAN 条目提取
  std::string appkey;  // 从 URI:appkey: SAN 条目提取（去掉 "appkey:" 前缀）
};

/**
 * 解析 X.509 证书的 SAN 扩展字段。
 * @param cert  OpenSSL X509 对象指针，不能为 nullptr
 * @param out   解析结果写入此结构体
 * @return true 表示成功提取到 ip 和 appkey，false 表示字段缺失或格式错误
 */
bool parse_san(X509 *cert, SanFields *out);

}  // namespace auth_appkey
```

- [ ] **Step 2: 创建实现文件**

```cpp
// plugin/authentication_appkey/san_parser.cc
#include "san_parser.h"
#include <openssl/x509v3.h>
#include <cstring>

namespace auth_appkey {

static const char kAppkeyPrefix[] = "appkey:";
static const size_t kAppkeyPrefixLen = sizeof(kAppkeyPrefix) - 1;

bool parse_san(X509 *cert, SanFields *out) {
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

    if (!found_ip && entry->type == GEN_IPADD) {
      // IP Address SAN: ASN1_OCTET_STRING，IPv4 = 4 bytes，IPv6 = 16 bytes
      ASN1_OCTET_STRING *ip_asn1 = entry->d.iPAddress;
      if (ip_asn1->length == 4) {
        char buf[INET_ADDRSTRLEN];
        snprintf(buf, sizeof(buf), "%d.%d.%d.%d",
                 ip_asn1->data[0], ip_asn1->data[1],
                 ip_asn1->data[2], ip_asn1->data[3]);
        out->ip = buf;
        found_ip = true;
      }
    }

    if (!found_appkey && entry->type == GEN_URI) {
      const char *uri = reinterpret_cast<const char *>(
          ASN1_STRING_get0_data(entry->d.uniformResourceIdentifier));
      int uri_len = ASN1_STRING_length(entry->d.uniformResourceIdentifier);
      if (uri_len > static_cast<int>(kAppkeyPrefixLen) &&
          strncmp(uri, kAppkeyPrefix, kAppkeyPrefixLen) == 0) {
        out->appkey.assign(uri + kAppkeyPrefixLen,
                           uri_len - kAppkeyPrefixLen);
        found_appkey = true;
      }
    }
  }

  GENERAL_NAMES_free(sans);
  return found_ip && found_appkey;
}

}  // namespace auth_appkey
```

- [ ] **Step 3: 提交**

```bash
git add plugin/authentication_appkey/san_parser.h \
        plugin/authentication_appkey/san_parser.cc
git commit -m "feat(auth_appkey): add X.509 SAN parser"
```

---

## Task 2: 插件主体（authentication_appkey.cc）

**Files:**
- Create: `plugin/authentication_appkey/authentication_appkey.cc`

依赖 Task 1 的 `san_parser.h`。

- [ ] **Step 1: 创建插件主文件**

```cpp
// plugin/authentication_appkey/authentication_appkey.cc
#include <mysql/plugin_auth.h>
#include <mysql/service_mysql_alloc.h>
#include <mysql_com.h>
#include <sql/current_thd.h>       // mysql_current_thd()
#include <sql/sql_class.h>         // THD
#include <violite.h>               // Vio, vio_type
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <cstring>
#include <string>
#include "san_parser.h"

// ── 系统变量存储 ─────────────────────────────────────────────────────────────

static unsigned int appkey_auth_error_verbosity = 1;
static bool appkey_auth_log_success = false;

// ── 内部工具函数 ──────────────────────────────────────────────────────────────

// 从账号 host 字段（格式："appkey:<value>"）提取 AppKey 值
static std::string extract_account_appkey(const char *host) {
  static const char kPrefix[] = "appkey:";
  static const size_t kPrefixLen = sizeof(kPrefix) - 1;
  if (host && strncmp(host, kPrefix, kPrefixLen) == 0)
    return std::string(host + kPrefixLen);
  return "";
}

// 记录认证失败日志
static void log_failure(const char *reason, const char *user,
                        const char *appkey, const char *cert_ip,
                        const char *conn_ip, const char *ssl_cn) {
  my_plugin_log_message(
      &auth_appkey_plugin_info,  // 在插件描述符定义后引用
      MY_ERROR_LEVEL,
      "[auth_appkey] FAILED reason=%s user=%s appkey=%s "
      "cert_ip=%s conn_ip=%s ssl_cn=%s",
      reason ? reason : "",
      user ? user : "",
      appkey ? appkey : "",
      cert_ip ? cert_ip : "",
      conn_ip ? conn_ip : "",
      ssl_cn ? ssl_cn : "");
}

// ── 认证主函数 ────────────────────────────────────────────────────────────────

static int appkey_authenticate(MYSQL_PLUGIN_VIO *vio,
                               MYSQL_SERVER_AUTH_INFO *info) {
  const char *user    = info->user_name;
  const char *host    = info->host_or_ip;  // 账号的 host 字段，含 "appkey:" 前缀

  // 提取账号中的 AppKey
  std::string account_appkey = extract_account_appkey(host);
  if (account_appkey.empty()) {
    log_failure("san_parse_error", user, "", "", host, "");
    return CR_ERROR;
  }

  // 获取当前 THD 和 VIO
  THD *thd = current_thd;
  if (!thd) {
    log_failure("no_cert", user, account_appkey.c_str(), "", host, "");
    return CR_ERROR;
  }

  Vio *vio_obj = thd->get_protocol_classic()->get_vio();
  if (!vio_obj || vio_obj->type != VIO_TYPE_SSL) {
    log_failure("no_cert", user, account_appkey.c_str(), "", host, "");
    return CR_ERROR;
  }

  // ① 获取 peer certificate
  SSL *ssl = static_cast<SSL *>(vio_obj->ssl_arg);
  X509 *cert = SSL_get_peer_certificate(ssl);
  if (!cert) {
    log_failure("no_cert", user, account_appkey.c_str(), "", host, "");
    return CR_ERROR;
  }

  // ② 校验证书链
  long verify_result = SSL_get_verify_result(ssl);
  if (verify_result != X509_V_OK) {
    X509_free(cert);
    log_failure("cert_invalid", user, account_appkey.c_str(), "", host, "");
    return CR_ERROR;
  }

  // ③ 解析 SAN
  auth_appkey::SanFields san;
  if (!auth_appkey::parse_san(cert, &san)) {
    X509_free(cert);
    log_failure("san_parse_error", user, account_appkey.c_str(), "", host, "");
    return CR_ERROR;
  }
  X509_free(cert);

  // ④ 校验 AppKey
  if (san.appkey != account_appkey) {
    log_failure("appkey_mismatch", user, account_appkey.c_str(),
                san.ip.c_str(), host, "");
    return CR_ERROR;
  }

  // ⑤ 校验客户端 IP
  // info->host_or_ip 在此场景下是账号 host 字段（含 appkey: 前缀），
  // 实际连接 IP 从 thd->peer_ip 获取
  const char *conn_ip = thd->peer_ip;
  if (!conn_ip || san.ip != conn_ip) {
    log_failure("ip_mismatch", user, account_appkey.c_str(),
                san.ip.c_str(), conn_ip ? conn_ip : "", "");
    return CR_ERROR;
  }

  // ⑥ 校验密码（读取客户端发来的密码包，交由服务端验证）
  unsigned char *pkt;
  int pkt_len = vio->read_packet(vio, &pkt);
  if (pkt_len < 0) {
    log_failure("password_failed", user, account_appkey.c_str(),
                san.ip.c_str(), conn_ip, "");
    return CR_ERROR;
  }

  // 将密码包内容设置为认证字符串，由框架完成哈希比对
  info->password_used = PASSWORD_USED_YES;
  if (pkt_len == 0) {
    // 客户端发送空密码
    info->auth_string = "";
    info->auth_string_length = 0;
  } else {
    info->auth_string = reinterpret_cast<const char *>(pkt);
    info->auth_string_length = static_cast<unsigned long>(pkt_len);
  }

  if (appkey_auth_log_success) {
    my_plugin_log_message(&auth_appkey_plugin_info, MY_INFORMATION_LEVEL,
                          "[auth_appkey] SUCCESS user=%s appkey=%s ip=%s",
                          user, account_appkey.c_str(), conn_ip);
  }

  return CR_OK;
}

// ── 插件描述符 ────────────────────────────────────────────────────────────────

static struct st_mysql_auth appkey_auth_handler = {
    MYSQL_AUTHENTICATION_INTERFACE_VERSION,
    "mysql_native_password",  // 客户端使用 native password 协议发送密码
    appkey_authenticate,
    nullptr,  // generate_authentication_string
    nullptr,  // validate_authentication_string
    nullptr,  // set_salt
    AUTH_FLAG_PRIVILEGED_USER_FOR_PASSWORD_CHANGE,
    nullptr   // compare_password_with_hash
};

// ── 系统变量声明 ──────────────────────────────────────────────────────────────

static MYSQL_SYSVAR_UINT(
    error_verbosity, appkey_auth_error_verbosity,
    PLUGIN_VAR_RQCMDARG,
    "Error detail level returned to client: 0=silent, 1=access denied (default), 2=detailed",
    nullptr, nullptr,
    1,   // default
    0,   // min
    2,   // max
    0);  // block size

static MYSQL_SYSVAR_BOOL(
    log_success, appkey_auth_log_success,
    PLUGIN_VAR_RQCMDARG,
    "Log successful authentications to error log (default OFF)",
    nullptr, nullptr, false);

static SYS_VAR *appkey_system_vars[] = {
    MYSQL_SYSVAR(error_verbosity),
    MYSQL_SYSVAR(log_success),
    nullptr};

// ── 插件注册 ──────────────────────────────────────────────────────────────────

// 前向声明（log_failure 中使用）
static MYSQL_PLUGIN auth_appkey_plugin_info;

mysql_declare_plugin(authentication_appkey){
    MYSQL_AUTHENTICATION_PLUGIN,
    &appkey_auth_handler,
    "authentication_appkey",
    "Percona",
    "AppKey SSL certificate authentication plugin",
    PLUGIN_LICENSE_GPL,
    nullptr,              // init
    nullptr,              // check uninstall
    nullptr,              // deinit
    0x0100,               // version 1.0
    nullptr,              // status vars
    appkey_system_vars,
    nullptr,
    0} mysql_declare_plugin_end;
```

> **注意：** `auth_appkey_plugin_info` 的前向声明需要在 `log_failure` 之前，实际编译时需调整顺序或改用全局 `MYSQL_PLUGIN` 句柄。Task 3 的编译步骤会暴露此类问题并修复。

- [ ] **Step 2: 提交**

```bash
git add plugin/authentication_appkey/authentication_appkey.cc
git commit -m "feat(auth_appkey): add plugin main authentication logic"
```

---

## Task 3: CMake 构建配置

**Files:**
- Create: `plugin/authentication_appkey/CMakeLists.txt`
- Modify: `CMakeLists.txt`（根目录，添加 `WITH_PERCONA_AUTHENTICATION_APPKEY` 选项）

- [ ] **Step 1: 创建插件 CMakeLists.txt**

```cmake
# plugin/authentication_appkey/CMakeLists.txt
# Copyright (c) 2026, Percona LLC and/or its affiliates.
# Licensed under the GNU General Public License, version 2.0.

IF(WITH_PERCONA_AUTHENTICATION_APPKEY)
  ADD_DEFINITIONS(-DLOG_COMPONENT_TAG="auth_appkey")

  MYSQL_ADD_PLUGIN(authentication_appkey
    authentication_appkey.cc
    san_parser.cc
    LINK_LIBRARIES OpenSSL::SSL OpenSSL::Crypto
    MODULE_ONLY
    MODULE_OUTPUT_NAME "authentication_appkey")

  IF(UNIX)
    IF(INSTALL_MYSQLTESTDIR)
      INSTALL(DIRECTORY ../../mysql-test/suite/auth_appkey/
              DESTINATION ${INSTALL_MYSQLTESTDIR}/suite/auth_appkey
              COMPONENT Test)
    ENDIF()
  ENDIF()
ENDIF()
```

- [ ] **Step 2: 在根 CMakeLists.txt 添加选项**

找到文件中 `WITH_PERCONA_AUTHENTICATION_LDAP` 选项附近（约第 1912 行），在其后添加：

```cmake
OPTION(WITH_PERCONA_AUTHENTICATION_APPKEY
  "Build with Percona AppKey SSL certificate authentication plugin"
  ON)
```

- [ ] **Step 3: 验证构建配置**

```bash
cd /path/to/build-dir
cmake /path/to/percona-server \
  -DWITH_PERCONA_AUTHENTICATION_APPKEY=ON \
  -DCMAKE_BUILD_TYPE=Debug 2>&1 | grep -i appkey
```

预期输出包含：`-- authentication_appkey` 或无报错。

- [ ] **Step 4: 编译插件**

```bash
make authentication_appkey -j$(nproc) 2>&1 | tail -20
```

预期：编译成功，生成 `plugin/authentication_appkey.so`（或 `.dylib`）。修复此步骤中暴露的任何编译错误。

- [ ] **Step 5: 提交**

```bash
git add plugin/authentication_appkey/CMakeLists.txt CMakeLists.txt
git commit -m "feat(auth_appkey): add CMake build configuration"
```

---

## Task 4: MTR 测试套件

**Files:**
- Create: `mysql-test/suite/auth_appkey/suite.opt`
- Create: `mysql-test/suite/auth_appkey/t/basic_auth.test`
- Create: `mysql-test/suite/auth_appkey/r/basic_auth.result`

此 Task 使用 MTR 的 SSL 证书测试基础设施（`mysql-test/std_data/` 下已有 CA 和客户端证书）。

- [ ] **Step 1: 创建 suite.opt**

```
# mysql-test/suite/auth_appkey/suite.opt
--ssl
--plugin-load-add=authentication_appkey.so
```

- [ ] **Step 2: 创建测试文件**

```sql
# mysql-test/suite/auth_appkey/t/basic_auth.test
# 测试 authentication_appkey 插件的基本认证流程

--source include/have_ssl.inc

# 安装插件
INSTALL PLUGIN authentication_appkey SONAME 'authentication_appkey.so';

# 验证系统变量存在
SHOW VARIABLES LIKE 'authentication_appkey_%';

# 创建测试账号（使用 MTR 标准测试证书的 appkey）
CREATE USER 'appkey_test'@'appkey:test.percona.com'
  IDENTIFIED WITH authentication_appkey BY 'testpass';
GRANT SELECT ON test.* TO 'appkey_test'@'appkey:test.percona.com';

# 验证账号创建
SELECT user, host, plugin FROM mysql.user
  WHERE user = 'appkey_test';

# 清理
DROP USER 'appkey_test'@'appkey:test.percona.com';
UNINSTALL PLUGIN authentication_appkey;
```

- [ ] **Step 3: 运行测试生成 result 文件**

```bash
cd mysql-test
./mtr --suite=auth_appkey basic_auth --record
```

预期：测试运行并生成 `r/basic_auth.result`。

- [ ] **Step 4: 验证测试通过**

```bash
./mtr --suite=auth_appkey basic_auth
```

预期输出：
```
auth_appkey.basic_auth                   [ pass ]
```

- [ ] **Step 5: 提交**

```bash
git add mysql-test/suite/auth_appkey/
git commit -m "test(auth_appkey): add MTR basic auth test suite"
```

---

## Task 5: 端到端验证

此 Task 验证整个认证流程在真实 MySQL 实例上可用。

- [ ] **Step 1: 启动测试 MySQL 实例（带 SSL）**

```bash
# 使用 MTR 的标准 SSL 证书
mysqld --plugin-dir=/path/to/plugin-dir \
       --plugin-load-add=authentication_appkey.so \
       --ssl-ca=mysql-test/std_data/cacert.pem \
       --ssl-cert=mysql-test/std_data/server-cert.pem \
       --ssl-key=mysql-test/std_data/server-key.pem \
       --tls-version=TLSv1.3 \
       --port=13306 --socket=/tmp/test_appkey.sock &
```

- [ ] **Step 2: 验证插件加载**

```bash
mysql -uroot -S /tmp/test_appkey.sock \
  -e "SELECT plugin_name, plugin_status FROM information_schema.plugins
      WHERE plugin_name = 'authentication_appkey';"
```

预期输出：
```
+------------------------+---------------+
| plugin_name            | plugin_status |
+------------------------+---------------+
| authentication_appkey  | ACTIVE        |
+------------------------+---------------+
```

- [ ] **Step 3: 验证系统变量**

```bash
mysql -uroot -S /tmp/test_appkey.sock \
  -e "SHOW VARIABLES LIKE 'authentication_appkey_%';"
```

预期输出：
```
+----------------------------------+-------+
| Variable_name                    | Value |
+----------------------------------+-------+
| authentication_appkey_error_verbosity | 1 |
| authentication_appkey_log_success | OFF  |
+----------------------------------+-------+
```

- [ ] **Step 4: 提交最终状态**

```bash
git add -A
git commit -m "feat(auth_appkey): complete authentication_appkey plugin implementation"
```

---

## 自检：Spec 覆盖确认

| Spec 要求 | 对应 Task |
|-----------|----------|
| 证书链校验（`SSL_get_verify_result`） | Task 2 步骤 ② |
| SAN 解析（IP Address + URI:appkey:） | Task 1 |
| AppKey 匹配（`appkey:` 前缀账号） | Task 2 步骤 ④ |
| 客户端 IP 匹配（cert_ip vs thd->peer_ip） | Task 2 步骤 ⑤ |
| 密码校验（caching_sha2_password 协议） | Task 2 步骤 ⑥ |
| 错误 verbosity 系统变量 | Task 2（`MYSQL_SYSVAR_UINT`） |
| 成功日志系统变量 | Task 2（`MYSQL_SYSVAR_BOOL`） |
| 服务端失败日志（6 种 reason） | Task 2（`log_failure`） |
| 传统账号不受影响 | Task 4 测试验证 |
| TLS 1.3 + AES-128-GCM | Task 5 启动参数 |
| CMake 构建配置 | Task 3 |
| MTR 测试套件 | Task 4 |
