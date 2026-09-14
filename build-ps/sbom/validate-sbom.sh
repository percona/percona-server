#!/bin/bash

set -uo pipefail

REQUIRED_COMPONENTS="protobuf zlib zstd lz4 icu abseil-cpp libkmip coredumper rocksdb boost"
MIN_COMPONENTS=15

ERRORS=0
err() { echo "validate-sbom: ERROR: $*" >&2; ERRORS=$((ERRORS + 1)); }
ok()  { echo "validate-sbom: OK: $*"; }

[ $# -gt 0 ] || { echo "usage: $0 ARTIFACT [ARTIFACT...]" >&2; exit 2; }

TMP=$(mktemp -d "${TMPDIR:-/tmp}/ps-vsbom.XXXXXX")
trap 'rm -rf "$TMP"' EXIT

extract() {
    local art=$1 dest=$2
    mkdir -p "$dest"
    case "$art" in
        *.rpm)    (cd "$dest" && rpm2cpio "$art" | cpio -idmu --quiet) ;;
        *.deb)    dpkg-deb -x "$art" "$dest" ;;
        *.tar.gz) tar xzf "$art" -C "$dest" ;;
        *)        [ -d "$art" ] && cp -r "$art/." "$dest/" ;;
    esac
}

check_json() {
    local f=$1
    if command -v python3 >/dev/null 2>&1; then
        python3 -c "import json,sys; json.load(open(sys.argv[1]))" "$f" 2>/dev/null
    else
        return 0
    fi
}

idx=0
for art in "$@"; do
    idx=$((idx + 1))
    [ -e "$art" ] || { err "artifact does not exist: $art"; continue; }

    name=$(basename "$art")
    errors_before=$ERRORS

    dest="${TMP}/$(printf '%03d' "$idx")_$(echo "$name" | tr -c 'A-Za-z0-9._-' '_')"
    rm -rf "$dest"
    extract "$art" "$dest"

    spdx=$(find "$dest" -name '*.spdx.json' -type f 2>/dev/null | head -1)
    cdx=$(find "$dest" -name '*.cdx.json' -type f 2>/dev/null | head -1)
    txt=$(find "$dest" -name '*.sbom.txt' -type f 2>/dev/null | head -1)
    lic=$(find "$dest" -name '*.licenses.txt' -type f 2>/dev/null | head -1)

    if [ -z "$spdx" ]; then
        err "$name: no .spdx.json found"
        continue
    fi
    [ -n "$cdx" ] || err "$name: no .cdx.json found"
    [ -n "$txt" ] || err "$name: no .sbom.txt found"
    [ -n "$lic" ] || err "$name: no .licenses.txt found"

    check_json "$spdx" || err "$name: .spdx.json is not valid JSON"
    [ -z "$cdx" ] || check_json "$cdx" || err "$name: .cdx.json is not valid JSON"

    count=$(grep -o '"SPDXID": "SPDXRef-Component-' "$spdx" 2>/dev/null | wc -l | tr -d ' ')
    count=${count:-0}
    if [ "$count" -lt "$MIN_COMPONENTS" ]; then
        err "$name: only ${count} components in SPDX, expected at least ${MIN_COMPONENTS}"
    fi

    for c in $REQUIRED_COMPONENTS; do
        if ! grep -q "\"name\": \"${c}\"" "$spdx"; then
            err "$name: required component missing from SPDX: ${c}"
        fi
    done

    if [ "$ERRORS" -eq "$errors_before" ]; then
        ok "$name: ${count} components, all required present"
    fi
done

if [ "$ERRORS" -gt 0 ]; then
    echo "validate-sbom: FAILED with ${ERRORS} error(s)" >&2
    exit 1
fi
echo "validate-sbom: all artifacts carry a valid SBOM"
