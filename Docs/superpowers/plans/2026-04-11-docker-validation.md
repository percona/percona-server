# Docker Validation for authentication_appkey Plugin

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build the `authentication_appkey` plugin against standard MySQL 8.0 headers in Docker, then run a full end-to-end authentication test (success + 4 failure cases) using docker-compose.

**Architecture:** B2 approach — adapt the plugin to use only the public MySQL Plugin API (no Percona-internal headers). Replace `MPVIO_EXT`/`current_thd` with `getpeername()` for connection IP, and use `info->host_or_ip` for account host. All Docker artifacts live in `docker/` directory. A builder container compiles the `.so`, a server container loads it, and a client container runs validation.

**Tech Stack:** Docker, docker-compose, MySQL 8.0, OpenSSL, CMake, bash (cert generation via `openssl` CLI), C++17

---

## File Structure

| File | Operation | Responsibility |
|------|-----------|----------------|
| `docker/builder/Dockerfile` | Create | Compile plugin against MySQL 8.0 dev headers |
| `docker/builder/CMakeLists.txt` | Create | Standalone CMake for B2 plugin (no Percona build system) |
| `docker/server/Dockerfile` | Create | MySQL 8.0 image with plugin `.so` mounted |
| `docker/server/my.cnf` | Create | MySQL server config: SSL, plugin-load-add |
| `docker/certs/gen-certs.sh` | Create | Generate CA + server cert + client cert with SAN |
| `docker/docker-compose.yml` | Create | Orchestrate builder → server → client |
| `docker/client/validate.sh` | Create | Run success + failure test cases |
| `plugin/authentication_appkey/authentication_appkey_b2.cc` | Create | B2-adapted plugin (no Percona-internal includes) |

---

## Task 1: B2-Adapted Plugin Source

**Files:**
- Create: `plugin/authentication_appkey/authentication_appkey_b2.cc`

The Percona version uses `MPVIO_EXT *mpvio = pointer_cast<MPVIO_EXT *>(vio)` to get the connection IP. The B2 version replaces this with `getpeername()` on the socket file descriptor, which is available via the standard MySQL Plugin VIO API.

- [ ] **Step 1: Create the B2 plugin source file**

```cpp
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
  @file plugin/authentication_appkey/authentication_appkey_b2.cc

  Percona authentication_appkey plugin — B2 build variant.

  Uses only the public MySQL Plugin API (mysql/plugin_auth.h).
  No Percona-internal headers (MPVIO_EXT, current_thd, violite.h).
  Connection IP is obtained via getpeername() on the socket fd
  exposed by MYSQL_PLUGIN_VIO_INFO.
*/

#include <openssl/ssl.h>
#include <openssl/x509.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <cstring>
#include <string>

#include "mysql/plugin.h"
#include "mysql/plugin_auth.h"
#include "mysql/plugin_auth_common.h"
#include "mysql/service_my_plugin_log.h"

#include "san_parser.h"

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
 * Constants
 * ---------------------------------------------------------------------- */

static const char kAppkeyHostPrefix[] = "appkey:";
static const size_t kAppkeyHostPrefixLen = sizeof(kAppkeyHostPrefix) - 1;

/* -------------------------------------------------------------------------
 * Logging helpers
 * ---------------------------------------------------------------------- */

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

static void log_success(const char *user, const char *appkey, const char *ip) {
  if (!appkey_auth_log_success) return;

  my_plugin_log_message(&appkey_plugin_info, MY_INFORMATION_LEVEL,
                        "[auth_appkey] SUCCESS user=%s appkey=%s ip=%s",
                        user ? user : "", appkey ? appkey : "", ip ? ip : "");
}

/* -------------------------------------------------------------------------
 * Get connection IP via getpeername() on the socket fd
 * ---------------------------------------------------------------------- */

/**
  Obtain the client's connection IP address as a dotted-decimal string.

  MYSQL_PLUGIN_VIO_INFO exposes the raw socket fd via the `socket` field
  (available in MySQL 8.0 plugin_auth.h). We call getpeername() to get the
  actual remote address, bypassing any reverse-DNS lookup.

  @param vio     Plugin VIO handle.
  @param out     Output buffer (at least INET_ADDRSTRLEN bytes).
  @param out_len Output buffer length.
  @return true on success, false on failure.
*/
static bool get_conn_ip(MYSQL_PLUGIN_VIO *vio, char *out, size_t out_len) {
  MYSQL_PLUGIN_VIO_INFO vio_info;
  vio->info(vio, &vio_info);

  if (vio_info.protocol != MYSQL_VIO_TCP &&
      vio_info.protocol != MYSQL_VIO_SOCKET) {
    /* Unix socket or named pipe: use "localhost" */
    snprintf(out, out_len, "localhost");
    return true;
  }

  struct sockaddr_storage ss;
  socklen_t ss_len = sizeof(ss);
  if (getpeername(vio_info.socket, reinterpret_cast<struct sockaddr *>(&ss),
                  &ss_len) != 0) {
    return false;
  }

  if (ss.ss_family == AF_INET) {
    struct sockaddr_in *s = reinterpret_cast<struct sockaddr_in *>(&ss);
    if (!inet_ntop(AF_INET, &s->sin_addr, out, static_cast<socklen_t>(out_len))) {
      return false;
    }
    return true;
  }

  if (ss.ss_family == AF_INET6) {
    struct sockaddr_in6 *s = reinterpret_cast<struct sockaddr_in6 *>(&ss);
    if (!inet_ntop(AF_INET6, &s->sin6_addr, out, static_cast<socklen_t>(out_len))) {
      return false;
    }
    return true;
  }

  return false;
}

/* -------------------------------------------------------------------------
 * Get SSL handle from VIO info
 * ---------------------------------------------------------------------- */

/**
  Obtain the OpenSSL SSL* handle from the plugin VIO.

  MYSQL_PLUGIN_VIO_INFO.ssl (available in MySQL 8.0) points to the SSL
  object for TLS connections.
*/
static SSL *get_ssl(MYSQL_PLUGIN_VIO *vio) {
  MYSQL_PLUGIN_VIO_INFO vio_info;
  vio->info(vio, &vio_info);
  return reinterpret_cast<SSL *>(vio_info.ssl);
}

/* -------------------------------------------------------------------------
 * Core authentication function
 * ---------------------------------------------------------------------- */

static int appkey_authenticate(MYSQL_PLUGIN_VIO *vio,
                               MYSQL_SERVER_AUTH_INFO *info) {
  const char *user = info->user_name ? info->user_name : "";

  /* Get connection IP via getpeername() */
  char conn_ip_buf[INET6_ADDRSTRLEN + 1] = {};
  if (!get_conn_ip(vio, conn_ip_buf, sizeof(conn_ip_buf))) {
    log_failure("no_conn_ip", user, "", "", "");
    return CR_ERROR;
  }
  const char *conn_ip = conn_ip_buf;

  /* The account host field is in info->authenticated_as for some versions,
     but info->host_or_ip contains the connecting client's host/IP.
     For appkey accounts the host is stored in the mysql.user table as
     "appkey:<value>". We access it via info->auth_string when the server
     passes the account's authentication string, but the host field is
     available in the plugin only through the authenticated_as field or
     by convention. In MySQL 8.0 the account host is passed as
     info->host_or_ip when it starts with "appkey:" — actually no,
     host_or_ip is the connecting client's host/ip.

     The correct way: the plugin descriptor's requires_password field and
     the account matching already happened. The account's host field value
     is NOT directly available in MYSQL_SERVER_AUTH_INFO in the public API.

     Workaround: MySQL passes info->authenticated_as = the resolved user name
     after matching. The host is not exposed. We use info->user_name and
     rely on the fact that the server only routes to this plugin when the
     account host starts with "appkey:". We must get the host another way.

     Solution: Use info->auth_string which in MySQL 8.0 contains the
     account's authentication string (the password hash). The host is
     passed separately. Looking at mysql/plugin_auth.h in MySQL 8.0:

       const char *host_or_ip  -- client's host name or IP address
       const char *auth_string -- the authentication string from mysql.user

     The account HOST is not in MYSQL_SERVER_AUTH_INFO. We need another
     approach: encode the expected appkey in the authentication string
     (the BY 'value' part of CREATE USER).

     REVISED DESIGN for B2:
     - CREATE USER 'app'@'%' IDENTIFIED WITH authentication_appkey
       BY 'appkey:com.test.app'
     - info->auth_string = "appkey:com.test.app"  (the BY value)
     - We strip "appkey:" prefix from auth_string to get expected_appkey
  */

  /* Extract expected appkey from auth_string (the BY '...' value) */
  const char *auth_str = info->auth_string ? info->auth_string : "";
  const char *expected_appkey = "";
  if (strncmp(auth_str, kAppkeyHostPrefix, kAppkeyHostPrefixLen) == 0) {
    expected_appkey = auth_str + kAppkeyHostPrefixLen;
  }

  if (*expected_appkey == '\0') {
    log_failure("bad_account_config", user, "", "", conn_ip);
    return CR_ERROR;
  }

  /* Step 1: Get SSL handle */
  SSL *ssl = get_ssl(vio);
  if (!ssl) {
    log_failure("no_ssl", user, "", "", conn_ip);
    return CR_ERROR;
  }

  /* Step 2: Get peer certificate */
  X509 *cert = SSL_get_peer_certificate(ssl);
  if (!cert) {
    log_failure("no_cert", user, "", "", conn_ip);
    return CR_ERROR;
  }

  /* Step 3: Verify certificate chain */
  if (SSL_get_verify_result(ssl) != X509_V_OK) {
    X509_free(cert);
    log_failure("cert_invalid", user, "", "", conn_ip);
    return CR_ERROR;
  }

  /* Step 4: Parse SAN */
  auth_appkey::SanFields san;
  if (!auth_appkey::parse_san(cert, &san)) {
    X509_free(cert);
    log_failure("san_parse_error", user, "", "", conn_ip);
    return CR_ERROR;
  }
  X509_free(cert);
  cert = nullptr;

  /* Step 5: Verify AppKey */
  if (san.appkey.empty() || strcmp(san.appkey.c_str(), expected_appkey) != 0) {
    log_failure("appkey_mismatch", user, san.appkey.c_str(), san.ip.c_str(),
                conn_ip);
    return CR_ERROR;
  }

  /* Step 6: Verify client IP matches certificate SAN IP */
  if (san.ip.empty() || strcmp(san.ip.c_str(), conn_ip) != 0) {
    log_failure("ip_mismatch", user, san.appkey.c_str(), san.ip.c_str(),
                conn_ip);
    return CR_ERROR;
  }

  /* Step 7: Read the password packet (liveness check) */
  unsigned char *pkt = nullptr;
  int pkt_len = vio->read_packet(vio, &pkt);
  if (pkt_len <= 0) {
    log_failure("password_failed", user, san.appkey.c_str(), san.ip.c_str(),
                conn_ip);
    return CR_ERROR;
  }

  info->password_used = PASSWORD_USED_YES;
  info->auth_string = reinterpret_cast<const char *>(pkt);
  info->auth_string_length = static_cast<unsigned long>(pkt_len);

  log_success(user, san.appkey.c_str(), conn_ip);
  return CR_OK;
}

/* -------------------------------------------------------------------------
 * Plugin lifecycle
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
    "mysql_native_password",
    appkey_authenticate,
    nullptr, /* generate_authentication_string */
    nullptr, /* validate_authentication_string */
    nullptr, /* set_salt */
    AUTH_FLAG_PRIVILEGED_USER_FOR_PASSWORD_CHANGE,
    nullptr  /* compare_password_with_hash */
};

mysql_declare_plugin(authentication_appkey){
    MYSQL_AUTHENTICATION_PLUGIN,
    &appkey_auth_handler,
    "authentication_appkey",
    "Percona LLC",
    "AppKey certificate-based authentication plugin (B2)",
    PLUGIN_LICENSE_GPL,
    appkey_plugin_init,
    appkey_plugin_deinit,
    nullptr,
    0x0100,
    nullptr,
    appkey_sysvars,
    nullptr
#if MYSQL_PLUGIN_INTERFACE_VERSION >= 0x103
    ,
    0
#endif
} mysql_declare_plugin_end;
```

- [ ] **Step 2: Commit the B2 source**

```bash
git add plugin/authentication_appkey/authentication_appkey_b2.cc
git commit -m "feat(auth_appkey): add B2 variant using public MySQL Plugin API only"
```

---

## Task 2: Certificate Generation Script

**Files:**
- Create: `docker/certs/gen-certs.sh`

The client certificate must have a SAN with `IP Address:127.0.0.1` and `URI:appkey:com.test.app` so the plugin can validate it.

- [ ] **Step 1: Create the cert generation script**

```bash
mkdir -p docker/certs
```

Create `docker/certs/gen-certs.sh`:

```bash
#!/bin/bash
# Generates CA, server cert, and client cert for auth_appkey Docker validation.
# Client cert SAN: IP:127.0.0.1, URI:appkey:com.test.app
# All certs written to the directory containing this script.

set -euo pipefail
DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$DIR"

# ── CA ────────────────────────────────────────────────────────────────────────
openssl genrsa -out ca-key.pem 2048
openssl req -new -x509 -days 3650 -key ca-key.pem -out ca-cert.pem \
  -subj "/CN=TestCA/O=Percona/C=US"

# ── Server cert ───────────────────────────────────────────────────────────────
openssl genrsa -out server-key.pem 2048
openssl req -new -key server-key.pem -out server-csr.pem \
  -subj "/CN=mysql-server/O=Percona/C=US"
cat > server-ext.cnf <<'EOF'
[v3_req]
subjectAltName = DNS:localhost,IP:127.0.0.1
EOF
openssl x509 -req -days 3650 -in server-csr.pem \
  -CA ca-cert.pem -CAkey ca-key.pem -CAcreateserial \
  -out server-cert.pem -extfile server-ext.cnf -extensions v3_req

# ── Client cert (with AppKey SAN) ─────────────────────────────────────────────
openssl genrsa -out client-key.pem 2048
openssl req -new -key client-key.pem -out client-csr.pem \
  -subj "/CN=appkey-client/O=Percona/C=US"
cat > client-ext.cnf <<'EOF'
[v3_req]
subjectAltName = IP:127.0.0.1,URI:appkey:com.test.app
EOF
openssl x509 -req -days 3650 -in client-csr.pem \
  -CA ca-cert.pem -CAkey ca-key.pem -CAcreateserial \
  -out client-cert.pem -extfile client-ext.cnf -extensions v3_req

# ── Client cert with WRONG AppKey (for negative test) ─────────────────────────
cat > client-wrong-appkey-ext.cnf <<'EOF'
[v3_req]
subjectAltName = IP:127.0.0.1,URI:appkey:com.wrong.app
EOF
openssl x509 -req -days 3650 -in client-csr.pem \
  -CA ca-cert.pem -CAkey ca-key.pem -CAcreateserial \
  -out client-wrong-appkey-cert.pem \
  -extfile client-wrong-appkey-ext.cnf -extensions v3_req

# ── Client cert with WRONG IP (for negative test) ─────────────────────────────
cat > client-wrong-ip-ext.cnf <<'EOF'
[v3_req]
subjectAltName = IP:10.0.0.99,URI:appkey:com.test.app
EOF
openssl x509 -req -days 3650 -in client-csr.pem \
  -CA ca-cert.pem -CAkey ca-key.pem -CAcreateserial \
  -out client-wrong-ip-cert.pem \
  -extfile client-wrong-ip-ext.cnf -extensions v3_req

echo "Certificates generated in $DIR"
ls -la *.pem
```

- [ ] **Step 2: Make it executable and run it**

```bash
chmod +x docker/certs/gen-certs.sh
bash docker/certs/gen-certs.sh
```

Expected output: list of `.pem` files including `ca-cert.pem`, `server-cert.pem`, `client-cert.pem`, `client-wrong-appkey-cert.pem`, `client-wrong-ip-cert.pem`.

- [ ] **Step 3: Verify the client cert SAN**

```bash
openssl x509 -in docker/certs/client-cert.pem -noout -text | grep -A5 "Subject Alternative Name"
```

Expected output:
```
X509v3 Subject Alternative Name:
    IP Address:127.0.0.1, URI:appkey:com.test.app
```

- [ ] **Step 4: Commit**

```bash
git add docker/certs/gen-certs.sh docker/certs/*.pem
git commit -m "feat(auth_appkey/docker): add cert generation script and test certificates"
```

---

## Task 3: Builder Container (Compile Plugin)

**Files:**
- Create: `docker/builder/Dockerfile`
- Create: `docker/builder/CMakeLists.txt`

The builder uses the official `mysql/mysql-server:8.0` image which includes MySQL dev headers, or we install them from the MySQL APT repository.

- [ ] **Step 1: Create the standalone CMakeLists.txt for the plugin**

Create `docker/builder/CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.13)
project(authentication_appkey CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# MySQL include path — set by Dockerfile via -DMYSQL_INCLUDE_DIR
if(NOT DEFINED MYSQL_INCLUDE_DIR)
  set(MYSQL_INCLUDE_DIR "/usr/include/mysql")
endif()

find_package(OpenSSL REQUIRED)

add_library(authentication_appkey MODULE
  authentication_appkey_b2.cc
  san_parser.cc
)

target_include_directories(authentication_appkey PRIVATE
  ${MYSQL_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(authentication_appkey PRIVATE
  OpenSSL::SSL
  OpenSSL::Crypto
)

# MySQL plugins must not have a "lib" prefix
set_target_properties(authentication_appkey PROPERTIES
  PREFIX ""
  OUTPUT_NAME "authentication_appkey"
)
```

- [ ] **Step 2: Create the builder Dockerfile**

Create `docker/builder/Dockerfile`:

```dockerfile
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    cmake \
    g++ \
    libssl-dev \
    wget \
    gnupg2 \
    lsb-release \
    && rm -rf /var/lib/apt/lists/*

# Install MySQL 8.0 dev headers from MySQL APT repository
RUN wget -q https://dev.mysql.com/get/mysql-apt-config_0.8.29-1_all.deb \
    && DEBIAN_FRONTEND=noninteractive dpkg -i mysql-apt-config_0.8.29-1_all.deb \
    && apt-get update \
    && apt-get install -y libmysqlclient-dev \
    && rm -rf /var/lib/apt/lists/* mysql-apt-config_0.8.29-1_all.deb

WORKDIR /build

COPY CMakeLists.txt .
COPY authentication_appkey_b2.cc .
COPY san_parser.cc .
COPY san_parser.h .

RUN cmake -DMYSQL_INCLUDE_DIR=/usr/include/mysql \
          -DCMAKE_BUILD_TYPE=RelWithDebInfo . \
    && make -j$(nproc) \
    && echo "Build successful: $(ls -la authentication_appkey.so)"

CMD ["bash"]
```

- [ ] **Step 3: Commit**

```bash
git add docker/builder/Dockerfile docker/builder/CMakeLists.txt
git commit -m "feat(auth_appkey/docker): add builder container for plugin compilation"
```

---

## Task 4: MySQL Server Container

**Files:**
- Create: `docker/server/Dockerfile`
- Create: `docker/server/my.cnf`
- Create: `docker/server/init.sql`

- [ ] **Step 1: Create MySQL server config**

Create `docker/server/my.cnf`:

```ini
[mysqld]
# SSL configuration
ssl_ca   = /etc/mysql/certs/ca-cert.pem
ssl_cert = /etc/mysql/certs/server-cert.pem
ssl_key  = /etc/mysql/certs/server-key.pem

# Require TLS 1.3
tls_version = TLSv1.3

# Load the authentication_appkey plugin
plugin-load-add = authentication_appkey=authentication_appkey.so

# Allow plugin dir to be /usr/lib/mysql/plugin (default)
# Logging
log_error_verbosity = 3

# Required for client certificate verification
require_secure_transport = OFF
```

- [ ] **Step 2: Create init SQL**

Create `docker/server/init.sql`:

```sql
-- Create the appkey test account
-- B2 design: expected appkey is stored in the BY clause (auth_string)
-- Account host is '%' (wildcard) since routing is done by plugin, not by host matching
CREATE USER IF NOT EXISTS 'appkey_user'@'%'
  IDENTIFIED WITH authentication_appkey BY 'appkey:com.test.app';

GRANT SELECT ON mysql.* TO 'appkey_user'@'%';

-- Verify plugin loaded
SELECT plugin_name, plugin_status
FROM information_schema.plugins
WHERE plugin_name = 'authentication_appkey';

-- Verify account created
SELECT user, host, plugin, authentication_string
FROM mysql.user
WHERE user = 'appkey_user';
```

- [ ] **Step 3: Create server Dockerfile**

Create `docker/server/Dockerfile`:

```dockerfile
FROM mysql:8.0

# The plugin .so will be mounted at runtime via docker-compose volume
# Copy server config
COPY my.cnf /etc/mysql/conf.d/appkey.cnf

# Copy init SQL
COPY init.sql /docker-entrypoint-initdb.d/01-appkey.sql
```

- [ ] **Step 4: Commit**

```bash
git add docker/server/Dockerfile docker/server/my.cnf docker/server/init.sql
git commit -m "feat(auth_appkey/docker): add MySQL server container config"
```

---

## Task 5: Client Validation Script

**Files:**
- Create: `docker/client/validate.sh`

This script runs 5 test cases: 1 success + 4 failures.

- [ ] **Step 1: Create the validation script**

Create `docker/client/validate.sh`:

```bash
#!/bin/bash
# End-to-end validation for authentication_appkey plugin.
# Runs inside the client container.

set -euo pipefail

MYSQL_HOST="${MYSQL_HOST:-mysql-server}"
MYSQL_PORT="${MYSQL_PORT:-3306}"
CERT_DIR="${CERT_DIR:-/certs}"
PASS=0
FAIL=0

mysql_connect() {
  local label="$1"
  local expect_success="$2"
  shift 2
  local args=("$@")

  echo -n "  [$label] ... "
  if mysql \
      --host="$MYSQL_HOST" \
      --port="$MYSQL_PORT" \
      --ssl-ca="$CERT_DIR/ca-cert.pem" \
      "${args[@]}" \
      --connect-timeout=5 \
      -e "SELECT 'connected' AS result;" 2>/dev/null | grep -q "connected"; then
    if [ "$expect_success" = "true" ]; then
      echo "PASS"
      PASS=$((PASS + 1))
    else
      echo "FAIL (expected rejection, got success)"
      FAIL=$((FAIL + 1))
    fi
  else
    if [ "$expect_success" = "false" ]; then
      echo "PASS (correctly rejected)"
      PASS=$((PASS + 1))
    else
      echo "FAIL (expected success, got rejection)"
      FAIL=$((FAIL + 1))
    fi
  fi
}

echo ""
echo "=== authentication_appkey Plugin Validation ==="
echo ""

# Wait for MySQL to be ready
echo "Waiting for MySQL server..."
for i in $(seq 1 30); do
  if mysqladmin ping --host="$MYSQL_HOST" --port="$MYSQL_PORT" \
      --ssl-ca="$CERT_DIR/ca-cert.pem" \
      --silent 2>/dev/null; then
    break
  fi
  sleep 2
done
echo "MySQL is ready."
echo ""

echo "--- Test Cases ---"

# TC1: Success — correct cert, correct appkey, correct IP, correct password
mysql_connect "TC1 success" "true" \
  --user=appkey_user \
  --password=anypass \
  --ssl-cert="$CERT_DIR/client-cert.pem" \
  --ssl-key="$CERT_DIR/client-key.pem" \
  --tls-version=TLSv1.3

# TC2: Failure — wrong appkey in cert
mysql_connect "TC2 wrong_appkey" "false" \
  --user=appkey_user \
  --password=anypass \
  --ssl-cert="$CERT_DIR/client-wrong-appkey-cert.pem" \
  --ssl-key="$CERT_DIR/client-key.pem" \
  --tls-version=TLSv1.3

# TC3: Failure — wrong IP in cert
mysql_connect "TC3 wrong_ip" "false" \
  --user=appkey_user \
  --password=anypass \
  --ssl-cert="$CERT_DIR/client-wrong-ip-cert.pem" \
  --ssl-key="$CERT_DIR/client-key.pem" \
  --tls-version=TLSv1.3

# TC4: Failure — no client cert
mysql_connect "TC4 no_cert" "false" \
  --user=appkey_user \
  --password=anypass \
  --tls-version=TLSv1.3

# TC5: Failure — wrong password (empty)
mysql_connect "TC5 empty_password" "false" \
  --user=appkey_user \
  --password="" \
  --ssl-cert="$CERT_DIR/client-cert.pem" \
  --ssl-key="$CERT_DIR/client-key.pem" \
  --tls-version=TLSv1.3

echo ""
echo "=== Results: $PASS passed, $FAIL failed ==="
echo ""

if [ "$FAIL" -gt 0 ]; then
  exit 1
fi
exit 0
```

- [ ] **Step 2: Commit**

```bash
git add docker/client/validate.sh
chmod +x docker/client/validate.sh
git add docker/client/validate.sh
git commit -m "feat(auth_appkey/docker): add client validation script"
```

---

## Task 6: Docker Compose Orchestration

**Files:**
- Create: `docker/docker-compose.yml`
- Create: `docker/run-validation.sh`

- [ ] **Step 1: Create docker-compose.yml**

Create `docker/docker-compose.yml`:

```yaml
version: "3.8"

services:
  # ── Builder: compile the plugin ──────────────────────────────────────────────
  builder:
    build:
      context: ..
      dockerfile: docker/builder/Dockerfile
    volumes:
      - plugin_so:/build/output
    command: >
      bash -c "cp authentication_appkey.so /build/output/ &&
               echo 'Plugin copied to shared volume'"

  # ── MySQL server with plugin ──────────────────────────────────────────────────
  mysql-server:
    build:
      context: .
      dockerfile: server/Dockerfile
    depends_on:
      builder:
        condition: service_completed_successfully
    environment:
      MYSQL_ROOT_PASSWORD: rootpass
      MYSQL_DATABASE: testdb
    volumes:
      - plugin_so:/usr/lib/mysql/plugin/appkey:ro
      - ./certs:/etc/mysql/certs:ro
    ports:
      - "13306:3306"
    healthcheck:
      test: ["CMD", "mysqladmin", "ping", "-h", "localhost",
             "--ssl-ca=/etc/mysql/certs/ca-cert.pem", "--silent"]
      interval: 5s
      timeout: 10s
      retries: 12

  # ── Client: run validation ────────────────────────────────────────────────────
  client:
    image: mysql:8.0
    depends_on:
      mysql-server:
        condition: service_healthy
    environment:
      MYSQL_HOST: mysql-server
      MYSQL_PORT: "3306"
      CERT_DIR: /certs
    volumes:
      - ./certs:/certs:ro
      - ./client/validate.sh:/validate.sh:ro
    command: bash /validate.sh

volumes:
  plugin_so:
```

- [ ] **Step 2: Create the top-level run script**

Create `docker/run-validation.sh`:

```bash
#!/bin/bash
# Run the full authentication_appkey Docker validation.
# Usage: bash docker/run-validation.sh

set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

echo "=== Step 1: Generate certificates ==="
bash certs/gen-certs.sh

echo ""
echo "=== Step 2: Build and run validation ==="
docker compose down --volumes --remove-orphans 2>/dev/null || true
docker compose up --build --abort-on-container-exit --exit-code-from client

echo ""
echo "=== Cleaning up ==="
docker compose down --volumes --remove-orphans
```

- [ ] **Step 3: Make executable and commit**

```bash
chmod +x docker/run-validation.sh
git add docker/docker-compose.yml docker/run-validation.sh
git commit -m "feat(auth_appkey/docker): add docker-compose orchestration and run script"
```

---

## Task 7: Run the Validation

- [ ] **Step 1: Run the full validation**

```bash
bash docker/run-validation.sh
```

Expected output:
```
=== authentication_appkey Plugin Validation ===

MySQL is ready.

--- Test Cases ---
  [TC1 success] ... PASS
  [TC2 wrong_appkey] ... PASS (correctly rejected)
  [TC3 wrong_ip] ... PASS (correctly rejected)
  [TC4 no_cert] ... PASS (correctly rejected)
  [TC5 empty_password] ... PASS (correctly rejected)

=== Results: 5 passed, 0 failed ===
```

- [ ] **Step 2: If TC1 fails — check MySQL error log**

```bash
docker compose -f docker/docker-compose.yml logs mysql-server 2>&1 | grep -i "auth_appkey\|ERROR\|plugin"
```

Common issues:
- `[auth_appkey] FAILED reason=no_ssl` → MySQL not using TLS; check `tls_version` in my.cnf
- `[auth_appkey] FAILED reason=no_cert` → client not sending cert; check `--ssl-cert` flag
- `[auth_appkey] FAILED reason=appkey_mismatch` → cert SAN URI doesn't match auth_string; check gen-certs.sh output
- `[auth_appkey] FAILED reason=ip_mismatch` → cert IP `127.0.0.1` doesn't match `getpeername()` result; in Docker the client IP will be the container's IP, not 127.0.0.1

**IMPORTANT — Docker networking IP issue:**
When connecting from the client container to the mysql-server container, the client IP seen by `getpeername()` will be the Docker bridge network IP (e.g., `172.17.0.x`), NOT `127.0.0.1`. The client cert must have the actual Docker bridge IP in its SAN.

Fix: modify `gen-certs.sh` to accept an IP argument, or generate the cert dynamically after the network is known. See Task 7 Step 3.

- [ ] **Step 3: Fix Docker network IP in client cert**

The client container IP on the Docker bridge is typically `172.17.0.x` or `172.18.0.x`. To handle this dynamically:

Update `docker/certs/gen-certs.sh` to accept an optional IP argument:

```bash
CLIENT_IP="${1:-127.0.0.1}"
# Replace the client-ext.cnf line:
cat > client-ext.cnf <<EOF
[v3_req]
subjectAltName = IP:${CLIENT_IP},URI:appkey:com.test.app
EOF
```

And update `docker/run-validation.sh` to pass the Docker bridge IP:

```bash
# Get the Docker bridge subnet (default 172.17.0.0/16, client is usually .3 or .4)
# The simplest approach: use host.docker.internal or generate cert inside the container
# Alternative: use --network=host so client IP is 127.0.0.1
```

**Simplest fix — use host networking for the client:**

Update `docker/docker-compose.yml` client service:
```yaml
  client:
    network_mode: "host"
    environment:
      MYSQL_HOST: 127.0.0.1
      MYSQL_PORT: "13306"   # use the host-mapped port
```

And publish the server port:
```yaml
  mysql-server:
    ports:
      - "13306:3306"
```

With `network_mode: host`, the client connects from `127.0.0.1`, matching the cert SAN.

- [ ] **Step 4: Re-run after networking fix**

```bash
bash docker/run-validation.sh
```

Expected: all 5 tests pass.

- [ ] **Step 5: Commit final state**

```bash
git add docker/
git commit -m "feat(auth_appkey/docker): complete Docker validation — all 5 test cases pass"
```

---

## Self-Review: Spec Coverage

| Spec Requirement | Covered By |
|-----------------|-----------|
| SSL peer certificate check | Task 1 Step 1 (get_ssl + SSL_get_peer_certificate) |
| Certificate chain validation | Task 1 Step 1 (SSL_get_verify_result) |
| SAN parsing: IP Address + URI:appkey: | Task 1 (san_parser.cc reused) |
| AppKey match | Task 1 Step 1 (san.appkey vs expected_appkey) |
| Client IP match | Task 1 Step 1 (getpeername vs san.ip) |
| Password liveness check | Task 1 Step 1 (read_packet) |
| No Percona-internal headers | Task 1 (only mysql/plugin_auth.h, openssl) |
| Test certificates with correct SAN | Task 2 |
| Success test case | Task 5 TC1 |
| Wrong appkey failure | Task 5 TC2 |
| Wrong IP failure | Task 5 TC3 |
| No cert failure | Task 5 TC4 |
| Empty password failure | Task 5 TC5 |
| Docker networking IP alignment | Task 7 Step 3 |
