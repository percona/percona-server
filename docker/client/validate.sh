#!/bin/bash
# End-to-end validation for authentication_appkey plugin.
# Runs inside a MySQL client container in the Docker bridge network.
# Detects the container's own IP and generates a client certificate with that
# IP in the SAN at startup, so TC1 (correct cert) works on any host OS.

set -euo pipefail

MYSQL_HOST="${MYSQL_HOST:-mysql-server}"
MYSQL_PORT="${MYSQL_PORT:-3306}"
CERT_DIR="${CERT_DIR:-/certs}"
PASS=0
FAIL=0

# ── Detect client IP ──────────────────────────────────────────────────────────
# Get the IP of this container on the Docker bridge network.
CLIENT_IP=$(hostname -I | awk '{print $1}')
echo ""
echo "=== authentication_appkey Plugin Validation ==="
echo ""
echo "Client IP: $CLIENT_IP"

# ── Generate client certificate with actual IP ────────────────────────────────
# We need openssl; it's available in the percona-server image via the SSL libs.
# The CA key is mounted read-only, so we copy it to a writable tmp dir.
TMPDIR=$(mktemp -d)
trap 'rm -rf "$TMPDIR"' EXIT

cp "$CERT_DIR/ca-cert.pem" "$TMPDIR/"
cp "$CERT_DIR/ca-key.pem"  "$TMPDIR/"

# Generate key
openssl genrsa -out "$TMPDIR/client-rt-key.pem" 2048 2>/dev/null

# Generate CSR
openssl req -new -key "$TMPDIR/client-rt-key.pem" \
    -out "$TMPDIR/client-rt-csr.pem" \
    -subj "/CN=appkey-client-rt" 2>/dev/null

# Extension file with actual IP and appkey URI
cat > "$TMPDIR/client-rt-ext.cnf" <<EOF
subjectAltName = IP:${CLIENT_IP}, URI:appkey:com.test.app
EOF

# Sign with CA
openssl x509 -req \
    -in "$TMPDIR/client-rt-csr.pem" \
    -CA "$TMPDIR/ca-cert.pem" \
    -CAkey "$TMPDIR/ca-key.pem" \
    -CAcreateserial \
    -out "$TMPDIR/client-rt-cert.pem" \
    -days 1 \
    -extfile "$TMPDIR/client-rt-ext.cnf" 2>/dev/null

echo "Generated client cert with IP:${CLIENT_IP}, URI:appkey:com.test.app"

# ── Helper ────────────────────────────────────────────────────────────────────

run_test() {
  local label="$1"
  local expect_success="$2"
  shift 2
  local args=("$@")

  printf "  [%-30s] ... " "$label"

  local output
  output=$(mysql \
      --host="$MYSQL_HOST" \
      --port="$MYSQL_PORT" \
      --ssl-ca="$CERT_DIR/ca-cert.pem" \
      "${args[@]}" \
      --connect-timeout=5 \
      --execute="SELECT 'connected' AS result;" 2>&1) || true

  if echo "$output" | grep -q "connected"; then
    if [ "$expect_success" = "true" ]; then
      echo "PASS"
      PASS=$((PASS + 1))
    else
      echo "FAIL  (expected rejection, got success)"
      FAIL=$((FAIL + 1))
    fi
  else
    if [ "$expect_success" = "false" ]; then
      echo "PASS  (correctly rejected)"
      PASS=$((PASS + 1))
    else
      echo "FAIL  (expected success)"
      echo "      mysql output: $output"
      FAIL=$((FAIL + 1))
    fi
  fi
}

# ── Wait for MySQL ────────────────────────────────────────────────────────────

echo ""
echo "Waiting for MySQL server at $MYSQL_HOST:$MYSQL_PORT ..."
for i in $(seq 1 40); do
  if mysqladmin ping \
      --host="$MYSQL_HOST" \
      --port="$MYSQL_PORT" \
      --ssl-ca="$CERT_DIR/ca-cert.pem" \
      --silent 2>/dev/null; then
    echo "MySQL is ready."
    break
  fi
  if [ "$i" -eq 40 ]; then
    echo "ERROR: MySQL did not become ready in time."
    exit 1
  fi
  sleep 3
done

echo ""
echo "--- Test Cases ---"
echo ""

# ── TC1: Success ──────────────────────────────────────────────────────────────
# Runtime-generated cert: IP=<container_ip>, appkey=com.test.app
# account host=appkey:com.test.app
run_test "TC1 success" "true" \
  --user=appkey_user \
  --password=anypass \
  --ssl-cert="$TMPDIR/client-rt-cert.pem" \
  --ssl-key="$TMPDIR/client-rt-key.pem" \
  --tls-version=TLSv1.3

# ── TC2: Wrong appkey in cert ─────────────────────────────────────────────────
run_test "TC2 appkey_mismatch" "false" \
  --user=appkey_user \
  --password=anypass \
  --ssl-cert="$CERT_DIR/client-wrong-appkey-cert.pem" \
  --ssl-key="$CERT_DIR/client-key.pem" \
  --tls-version=TLSv1.3

# ── TC3: Wrong IP in cert ─────────────────────────────────────────────────────
run_test "TC3 ip_mismatch" "false" \
  --user=appkey_user \
  --password=anypass \
  --ssl-cert="$CERT_DIR/client-wrong-ip-cert.pem" \
  --ssl-key="$CERT_DIR/client-key.pem" \
  --tls-version=TLSv1.3

# ── TC4: No client cert ───────────────────────────────────────────────────────
run_test "TC4 no_cert" "false" \
  --user=appkey_user \
  --password=anypass \
  --tls-version=TLSv1.3

# ── TC5: Empty password ───────────────────────────────────────────────────────
run_test "TC5 empty_password" "false" \
  --user=appkey_user \
  --password="" \
  --ssl-cert="$TMPDIR/client-rt-cert.pem" \
  --ssl-key="$TMPDIR/client-rt-key.pem" \
  --tls-version=TLSv1.3

# ── Results ───────────────────────────────────────────────────────────────────

echo ""
echo "=== Results: $PASS passed, $FAIL failed ==="
echo ""

if [ "$FAIL" -gt 0 ]; then
  exit 1
fi
exit 0
