#!/bin/sh

set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT="."
REGISTRY="${SCRIPT_DIR}/components.txt"
PINS=""

KNOWN_UNCERTAIN=""

while [ $# -gt 0 ]; do
    case "$1" in
        --root)     ROOT=${2:?};     shift 2 ;;
        --registry) REGISTRY=${2:?}; shift 2 ;;
        --pins)     PINS=${2:?};     shift 2 ;;
        *) echo "check-components: unknown argument: $1" >&2; exit 2 ;;
    esac
done

[ -f "$REGISTRY" ] || { echo "check-components: registry not found: $REGISTRY" >&2; exit 2; }
[ -d "$ROOT/extra" ] || { echo "check-components: no extra/ under root: $ROOT" >&2; exit 2; }
[ -z "$PINS" ] || [ -f "$PINS" ] || { echo "check-components: pins file not found: $PINS" >&2; exit 2; }

if [ -z "$PINS" ] && [ -f "$ROOT/build-ps/sbom/submodule-pins.txt" ]; then
    PINS="$ROOT/build-ps/sbom/submodule-pins.txt"
fi

ERRORS=0
UNVERIFIED=0
err()  { echo "check-components: ERROR: $*" >&2; ERRORS=$((ERRORS + 1)); }
warn() { echo "check-components: WARNING: $*" >&2; }

ROWS=$(mktemp "${TMPDIR:-/tmp}/ps-reg.XXXXXX")
SUBS=$(mktemp "${TMPDIR:-/tmp}/ps-sub.XXXXXX")
trap 'rm -f "$ROWS" "$SUBS"' EXIT

awk -F'|' '
    { sub(/#.*/, "") }
    /^[[:space:]]*$/ { next }
    {
        for (i = 1; i <= NF; i++) gsub(/^[ \t]+|[ \t]+$/, "", $i)
        if ($1 == "") next
        print $1 "|" $2 "|" $4 "|" $6 "|" $7
    }
' "$REGISTRY" > "$ROWS"

if [ -f "$ROOT/.gitmodules" ]; then
    awk '/^[[:space:]]*path[[:space:]]*=/ {
        sub(/^[^=]*=[[:space:]]*/, ""); gsub(/[[:space:]]+$/, ""); print
    }' "$ROOT/.gitmodules" > "$SUBS"
else
    : > "$SUBS"
fi

is_submodule() {
    while IFS= read -r _s; do
        [ "$_s" = "$1" ] && return 0
    done < "$SUBS"
    return 1
}

resolve_commit() {
    _path=$1
    if [ -n "$PINS" ] && [ -f "$PINS" ]; then
        _c=$(awk -F'|' -v p="$_path" '$1 == p { print $2; exit }' "$PINS")
        if [ -n "$_c" ]; then printf '%s' "$_c"; return 0; fi
    fi
    if command -v git >/dev/null 2>&1 && [ -e "$ROOT/$_path/.git" ]; then
        _c=$(git -C "$ROOT/$_path" rev-parse HEAD 2>/dev/null) || _c=""
        if [ -n "$_c" ]; then printf '%s' "$_c"; return 0; fi
    fi
    return 1
}

is_sha() {
    printf '%s' "$1" | grep -qE '^[0-9a-f]{40}$'
}

norm() { printf '%s' "$1" | tr 'A-Z' 'a-z' | tr -cd 'a-z0-9'; }

while IFS='|' read -r name version ships path vsrc; do
    [ -n "$name" ] || continue

    case " $KNOWN_UNCERTAIN " in
        *" $name "*) [ "$ships" = "uncertain" ] && warn "$name: ships=uncertain (allowlisted; resolve and set yes/no/tarball)" ;;
        *) [ "$ships" = "uncertain" ] && err "$name: ships=uncertain and not allowlisted" ;;
    esac

    case "$ships" in
        yes|tarball|no) ;;
        *) err "$name: invalid ships value '$ships' (expected yes, tarball, no or uncertain)" ;;
    esac

    if [ "$ships" != "no" ] && [ -z "$version" ]; then
        err "$name: ships=$ships with an empty version (use 'unknown' if genuinely unversioned)"
    fi

    if [ "$path" != "-" ]; then
        if [ ! -e "$ROOT/$path" ]; then
            if is_submodule "$path"; then
                err "$name: $path missing - run 'git submodule update --init $path'"
            else
                err "$name: path does not exist: $path"
            fi
            continue
        fi
        if is_submodule "$path" && [ -z "$(ls -A "$ROOT/$path" 2>/dev/null)" ]; then
            err "$name: $path is an empty submodule - run 'git submodule update --init $path'"
            continue
        fi
    fi

    [ "$version" = "unknown" ] && continue

    if [ "$path" != "-" ] && is_submodule "$path" && is_sha "$version"; then
        actual=$(resolve_commit "$path") || actual=""
        if [ -z "$actual" ]; then
            err "$name: version is a commit but the submodule pin could not be resolved (no pins file and no .git)"
        elif [ "$actual" != "$version" ]; then
            err "$name: registry version '$version' does not match submodule pin '$actual'"
        fi
    fi

    case "$vsrc" in
        dir)
            if [ "$path" = "-" ]; then
                err "$name: version-src=dir requires a path"
                continue
            fi
            nv=$(norm "$version")
            nb=$(norm "$(basename "$path")")
            case "$nb" in
                *"$nv"*) ;;
                *) err "$name: version '$version' not found in path '$path'" ;;
            esac
            ;;
        file:*)
            vfile=${vsrc#file:}
            if [ ! -f "$ROOT/$vfile" ]; then
                err "$name: version-src file does not exist: $vfile"
                continue
            fi
            if ! grep -qF -- "$version" "$ROOT/$vfile"; then
                err "$name: version '$version' not found in $vfile"
            fi
            ;;
        none)
            UNVERIFIED=$((UNVERIFIED + 1))
            ;;
        *)
            err "$name: invalid version-src '$vsrc' (expected dir, file:<path> or none)"
            ;;
    esac
done < "$ROWS"

for d in "$ROOT"/extra/*/; do
    [ -d "$d" ] || continue
    dname=$(basename "$d")
    if ! awk -F'|' -v d="extra/${dname}" '
            $4 == d || index($4, d "/") == 1 { found = 1 }
            END { exit found ? 0 : 1 }' "$ROWS"; then
        err "extra/${dname} has no registry entry (add it, with ships=yes, tarball or no)"
    fi
done

while IFS= read -r sub; do
    [ -n "$sub" ] || continue
    if ! awk -F'|' -v d="$sub" '
            $4 == d { found = 1 }
            END { exit found ? 0 : 1 }' "$ROWS"; then
        err "submodule $sub has no registry entry"
    fi
done < "$SUBS"

if [ "$UNVERIFIED" -gt 0 ]; then
    echo "check-components: ${UNVERIFIED} component(s) have no automated version verification"
fi

if [ "$ERRORS" -gt 0 ]; then
    echo "check-components: FAILED with ${ERRORS} error(s)" >&2
    exit 1
fi
echo "check-components: registry consistent with tree"
