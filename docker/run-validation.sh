#!/bin/bash
# Run the full authentication_appkey Docker validation.
# Usage: bash docker/run-validation.sh [--no-cache]

set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

NO_CACHE=""
if [[ "${1:-}" == "--no-cache" ]]; then
  NO_CACHE="--no-cache"
fi

echo "=== Step 1: Generate test certificates ==="
bash certs/gen-certs.sh

echo ""
echo "=== Step 2: Build containers and run validation ==="
docker compose down --volumes --remove-orphans 2>/dev/null || true
docker compose up --build $NO_CACHE --abort-on-container-exit --exit-code-from client

EXIT_CODE=$?

echo ""
echo "=== Step 3: Cleanup ==="
docker compose down --volumes --remove-orphans

exit $EXIT_CODE
