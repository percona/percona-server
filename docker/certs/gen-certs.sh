#!/bin/bash
# Generates CA, server cert, and client certs for auth_appkey Docker validation.
#
# Cert layout:
#   ca-cert.pem / ca-key.pem              — root CA
#   server-cert.pem / server-key.pem      — MySQL server cert
#   client-cert.pem / client-key.pem      — valid client (IP:127.0.0.1, appkey:com.test.app)
#   client-wrong-appkey-cert.pem          — wrong appkey (com.wrong.app)
#   client-wrong-ip-cert.pem              — wrong IP (10.0.0.99)
#   client-no-san-cert.pem                — no SAN at all

set -euo pipefail
DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$DIR"

# ── CA ────────────────────────────────────────────────────────────────────────
openssl genrsa -out ca-key.pem 2048 2>/dev/null
openssl req -new -x509 -days 3650 -key ca-key.pem -out ca-cert.pem \
  -subj "/CN=TestCA/O=Percona/C=US"

# ── Server cert ───────────────────────────────────────────────────────────────
openssl genrsa -out server-key.pem 2048 2>/dev/null
openssl req -new -key server-key.pem -out server-csr.pem \
  -subj "/CN=mysql-server/O=Percona/C=US"
cat > server-ext.cnf <<'EOF'
[v3_req]
subjectAltName = DNS:localhost,IP:127.0.0.1
EOF
openssl x509 -req -days 3650 -in server-csr.pem \
  -CA ca-cert.pem -CAkey ca-key.pem -CAcreateserial \
  -out server-cert.pem -extfile server-ext.cnf -extensions v3_req 2>/dev/null

# ── Client key (shared across all client certs) ───────────────────────────────
openssl genrsa -out client-key.pem 2048 2>/dev/null
openssl req -new -key client-key.pem -out client-csr.pem \
  -subj "/CN=appkey-client/O=Percona/C=US"

# ── TC1: valid client cert ────────────────────────────────────────────────────
cat > client-ext.cnf <<'EOF'
[v3_req]
subjectAltName = IP:127.0.0.1,URI:appkey:com.test.app
EOF
openssl x509 -req -days 3650 -in client-csr.pem \
  -CA ca-cert.pem -CAkey ca-key.pem -CAcreateserial \
  -out client-cert.pem -extfile client-ext.cnf -extensions v3_req 2>/dev/null

# ── TC2: wrong appkey ─────────────────────────────────────────────────────────
cat > client-wrong-appkey-ext.cnf <<'EOF'
[v3_req]
subjectAltName = IP:127.0.0.1,URI:appkey:com.wrong.app
EOF
openssl x509 -req -days 3650 -in client-csr.pem \
  -CA ca-cert.pem -CAkey ca-key.pem -CAcreateserial \
  -out client-wrong-appkey-cert.pem \
  -extfile client-wrong-appkey-ext.cnf -extensions v3_req 2>/dev/null

# ── TC3: wrong IP ─────────────────────────────────────────────────────────────
cat > client-wrong-ip-ext.cnf <<'EOF'
[v3_req]
subjectAltName = IP:10.0.0.99,URI:appkey:com.test.app
EOF
openssl x509 -req -days 3650 -in client-csr.pem \
  -CA ca-cert.pem -CAkey ca-key.pem -CAcreateserial \
  -out client-wrong-ip-cert.pem \
  -extfile client-wrong-ip-ext.cnf -extensions v3_req 2>/dev/null

echo "Certificates generated in $DIR"
echo ""
echo "Verifying client cert SAN:"
openssl x509 -in client-cert.pem -noout -text | grep -A3 "Subject Alternative Name"
