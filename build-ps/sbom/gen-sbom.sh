#!/bin/sh

set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)

PKG=""
VER=""
DEST=""
ROOT="."
REGISTRY="${SCRIPT_DIR}/components.txt"
ARTIFACT="package"
SCAN_LIBS=""
PINS=""
EXCLUDE=""

usage() {
    echo "usage: $0 --pkg NAME --version VER --dest DIR" >&2
    echo "          [--root DIR] [--registry FILE] [--pins FILE]" >&2
    echo "          [--artifact package|tarball|source] [--scan-libs DIR]" >&2
    echo "          [--exclude NAME[,NAME...]]" >&2
    exit 2
}

die() { echo "gen-sbom: ERROR: $*" >&2; exit 1; }

while [ $# -gt 0 ]; do
    case "$1" in
        --pkg)       PKG=${2:?};       shift 2 ;;
        --version)   VER=${2:?};       shift 2 ;;
        --dest)      DEST=${2:?};      shift 2 ;;
        --root)      ROOT=${2:?};      shift 2 ;;
        --registry)  REGISTRY=${2:?};  shift 2 ;;
        --pins)      PINS=${2:?};      shift 2 ;;
        --artifact)  ARTIFACT=${2:?};  shift 2 ;;
        --scan-libs) SCAN_LIBS=${2:?}; shift 2 ;;
        --exclude)   EXCLUDE=${2:?};   shift 2 ;;
        -h|--help)   usage ;;
        *) echo "gen-sbom: unknown argument: $1" >&2; usage ;;
    esac
done

[ -n "$PKG" ]  || usage
[ -n "$VER" ]  || usage
[ -n "$DEST" ] || usage
[ -f "$REGISTRY" ] || die "registry not found: $REGISTRY"
[ -z "$PINS" ] || [ -f "$PINS" ] || die "pins file not found: $PINS"
[ -z "$SCAN_LIBS" ] || [ -d "$SCAN_LIBS" ] || die "scan-libs directory not found: $SCAN_LIBS"

if [ -z "$PINS" ] && [ -f "$ROOT/build-ps/sbom/submodule-pins.txt" ]; then
    PINS="$ROOT/build-ps/sbom/submodule-pins.txt"
fi

case "$ARTIFACT" in
    package|tarball|source) ;;
    *) die "invalid --artifact '$ARTIFACT' (expected package, tarball or source)" ;;
esac

if command -v sha256sum >/dev/null 2>&1; then
    sha256() { sha256sum | cut -d' ' -f1; }
elif command -v shasum >/dev/null 2>&1; then
    sha256() { shasum -a 256 | cut -d' ' -f1; }
else
    die "no sha256sum or shasum available"
fi

WORK=$(mktemp -d "${TMPDIR:-/tmp}/ps-sbom.XXXXXX")
trap 'rm -rf "$WORK"' EXIT

COMPONENTS="${WORK}/components"
ROWS="${WORK}/rows"
SUBS="${WORK}/subs"

tr '\t' ' ' < "$REGISTRY" | tr -d '\000-\010\013\014\016-\037\177' > "${WORK}/registry.clean"

awk -F'|' '
    { sub(/#.*/, "") }
    /^[[:space:]]*$/ { next }
    {
        for (i = 1; i <= NF; i++) gsub(/^[ \t]+|[ \t]+$/, "", $i)
        if ($1 == "") next
        print $1 "|" $2 "|" $3 "|" $4 "|" $5 "|" $6
    }
' "${WORK}/registry.clean" > "$ROWS"

if [ -f "$ROOT/.gitmodules" ]; then
    awk '/^[[:space:]]*path[[:space:]]*=/ {
        sub(/^[^=]*=[[:space:]]*/, ""); gsub(/[[:space:]]+$/, ""); print
    }' "$ROOT/.gitmodules" > "$SUBS"
else
    : > "$SUBS"
fi

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

is_submodule() {
    while IFS= read -r _s; do
        [ "$_s" = "$1" ] && return 0
    done < "$SUBS"
    return 1
}

: > "$COMPONENTS"
UNPINNED=""
EXCLUDED=""

while IFS='|' read -r name version license ships linkage path; do
    [ -n "$name" ] || continue

    case "$ships" in
        yes) ;;
        tarball) [ "$ARTIFACT" = "tarball" ] || continue ;;
        *) continue ;;
    esac

    case ",${EXCLUDE}," in
        *",${name},"*) EXCLUDED="${EXCLUDED} ${name}"; continue ;;
    esac

    [ -n "$version" ] || die "$name: ships=$ships with an empty version"

    commit=""
    if [ "$path" != "-" ] && is_submodule "$path"; then
        commit=$(resolve_commit "$path") || commit=""
        [ -n "$commit" ] || UNPINNED="${UNPINNED} ${name}"
    fi

    [ -n "$license" ] || license="NOASSERTION"

    printf '%s|%s|%s|%s|generic|vendored|%s\n' \
        "$name" "$version" "$license" "$linkage" "$commit" >> "$COMPONENTS"
done < "$ROWS"

if [ -s "$COMPONENTS" ]; then :; else
    die "no components selected for artifact '$ARTIFACT' - registry is empty or over-filtered"
fi

if [ -n "$EXCLUDED" ]; then
    echo "gen-sbom: excluded by caller (resolved to a system library):${EXCLUDED}"
fi

if [ -n "$UNPINNED" ]; then
    echo "gen-sbom: WARNING: no commit resolved for submodule component(s):${UNPINNED}" >&2
    echo "gen-sbom: WARNING: pass --pins, or place submodule-pins.txt under <root>/build-ps/sbom/" >&2
fi

row_is_sane() {
    [ "$(printf '%s' "$1" | awk -F'|' 'END { print NF }')" = "3" ] || return 1
    case $1 in
        *'	'*) return 1 ;;
    esac
    printf '%s' "$1" | awk -F'|' '{ exit ($1 == "" || $2 == "") ? 1 : 0 }'
}

resolve_owner() {
    _p=$1
    if command -v rpm >/dev/null 2>&1; then
        _r=$(rpm -qf --qf '%{NAME}|%{VERSION}-%{RELEASE}|%{LICENSE}\n' "$_p" 2>/dev/null | head -1) || _r=""
        case $_r in
            *'|'*'|'*) if row_is_sane "$_r"; then printf '%s|rpm' "$_r"; return 0; fi ;;
        esac
    fi
    if command -v dpkg-query >/dev/null 2>&1; then
        _pk=$(dpkg -S "$_p" 2>/dev/null \
              | grep -v '^diversion by ' \
              | head -1 | cut -d: -f1) || _pk=""
        if [ -n "$_pk" ]; then
            _v=$(dpkg-query -W -f='${Version}\n' "$_pk" 2>/dev/null | head -1) || _v=""
            [ -n "$_v" ] || _v="unknown"
            printf '%s|%s|NOASSERTION|deb' "$_pk" "$_v"
            return 0
        fi
    fi
    return 1
}

SCANNED=0
RESOLVED=0

if [ -n "$SCAN_LIBS" ]; then
    HOSTMAP="${WORK}/hostmap"
    ldconfig -p 2>/dev/null | awk '/=>/ { print $NF }' | while IFS= read -r _p; do
        _rp=$(readlink -f "$_p" 2>/dev/null) || continue
        [ -f "$_rp" ] || continue
        printf '%s|%s\n' "$(basename "$_rp")" "$_rp"
    done | sort -u > "$HOSTMAP" || true

    for f in "$SCAN_LIBS"/*; do
        [ -L "$f" ] && continue
        [ -f "$f" ] || continue
        SCANNED=$((SCANNED + 1))
        b=$(basename "$f")
        hp=$(awk -F'|' -v b="$b" '$1 == b { print $2; exit }' "$HOSTMAP" 2>/dev/null) || hp=""
        [ -n "$hp" ] || continue
        owner=$(resolve_owner "$hp") || continue
        RESOLVED=$((RESOLVED + 1))
        oname=$(printf '%s' "$owner" | cut -d'|' -f1)
        over=$(printf '%s' "$owner" | cut -d'|' -f2)
        olic=$(printf '%s' "$owner" | cut -d'|' -f3)
        otype=$(printf '%s' "$owner" | cut -d'|' -f4)
        [ -n "$olic" ] || olic="NOASSERTION"
        if ! awk -F'|' -v n="$oname" '$1 == n { found = 1 } END { exit found ? 0 : 1 }' "$COMPONENTS"; then
            printf '%s|%s|%s|shared-lib|%s|host|\n' \
                "$oname" "$over" "$olic" "$otype" >> "$COMPONENTS"
        fi
    done

    echo "gen-sbom: scanned ${SCANNED} bundled libraries, resolved ${RESOLVED} to host packages"
    if [ "$SCANNED" -gt 0 ] && [ "$RESOLVED" -eq 0 ]; then
        echo "gen-sbom: WARNING: no bundled library resolved to a host package;" \
             "is ldconfig available and is the rpm or dpkg database readable?" >&2
    fi
fi

UUID=$(printf '%s|%s|%s|%s' "$PKG" "$VER" "$ARTIFACT" "$(cat "$COMPONENTS")" | sha256 | awk '{
    h = $0
    printf "%s-%s-4%s-a%s-%s",
        substr(h, 1, 8), substr(h, 9, 4), substr(h, 14, 3),
        substr(h, 18, 3), substr(h, 21, 12)
}')

if [ -n "${SOURCE_DATE_EPOCH:-}" ]; then
    CREATED=$(date -u -d "@${SOURCE_DATE_EPOCH}" '+%Y-%m-%dT%H:%M:%SZ' 2>/dev/null \
              || date -u -r "${SOURCE_DATE_EPOCH}" '+%Y-%m-%dT%H:%M:%SZ' 2>/dev/null) || CREATED=""
fi
[ -n "${CREATED:-}" ] || CREATED=$(date -u '+%Y-%m-%dT%H:%M:%SZ')

DUPS=$(cut -d'|' -f1 "$COMPONENTS" | sort | uniq -d)
[ -z "$DUPS" ] || die "duplicate component names would collide as SPDX identifiers: $(echo $DUPS)"

mkdir -p "$DEST"

AWK_LIB='
function jesc(s,   i, c, o) {
    o = ""
    for (i = 1; i <= length(s); i++) {
        c = substr(s, i, 1)
        if (c == "\\") o = o "\\\\"
        else if (c == "\"") o = o "\\\""
        else if (c == "\n") o = o "\\n"
        else if (c == "\r") o = o "\\r"
        else if (c == "\t") o = o "\\t"
        else if (c < " ") o = o sprintf("\\u%04x", ORD[c])
        else o = o c
    }
    return o
}
function penc(s,   i, c, o) {
    o = ""
    for (i = 1; i <= length(s); i++) {
        c = substr(s, i, 1)
        if (c ~ /[A-Za-z0-9._~:-]/) o = o c
        else o = o sprintf("%%%02X", ORD[c])
    }
    return o
}
function purl(type, name, version) {
    return "pkg:" type "/" penc(name) "@" penc(version)
}
BEGIN { for (i = 0; i < 256; i++) ORD[sprintf("%c", i)] = i }
'

awk -F'|' -v pkg="$PKG" -v ver="$VER" -v uuid="$UUID" -v created="$CREATED" \
    -v artifact="$ARTIFACT" "$AWK_LIB"'
{
    name[NR] = $1; vers[NR] = $2; lic[NR] = $3
    link[NR] = $4; ptype[NR] = $5; orig[NR] = $6; commit[NR] = $7
    n = NR
}
END {
    printf "{\n"
    printf "  \"spdxVersion\": \"SPDX-2.3\",\n"
    printf "  \"dataLicense\": \"CC0-1.0\",\n"
    printf "  \"SPDXID\": \"SPDXRef-DOCUMENT\",\n"
    printf "  \"name\": \"%s-%s\",\n", jesc(pkg), jesc(ver)
    printf "  \"documentNamespace\": \"https://percona.com/spdxdocs/%s-%s-%s\",\n", jesc(pkg), jesc(ver), uuid
    printf "  \"creationInfo\": {\n"
    printf "    \"created\": \"%s\",\n", created
    printf "    \"creators\": [\"Organization: Percona LLC\", \"Tool: gen-sbom.sh\"]\n"
    printf "  },\n"
    printf "  \"packages\": [\n"
    printf "    {\n"
    printf "      \"SPDXID\": \"SPDXRef-Package-root\",\n"
    printf "      \"name\": \"%s\",\n", jesc(pkg)
    printf "      \"versionInfo\": \"%s\",\n", jesc(ver)
    printf "      \"downloadLocation\": \"NOASSERTION\",\n"
    printf "      \"filesAnalyzed\": false,\n"
    printf "      \"licenseConcluded\": \"GPL-2.0-only\",\n"
    printf "      \"licenseDeclared\": \"GPL-2.0-only\",\n"
    printf "      \"copyrightText\": \"NOASSERTION\",\n"
    printf "      \"supplier\": \"Organization: Percona LLC\"\n"
    printf "    }"
    for (i = 1; i <= n; i++) {
        printf ",\n    {\n"
        printf "      \"SPDXID\": \"SPDXRef-Component-%d\",\n", i
        printf "      \"name\": \"%s\",\n", jesc(name[i])
        printf "      \"versionInfo\": \"%s\",\n", jesc(vers[i])
        printf "      \"downloadLocation\": \"NOASSERTION\",\n"
        printf "      \"filesAnalyzed\": false,\n"
        printf "      \"licenseConcluded\": \"NOASSERTION\",\n"
        printf "      \"licenseDeclared\": \"%s\",\n", jesc(lic[i])
        printf "      \"copyrightText\": \"NOASSERTION\",\n"
        printf "      \"externalRefs\": [{\n"
        printf "        \"referenceCategory\": \"PACKAGE-MANAGER\",\n"
        printf "        \"referenceType\": \"purl\",\n"
        printf "        \"referenceLocator\": \"%s\"\n", purl(ptype[i], name[i], vers[i])
        printf "      }],\n"
        if (commit[i] != "")
            printf "      \"sourceInfo\": \"linkage=%s origin=%s commit=%s\",\n", jesc(link[i]), jesc(orig[i]), jesc(commit[i])
        else
            printf "      \"sourceInfo\": \"linkage=%s origin=%s\",\n", jesc(link[i]), jesc(orig[i])
        printf "      \"comment\": \"artifact=%s\"\n", jesc(artifact)
        printf "    }"
    }
    printf "\n  ],\n"
    printf "  \"relationships\": [\n"
    printf "    {\"spdxElementId\": \"SPDXRef-DOCUMENT\", \"relatedSpdxElement\": \"SPDXRef-Package-root\", \"relationshipType\": \"DESCRIBES\"}"
    for (i = 1; i <= n; i++) {
        printf ",\n    {\"spdxElementId\": \"SPDXRef-Package-root\", \"relatedSpdxElement\": \"SPDXRef-Component-%d\", \"relationshipType\": \"CONTAINS\"}", i
    }
    printf "\n  ]\n"
    printf "}\n"
}
' "$COMPONENTS" > "${DEST}/${PKG}.spdx.json"

awk -F'|' -v pkg="$PKG" -v ver="$VER" -v uuid="$UUID" -v created="$CREATED" \
    -v artifact="$ARTIFACT" "$AWK_LIB"'
{
    name[NR] = $1; vers[NR] = $2; lic[NR] = $3
    link[NR] = $4; ptype[NR] = $5; orig[NR] = $6; commit[NR] = $7
    n = NR
}
END {
    printf "{\n"
    printf "  \"bomFormat\": \"CycloneDX\",\n"
    printf "  \"specVersion\": \"1.5\",\n"
    printf "  \"serialNumber\": \"urn:uuid:%s\",\n", uuid
    printf "  \"version\": 1,\n"
    printf "  \"metadata\": {\n"
    printf "    \"timestamp\": \"%s\",\n", created
    printf "    \"tools\": [{\"vendor\": \"Percona\", \"name\": \"gen-sbom.sh\"}],\n"
    printf "    \"component\": {\n"
    printf "      \"type\": \"application\",\n"
    printf "      \"bom-ref\": \"root\",\n"
    printf "      \"name\": \"%s\",\n", jesc(pkg)
    printf "      \"version\": \"%s\",\n", jesc(ver)
    printf "      \"supplier\": {\"name\": \"Percona LLC\"},\n"
    printf "      \"licenses\": [{\"license\": {\"id\": \"GPL-2.0-only\"}}]\n"
    printf "    }\n"
    printf "  },\n"
    printf "  \"components\": [\n"
    for (i = 1; i <= n; i++) {
        printf "    {\n"
        printf "      \"type\": \"library\",\n"
        printf "      \"bom-ref\": \"component-%d\",\n", i
        printf "      \"name\": \"%s\",\n", jesc(name[i])
        printf "      \"version\": \"%s\",\n", jesc(vers[i])
        printf "      \"scope\": \"required\",\n"
        printf "      \"licenses\": [{\"license\": {\"name\": \"%s\"}}],\n", jesc(lic[i])
        printf "      \"purl\": \"%s\",\n", purl(ptype[i], name[i], vers[i])
        printf "      \"properties\": [\n"
        printf "        {\"name\": \"percona:linkage\", \"value\": \"%s\"},\n", jesc(link[i])
        printf "        {\"name\": \"percona:origin\", \"value\": \"%s\"},\n", jesc(orig[i])
        if (commit[i] != "")
            printf "        {\"name\": \"percona:commit\", \"value\": \"%s\"},\n", jesc(commit[i])
        printf "        {\"name\": \"percona:artifact\", \"value\": \"%s\"}\n", jesc(artifact)
        printf "      ]\n"
        printf "    }%s\n", (i < n ? "," : "")
    }
    printf "  ],\n"
    printf "  \"dependencies\": [\n"
    printf "    {\"ref\": \"root\", \"dependsOn\": ["
    for (i = 1; i <= n; i++) printf "%s\"component-%d\"", (i > 1 ? ", " : ""), i
    printf "]}\n"
    printf "  ]\n"
    printf "}\n"
}
' "$COMPONENTS" > "${DEST}/${PKG}.cdx.json"

{
    echo "Percona Server for MySQL - third-party components"
    echo "Package:  ${PKG}"
    echo "Version:  ${VER}"
    echo "Artifact: ${ARTIFACT}"
    echo "Created:  ${CREATED}"
    echo
    printf '%-22s %-26s %-14s %s\n' "COMPONENT" "VERSION" "ORIGIN" "LICENSE"
    awk -F'|' '{ printf "%-22s %-26s %-14s %s\n", $1, $2, $6, $3 }' "$COMPONENTS"
} > "${DEST}/${PKG}.sbom.txt"

awk -F'|' '{ print $3 }' "$COMPONENTS" | sort -u > "${DEST}/${PKG}.licenses.txt"

chmod 0755 "$DEST"
for f in "${DEST}/${PKG}.spdx.json" "${DEST}/${PKG}.cdx.json" \
         "${DEST}/${PKG}.sbom.txt" "${DEST}/${PKG}.licenses.txt"; do
    chmod 0644 "$f"
done

echo "gen-sbom: wrote $(wc -l < "$COMPONENTS") components to ${DEST} (artifact=${ARTIFACT})"
