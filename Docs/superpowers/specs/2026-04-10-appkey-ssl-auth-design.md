# AppKey SSL 证书认证插件设计文档

**日期：** 2026-04-10
**状态：** 草稿
**分支：** feature/pr-20260406-mysql-user

---

## 背景

MySQL 传统的 `user@host` 授权方式需要在创建账号时指定客户端 IP 或 DNS。在实际运维中，客户端 IP 会频繁变更，账号密码也存在泄露风险，带来了较大的管理成本。

本设计为 Percona Server 8.0 引入一个基于证书的认证插件（`authentication_appkey`）。客户端 SSL 证书的 SAN 字段中携带 AppKey（服务标识）和客户端 IP，插件通过校验这些信息完成认证。该插件与传统 `user@host` 认证并存——host 字段带有 `appkey:` 前缀的账号走证书认证，其余账号继续使用现有流程，完全不受影响。

---

## 目标

- 通过 SSL 客户端证书（含 AppKey 和客户端 IP）完成 MySQL 连接认证
- 与传统 `user@host` 账号并存，不修改现有认证路径
- 以独立插件形式实现，不修改 MySQL 核心认证逻辑
- 支持可配置的错误详细程度，区分生产环境和开发环境
- 强制使用 TLS 1.3 + AES-128-GCM，将数据传输阶段的性能损耗降至最低

---

## 非目标

- 替换或废弃传统 `user@host` 认证方式
- 认证完成后将 SSL 降级为明文 TCP（受 MySQL VIO 架构限制，技术上不可行）
- 证书的签发与管理（由现有企业 PKI 负责）
- 修改 `mysql.user` 表结构

---

## 证书格式

客户端证书由现有企业 PKI 签发，AppKey 和客户端 IP 存储在 X.509v3 Subject Alternative Name（SAN）扩展字段中：

```
X509v3 Subject Alternative Name:
    DNS:set-hh-admin-bdmp-worker-dev02.mt
    IP Address:10.216.129.76
    URI:appkey:com.sankuai.admin.bdmp.worker
    URI:env:dev
```

插件从 SAN 中提取：
- `IP Address` 条目 → `cert_ip`（必须与实际 TCP 连接的对端 IP 一致）
- `URI:appkey:` 条目 → `cert_appkey`（必须与账号的 AppKey 一致）

---

## 账号格式

AppKey 账号使用标准 `CREATE USER` 语法，`host` 字段存储带 `appkey:` 前缀的 AppKey：

```sql
-- 创建 AppKey 账号
CREATE USER 'bdmp_worker'@'appkey:com.sankuai.admin.bdmp.worker'
  IDENTIFIED WITH authentication_appkey BY 'password123';

-- 授权
GRANT SELECT, INSERT, UPDATE ON bdmp.*
  TO 'bdmp_worker'@'appkey:com.sankuai.admin.bdmp.worker';
```

`appkey:` 前缀是 `acl_authenticate()` 将连接路由到本插件的识别信号。不需要修改 `mysql.user` 表结构——host 列直接存储完整的 `appkey:<value>` 字符串。`mysqldump`、`SHOW GRANTS`、`SHOW CREATE USER` 等现有工具无需任何改动。

传统账号完全不受影响：

```sql
-- 传统账号，行为不变
CREATE USER 'admin'@'10.216.129.76' IDENTIFIED BY 'pass';
```

---

## 整体架构

```
客户端（SSL + 证书）
        │
        ▼
  SSL 握手（现有 OpenSSL 层）
        │
        ▼
  acl_authenticate()
        │
        ├─ host 无 appkey: 前缀 ──→ 现有认证流程（不变）
        │
        └─ host 有 appkey: 前缀
                 │
                 ▼
        authentication_appkey 插件
                 │
                 ├─ ① 获取 peer certificate（SSL_get_peer_certificate）
                 ├─ ② 校验证书链（SSL_get_verify_result）
                 ├─ ③ 解析 SAN，提取 cert_ip 和 cert_appkey
                 ├─ ④ 校验 cert_appkey == 账号 AppKey
                 ├─ ⑤ 校验 cert_ip == thd->peer_ip
                 └─ ⑥ 校验密码（caching_sha2_password 协议）
```

插件通过 `mysql_current_thd()`（官方 server service）→ `thd->get_protocol()->get_vio()` → `SSL_get_peer_certificate()` 获取 SSL peer certificate，无需修改任何 MySQL 核心代码。

---

## 认证流程

1. **客户端发起连接**，携带 SSL 客户端证书。
2. **SSL 握手完成**（OpenSSL/VIO 层，现有行为）。
3. **`acl_authenticate()` 查找账号**。若 host 字段以 `appkey:` 开头，则将连接分发给 `authentication_appkey` 插件处理。
4. **插件步骤 ①：获取 peer certificate。**
   调用 `mysql_current_thd()` → `get_vio()` → `SSL_get_peer_certificate()`。
   若客户端未提供证书 → 拒绝（`no_cert`）。
5. **插件步骤 ②：校验证书链合法性。**
   调用 `SSL_get_verify_result()`，必须返回 `X509_V_OK`。
   若证书不合法 → 拒绝（`cert_invalid`）。
6. **插件步骤 ③：解析 SAN 字段。**
   调用 `X509_get_ext_d2i(cert, NID_subject_alt_name)`。
   提取 `IP Address` → `cert_ip`，`URI:appkey:` → `cert_appkey`。
   若 SAN 缺失或格式错误 → 拒绝（`san_parse_error`）。
7. **插件步骤 ④：校验 AppKey。**
   从账号 host 字段去掉 `appkey:` 前缀得到 `account_appkey`。
   比较 `cert_appkey == account_appkey`。
   若不匹配 → 拒绝（`appkey_mismatch`）。
8. **插件步骤 ⑤：校验客户端 IP。**
   比较 `cert_ip == thd->peer_ip`（实际 TCP 连接的对端 IP）。
   若不匹配 → 拒绝（`ip_mismatch`）。
9. **插件步骤 ⑥：校验密码。**
   执行 `caching_sha2_password` 挑战-响应协议。
   若密码错误 → 拒绝（`password_failed`）。
10. **认证成功**，连接在 TLS 通道上正常进行。

---

## 错误处理

### 客户端错误响应

由系统变量 `appkey_auth_error_verbosity` 控制（默认值：`1`）：

| 值 | 行为 | 推荐环境 |
|----|------|---------|
| `0` | 静默断开连接，不返回任何错误 | 生产环境（最高安全级别） |
| `1` | 返回 `ER_ACCESS_DENIED_ERROR`，不说明具体原因 | 生产环境（默认） |
| `2` | 返回详细错误信息，明确指出失败原因 | 开发 / 测试环境 |

### 服务端日志

无论 verbosity 级别如何，所有认证失败都会写入 MySQL error log：

```
[auth_appkey] FAILED reason=<reason> user=<user> appkey=<appkey>
              cert_ip=<cert_ip> conn_ip=<conn_ip> ssl_cn=<cert_cn>
```

失败原因说明：

| 原因码 | 说明 |
|--------|------|
| `no_cert` | 客户端未提供证书 |
| `cert_invalid` | 证书链校验失败（证书过期、CA 不可信） |
| `san_parse_error` | SAN 扩展字段缺失或无法解析 |
| `appkey_mismatch` | 证书中的 AppKey 与账号 AppKey 不一致 |
| `ip_mismatch` | 证书中的 IP 与实际连接 IP 不一致 |
| `password_failed` | 密码挑战-响应校验失败 |

认证成功的日志记录由 `appkey_auth_log_success` 控制（默认：`OFF`），避免高 QPS 场景下日志量过大。

---

## TLS 性能

插件强制使用 TLS 1.3 并限制加密套件，将数据传输阶段的性能损耗降至最低：

- **要求 TLS 版本：** TLS 1.3
- **要求加密套件：** `TLS_AES_128_GCM_SHA256`
- AES-128-GCM 在具备 AES-NI 硬件加速的 CPU 上，吞吐量损耗 < 1%
- TLS 1.3 Session Ticket 支持会话复用，降低连接池场景下的重复握手开销

服务端配置：

```ini
[mysqld]
tls_version          = TLSv1.3
tls_ciphersuites     = TLS_AES_128_GCM_SHA256
ssl_ca               = /etc/mysql/ca.crt
ssl_cert             = /etc/mysql/server.crt
ssl_key              = /etc/mysql/server.key
```

若客户端未使用 TLS 1.3 或使用了不在许可列表中的加密套件，插件在步骤 ② 以 `cert_invalid` 拒绝连接。

---

## 客户端连接示例

```bash
mysql -h 127.0.0.1 -u bdmp_worker -p \
  --ssl-cert=/etc/certs/client.crt \
  --ssl-key=/etc/certs/client.key \
  --ssl-ca=/etc/certs/ca.crt \
  --tls-version=TLSv1.3
```

`appkey:` 账号的客户端若未携带证书，插件在步骤 ① 即拒绝连接（`no_cert`），不会降级为仅密码认证。

---

## 关键文件

| 文件 | 用途 |
|------|------|
| `plugin/authentication_appkey/authentication_appkey.cc` | 插件入口，认证主逻辑 |
| `plugin/authentication_appkey/san_parser.cc` | X.509 SAN 字段解析 |
| `plugin/authentication_appkey/san_parser.h` | SAN 解析器接口 |
| `plugin/authentication_appkey/CMakeLists.txt` | 构建配置 |
| `mysql-test/suite/auth_appkey/` | MTR 测试套件 |

---

## 系统变量

| 变量名 | 类型 | 默认值 | 说明 |
|--------|------|--------|------|
| `appkey_auth_error_verbosity` | `INT`（0–2） | `1` | 返回给客户端的错误详细程度 |
| `appkey_auth_log_success` | `BOOL` | `OFF` | 是否记录认证成功的日志 |

---

## 兼容性

- Percona Server 8.0
- OpenSSL 1.1.x 和 3.x（通过现有 `vio/viossl.cc` 抽象层支持）
- 现有 `user@host` 账号：完全不受影响
- `mysqldump`、`SHOW GRANTS`、`SHOW CREATE USER`：无需修改，host 列原样存储 `appkey:` 字符串
