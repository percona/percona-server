#!/usr/bin/env bash
# Behaviour tests for check-pr-commit-subjects.sh against a stubbed gh.
#
# The checker makes two API calls. The stub answers both:
#   repos/O/R/pulls/N          -> CASE_BASE US CASE_TOTAL US CASE_TITLE
#   repos/O/R/pulls/N/commits  -> CASE_COMMITS verbatim
# CASE_RC makes gh fail, CASE_META overrides the metadata line wholesale.
set -uo pipefail

readonly SCRIPT="$1"
BIN="$(mktemp -d)"
readonly BIN
readonly US=$'\x1f'

cat >"${BIN}/gh" <<'STUB'
#!/usr/bin/env bash
if [[ "${FIXTURE_RC:-0}" -ne 0 ]]; then
  echo "gh: API error" >&2
  exit "${FIXTURE_RC}"
fi
for arg in "$@"; do
  case "${arg}" in
    */commits) printf '%s' "${FIXTURE_COMMITS}"; exit 0 ;;
  esac
done
printf '%s\n' "${FIXTURE_META}"
STUB
chmod +x "${BIN}/gh"
export PATH="${BIN}:${PATH}"

pass=0
fail=0

# run_case <name> <want_exit> <want_grep> ; reads CASE_* from the environment.
run_case() {
  local name="$1" want_exit="$2" want_grep="$3"
  local meta out rc
  meta="${CASE_META-${CASE_BASE}${US}${CASE_TOTAL}${US}${CASE_TITLE}}"
  out="$(PR_NUMBER=1 REPO=percona/percona-server \
    FIXTURE_META="${meta}" FIXTURE_COMMITS="${CASE_COMMITS}" \
    FIXTURE_RC="${CASE_RC:-0}" LC_ALL="${CASE_LOCALE:-C.utf8}" \
    bash "${SCRIPT}" 2>&1)"
  rc=$?
  if [[ "${rc}" -ne "${want_exit}" ]]; then
    echo "FAIL ${name}: exit ${rc}, want ${want_exit}"
    indent "${out}"
    fail=$((fail + 1))
    return
  fi
  if [[ -n "${want_grep}" ]] && ! grep -q "${want_grep}" <<<"${out}"; then
    echo "FAIL ${name}: output lacks '${want_grep}'"
    indent "${out}"
    fail=$((fail + 1))
    return
  fi
  printf 'ok   %s\n' "${name}"
  pass=$((pass + 1))
}

commit() { printf '%s%s%s%s%s\n' "$1" "${US}" "$2" "${US}" "$3"; }
indent() { printf '       %s\n' "${1//$'\n'/$'\n'       }"; }
section() { echo; echo "-- $1"; }

CASE_RC=0

# ---------------------------------------------------------------- review fixes
section 'the three maintainer findings'

CASE_BASE=8.4 CASE_TITLE='Merge partition pruning optimization' CASE_TOTAL=1
CASE_COMMITS="$(commit abc123abc123 1 'partition pruning optimization')"
run_case 'Merge title does not bypass the commit check' 1 'subject lacks a ticket key'
run_case 'Merge title is itself flagged' 1 'PR title lacks a ticket key'

CASE_BASE=8.4 CASE_TITLE='Null-merge 8.0 into 8.4' CASE_TOTAL=1
CASE_COMMITS="$(commit abc123abc123 1 'cherry-picked fix, no key')"
run_case 'Null-merge title does not bypass' 1 'subject lacks a ticket key'

CASE_BASE=8.4 CASE_TITLE='PS-123 Add feature' CASE_TOTAL=1
CASE_COMMITS="$(commit abc123abc123 1 'PS-123 [8.4] Add feature')"
run_case 'single-commit title needs the tag' 1 'PR title lacks the \[8.4\] branch tag'

CASE_BASE=trunk CASE_TITLE='PS-123 [10.x] Add feature' CASE_TOTAL=1
CASE_COMMITS="$(commit abc123abc123 1 'PS-123 [10.x] Add feature')"
run_case '[10.x] rejected on trunk' 1 'carries \[10.x\] but a pull request to trunk needs \[trunk\]'

CASE_BASE=trunk CASE_TITLE='PS-123 [trunk] Add feature' CASE_TOTAL=1
CASE_COMMITS="$(commit abc123abc123 1 'PS-123 [trunk] Add feature')"
run_case '[trunk] accepted on trunk' 0 'carry a ticket key'

# ------------------------------------------------------- second-review findings
section 'findings from the multi-model review'

# Metadata comes from the API, so a re-run of an old event judges the PR as it is.
CASE_BASE=8.4 CASE_TITLE='PS-1 [8.4] Title fixed after the run' CASE_TOTAL=1
CASE_COMMITS="$(commit abc123abc123 1 'PS-1 [8.4] one')"
run_case 'metadata is read from the API, not the event' 0 'carry a ticket key'

CASE_META='' CASE_BASE=8.4 CASE_TITLE=x CASE_TOTAL=1
CASE_COMMITS="$(commit abc123abc123 1 'PS-1 [8.4] one')"
run_case 'unreadable PR metadata fails' 1 'could not read pull request'
unset CASE_META

CASE_META="${US}${US}" CASE_BASE=8.4 CASE_TITLE=x CASE_TOTAL=1
CASE_COMMITS="$(commit abc123abc123 1 'PS-1 [8.4] one')"
run_case 'blank base branch fails instead of skipping' 1 'could not read the base branch'
unset CASE_META

# A zero-padded count used to be read as octal, so the truncation guard silently
# did not fire.
CASE_BASE=8.4 CASE_TITLE='PS-1 [8.4] Big' CASE_TOTAL=08
CASE_COMMITS="$(commit abc123abc123 1 'PS-1 [8.4] one')"
run_case 'a zero-padded count still trips the truncation guard' 1 'only 1 could be listed'

# The revert exemption survives, because PR 6149 reverted a subject that carried
# neither key nor tag. That is how the convention gets repaired.
CASE_BASE=8.4 CASE_TITLE='PS-11435 [8.4] Re-apply OIDC MTR suite change' CASE_TOTAL=2
CASE_COMMITS="$(commit abc123abc123 1 'Revert "fix(mtr): load OpenID Connect defaults for tests (#6076)"'
                commit def456def456 1 'PS-11435 [8.4] Add auth_openid_connect suite to default MTR runs')"
run_case 'the real PR 6149 shape passes' 0 'revert, subject checks skipped'

# Reapply is no longer exempt: a conforming one passes on its own merits.
CASE_BASE=8.4 CASE_TITLE='PS-1 [8.4] Reapply' CASE_TOTAL=1
CASE_COMMITS="$(commit abc123abc123 1 'Reapply "PS-1 [8.4] one"')"
run_case 'a conforming reapply passes unaided' 0 'carry a ticket key'

CASE_BASE=8.4 CASE_TITLE='PS-1 [8.4] Reapply' CASE_TOTAL=1
CASE_COMMITS="$(commit abc123abc123 1 'Reapply "sloppy subject"')"
run_case 'a non-conforming reapply is no longer exempt' 1 'subject lacks a ticket key'

# The merge exemption is announced, not silent.
CASE_BASE=8.4 CASE_TITLE='garbage' CASE_TOTAL=2
CASE_COMMITS="$(commit abc123abc123 1 'PS-1 [8.4] a'; commit def456def456 2 'Merge branch 8.0 into 8.4')"
run_case 'the merge exemption warns rather than passing silently' 0 '::warning::'

# A carriage return in a subject must not open a second workflow command.
CASE_BASE=8.4 CASE_TITLE='PS-1 [8.4] CR' CASE_TOTAL=1
CASE_COMMITS="$(commit abc123abc123 1 $'no key\r::stop-commands::PWN')"
run_case 'a CR in a subject is escaped' 1 '%0D'
CASE_COMMITS="$(commit abc123abc123 1 $'no key\r::stop-commands::PWN')"
CASE_BASE=8.4 CASE_TITLE='PS-1 [8.4] CR' CASE_TOTAL=1
out="$(PR_NUMBER=1 REPO=percona/percona-server \
  FIXTURE_META="8.4${US}1${US}PS-1 [8.4] CR" FIXTURE_COMMITS="${CASE_COMMITS}" \
  bash "${SCRIPT}" 2>&1)" || true
if grep -q $'\r' <<<"${out}"; then
  echo "FAIL no raw CR reaches the log"
  fail=$((fail + 1))
else
  echo "ok   no raw CR reaches the log"
  pass=$((pass + 1))
fi

CASE_BASE=8.4 CASE_TITLE='PS-1 [8.4] percent 100%' CASE_TOTAL=1
CASE_COMMITS="$(commit abc123abc123 1 'no key 50% off')"
run_case 'a percent sign is escaped' 1 '50%25 off'

# The key regex must not change verdict with the runner locale.
CASE_BASE=8.4 CASE_TITLE='PS-1 [8.4] Fix it' CASE_TOTAL=1
CASE_COMMITS="$(commit abc123abc123 1 'éPS-1 [8.4] Fix it')"
for loc in C C.utf8 en_US.UTF-8; do
  CASE_LOCALE="${loc}" run_case "verdict is the same under LC_ALL=${loc}" 0 'carry a ticket key'
done
unset CASE_LOCALE

# Same input, two locales, byte-identical output.
run_locale() {
  PR_NUMBER=1 REPO=percona/percona-server LC_ALL="$1" \
    FIXTURE_META="8.4${US}1${US}PS-1 [8.4] Fix it" \
    FIXTURE_COMMITS="$(commit abc123abc123 1 'éPS-1 [8.4] Fix it')" \
    bash "${SCRIPT}" 2>&1
}
if [[ "$(run_locale C)" == "$(run_locale C.utf8)" ]]; then
  echo "ok   output is byte-identical across locales"
  pass=$((pass + 1))
else
  echo "FAIL output differs across locales"
  fail=$((fail + 1))
fi

# ------------------------------------------------------------ real-repo sweep
section 'wrong tag versus missing tag, from the sweep of merged pull requests'

# A forward port that kept the lower branch tag on its commit.
CASE_BASE=9.7 CASE_TITLE='PS-11291 [9.7] Telemetry tests failing on MacOS' CASE_TOTAL=1
CASE_COMMITS="$(commit abc123abc123 1 'PS-11291 [8.4] Telemetry tests failing on MacOS')"
run_case 'a forward-ported [8.4] commit on 9.7 is told to use [9.7]' 1 'subject carries \[8.4\] but a pull request to 9.7 needs \[9.7\]'

# The title is reported the same way.
CASE_BASE=trunk CASE_TITLE='PS-11202 [10.x]: Handle corrupted page-tracking groups' CASE_TOTAL=1
CASE_COMMITS="$(commit abc123abc123 1 'PS-11202 [trunk]: Handle corrupted page-tracking groups')"
run_case 'a [10.x] title on trunk is told to use [trunk]' 1 'PR title carries \[10.x\] but a pull request to trunk needs \[trunk\]'

# Several wrong tags in one pull request each get their own line.
CASE_BASE=trunk CASE_TITLE='PS-10999 [trunk] OIDC authentication' CASE_TOTAL=3
CASE_COMMITS="$(commit abc123abc123 1 'PS-11248 [8.4]: External roles never revoked'
                commit def456def456 1 'PS-10999 [9.7]: OIDC Authentication'
                commit 789789789789 1 'PS-11413 [8.4] Improve the json error handling')"
out="$(PR_NUMBER=1 REPO=percona/percona-server \
  FIXTURE_META="trunk${US}3${US}${CASE_TITLE}" FIXTURE_COMMITS="${CASE_COMMITS}" \
  bash "${SCRIPT}" 2>&1)" || true
if [[ "$(grep -c 'carries \[8.4\]' <<<"${out}")" -eq 2 && "$(grep -c 'carries \[9.7\]' <<<"${out}")" -eq 1 ]]; then
  echo "ok   each wrong-tagged commit names its own tag"
  pass=$((pass + 1))
else
  echo "FAIL each wrong-tagged commit names its own tag"
  indent "${out}"
  fail=$((fail + 1))
fi

# No tag at all keeps the plain wording.
CASE_BASE=8.4 CASE_TITLE='PS-11238 [8.4] debian package postinst inconsistency' CASE_TOTAL=1
CASE_COMMITS="$(commit abc123abc123 1 'PS-11238 debian package postinst inconsistency')"
run_case 'a subject with no tag is told it lacks one' 1 'subject lacks the \[8.4\] branch tag'

# A bracket that is not a version tag is not reported as one.
CASE_BASE=8.4 CASE_TITLE='PS-1 [8.4] upstream fix' CASE_TOTAL=1
CASE_COMMITS="$(commit abc123abc123 1 '[upstream] PS-1 fix')"
run_case '[upstream] is not mistaken for a wrong version tag' 1 'subject lacks the \[8.4\] branch tag'

# A parenthesised branch in the title is not the tag.
CASE_BASE=9.7 CASE_TITLE='(9.7)PS-11216: Timestamps needed in the GCS_DEBUG_TRACE file' CASE_TOTAL=1
CASE_COMMITS="$(commit abc123abc123 1 'PS-11216: Timestamps needed in the GCS_DEBUG_TRACE file')"
run_case '(9.7) in parentheses is not the tag' 1 'PR title lacks the \[9.7\] branch tag'

# A branch-name-derived title has no key.
CASE_BASE=9.7 CASE_TITLE='Ps 10999 9.7 OIDC authentication' CASE_TOTAL=1
CASE_COMMITS="$(commit abc123abc123 1 'PS-10999 [9.7]: OIDC Authentication')"
run_case 'a branch-name title like "Ps 10999" has no key' 1 'PR title lacks a ticket key'

# ---------------------------------------------------------------- happy paths
section 'conforming pull requests'

CASE_BASE=8.4 CASE_TITLE='DISTMYSQL-645 [8.4] Check commit subjects' CASE_TOTAL=1
CASE_COMMITS="$(commit abc123abc123 1 'DISTMYSQL-645 [8.4] Check commit subjects')"
run_case 'single conforming commit passes' 0 'carry a ticket key'

CASE_BASE=8.4 CASE_TITLE='PS-1 [8.4] Two commits' CASE_TOTAL=2
CASE_COMMITS="$(commit abc123abc123 1 'PS-1 [8.4] one'; commit def456def456 1 'PS-1 [8.4] two')"
run_case 'multiple conforming commits pass' 0 'carry a ticket key'

for key in PS PXB PXC DISTMYSQL; do
  CASE_BASE=8.0 CASE_TITLE="${key}-42 [8.0] Fix it" CASE_TOTAL=1
  CASE_COMMITS="$(commit abc123abc123 1 "${key}-42 [8.0] Fix it")"
  run_case "${key} key accepted" 0 'carry a ticket key'
done

CASE_BASE=9.7 CASE_TITLE='PS-1 [9.7] Fix it' CASE_TOTAL=1
CASE_COMMITS="$(commit abc123abc123 1 'PS-1 [9.7] Fix it')"
run_case '9.7 base accepted' 0 'carry a ticket key'

CASE_BASE=8.4 CASE_TITLE='PS-1 Fix the thing [8.4]' CASE_TOTAL=1
CASE_COMMITS="$(commit abc123abc123 1 'PS-1 Fix the thing [8.4]')"
run_case 'tag anywhere in the subject is accepted' 0 'carry a ticket key'

# ------------------------------------------------------------------ out of scope
section 'out of scope'

CASE_BASE=release-8.4.0 CASE_TITLE='anything at all' CASE_TOTAL=1
CASE_COMMITS="$(commit abc123abc123 1 'no key no tag')"
run_case 'non-version base skips' 0 'not a version branch'

CASE_BASE=8.4 CASE_TITLE='Merge remote-tracking branch' CASE_TOTAL=2
CASE_COMMITS="$(commit abc123abc123 1 'upstream commit with no key'; commit def456def456 2 'Merge branch 8.0 into 8.4')"
run_case 'a merge commit skips the PR' 0 'contains a merge commit'

CASE_BASE=8.4 CASE_TITLE='PS-1 [8.4] not a merge by title' CASE_TOTAL=1
CASE_COMMITS="$(commit abc123abc123 2 'Merge branch 8.0 into 8.4')"
run_case 'merge commit skips regardless of title' 0 'contains a merge commit'

CASE_BASE=8.4 CASE_TITLE='Merge three branches' CASE_TOTAL=1
CASE_COMMITS="$(commit abc123abc123 3 'Merge branches a, b and c')"
run_case 'octopus merge skips' 0 'contains a merge commit'

# A real null merge runs to hundreds of commits, so the exemption comes first.
CASE_BASE=8.4 CASE_TITLE="Null-merge branch '8.0' (after 8.0.40) into '8.4'" CASE_TOTAL=300
CASE_COMMITS="$(commit abc123abc123 1 'Bug#36302624 upstream subject'; commit def456def456 2 'Merge branch 8.0')"
run_case 'a 300-commit null merge is exempt, not a truncation failure' 0 'contains a merge commit'

# ------------------------------------------------------------------ near misses
section 'tag and key near misses'

CASE_BASE=8.4 CASE_TITLE='PS-1 [8.4.0] Fix it' CASE_TOTAL=1
CASE_COMMITS="$(commit abc123abc123 1 'PS-1 [8.4.0] Fix it')"
run_case '[8.4.0] is not [8.4]' 1 'lacks the \[8.4\] branch tag'

CASE_BASE=8.4 CASE_TITLE='PS-1 8.4 Fix it' CASE_TOTAL=1
CASE_COMMITS="$(commit abc123abc123 1 'PS-1 8.4 Fix it')"
run_case 'bare 8.4 without brackets is not the tag' 1 'lacks the \[8.4\] branch tag'

CASE_BASE=8.4 CASE_TITLE='PS-1 [8.0] Wrong branch tag' CASE_TOTAL=1
CASE_COMMITS="$(commit abc123abc123 1 'PS-1 [8.0] Wrong branch tag')"
run_case 'the wrong version tag is rejected' 1 'carries \[8.0\] but a pull request to 8.4 needs \[8.4\]'

CASE_BASE=8.4 CASE_TITLE='XPS-123 [8.4] Add feature' CASE_TOTAL=1
CASE_COMMITS="$(commit abc123abc123 1 'XPS-123 [8.4] Add feature')"
run_case 'XPS-123 is not a ticket key' 1 'lacks a ticket key'

CASE_BASE=8.4 CASE_TITLE='PS-123x [8.4] Add feature' CASE_TOTAL=1
CASE_COMMITS="$(commit abc123abc123 1 'PS-123x [8.4] Add feature')"
run_case 'PS-123x is not a ticket key' 1 'lacks a ticket key'

CASE_BASE=8.4 CASE_TITLE='PS- [8.4] Add feature' CASE_TOTAL=1
CASE_COMMITS="$(commit abc123abc123 1 'PS- [8.4] Add feature')"
run_case 'PS- with no number is not a ticket key' 1 'lacks a ticket key'

CASE_BASE=8.4 CASE_TITLE='(PS-123) [8.4] Add feature' CASE_TOTAL=1
CASE_COMMITS="$(commit abc123abc123 1 '(PS-123) [8.4] Add feature')"
run_case 'a key in parentheses is still a key' 0 'carry a ticket key'

CASE_BASE=8.4 CASE_TITLE='ps-123 [8.4] Add feature' CASE_TOTAL=1
CASE_COMMITS="$(commit abc123abc123 1 'ps-123 [8.4] Add feature')"
run_case 'a lowercase key is rejected' 1 'lacks a ticket key'

# ------------------------------------------------------------------ counting
section 'commit counting and API failures'

CASE_BASE=8.4 CASE_TITLE='PS-1 [8.4] Big' CASE_TOTAL=300
CASE_COMMITS="$(commit abc123abc123 1 'PS-1 [8.4] one')"
run_case 'a truncated listing fails loudly' 1 'only 1 could be listed'

CASE_BASE=8.4 CASE_TITLE='PS-1 [8.4] Empty' CASE_TOTAL=0 CASE_COMMITS=''
run_case 'an empty commit listing fails' 1 'no commits found'

CASE_BASE=8.4 CASE_TITLE='PS-1 [8.4] Null' CASE_TOTAL='null'
CASE_COMMITS="$(commit abc123abc123 1 'PS-1 [8.4] one')"
run_case 'a non-numeric commit count fails' 1 'could not read the base branch'

CASE_BASE=8.4 CASE_TITLE='PS-1 [8.4] Boom' CASE_TOTAL=1
CASE_COMMITS="$(commit abc123abc123 1 'PS-1 [8.4] one')" CASE_RC=1
run_case 'a failing gh call aborts, it does not pass' 1 ''
CASE_RC=0

# ------------------------------------------------------------------ odd subjects
section 'odd subjects'

CASE_BASE=8.4 CASE_TITLE='PS-1 [8.4] Tabbed' CASE_TOTAL=1
CASE_COMMITS="$(commit abc123abc123 1 $'PS-1 [8.4]\ttabbed subject')"
run_case 'a tab in the subject survives the split' 0 'carry a ticket key'

CASE_BASE=8.4 CASE_TITLE=$'PS-1 [8.4]\ttabbed title' CASE_TOTAL=1
CASE_COMMITS="$(commit abc123abc123 1 'PS-1 [8.4] one')"
run_case 'a tab in the title survives the metadata split' 0 'carry a ticket key'

CASE_BASE=8.4 CASE_TITLE='PS-1 [8.4] Quotes and globs' CASE_TOTAL=1
# shellcheck disable=SC2016  # the literal $VARS is what this case feeds the checker
CASE_COMMITS="$(commit abc123abc123 1 'PS-1 [8.4] handle *.cc and "quoted" $VARS')"
run_case 'globs, quotes and dollars in a subject are literal' 0 'carry a ticket key'

CASE_BASE=8.4 CASE_TITLE='PS-1 [8.4] Backslash' CASE_TOTAL=1
CASE_COMMITS="$(commit abc123abc123 1 'PS-1 [8.4] path C:\temp\new')"
run_case 'backslashes are not interpreted' 0 'carry a ticket key'

CASE_BASE=8.4 CASE_TITLE='PS-1 [8.4] Empty subject' CASE_TOTAL=1
CASE_COMMITS="$(commit abc123abc123 1 '')"
run_case 'an empty commit subject fails' 1 'subject lacks a ticket key'

CASE_BASE=8.4 CASE_TITLE='PS-1 [8.4] Leading spaces' CASE_TOTAL=1
CASE_COMMITS="$(commit abc123abc123 1 '   PS-1 [8.4] indented subject')"
run_case 'a subject with leading spaces still matches' 0 'carry a ticket key'

CASE_BASE=8.4 CASE_TITLE='no key no tag' CASE_TOTAL=2
CASE_COMMITS="$(commit abc123abc123 1 'first bad'; commit def456def456 1 'PS-1 second untagged')"
out="$(PR_NUMBER=1 REPO=percona/percona-server \
  FIXTURE_META="8.4${US}2${US}${CASE_TITLE}" FIXTURE_COMMITS="${CASE_COMMITS}" \
  bash "${SCRIPT}" 2>&1)" || true
n_errors="$(grep -c '::error::' <<<"${out}")"
if [[ "${n_errors}" -eq 4 ]]; then
  echo "ok   every violation is reported (4 ::error:: lines)"
  pass=$((pass + 1))
else
  echo "FAIL every violation is reported: ${n_errors} ::error:: lines, want 4"
  indent "${out}"
  fail=$((fail + 1))
fi

# ------------------------------------------------------------------ inputs
section 'required inputs'

out="$(env PR_NUMBER= REPO=percona/percona-server FIXTURE_META="8.4${US}1${US}x" \
  FIXTURE_COMMITS=x bash "${SCRIPT}" 2>&1)"
rc=$?
if [[ "${rc}" -ne 0 ]] && grep -q 'PR_NUMBER is required' <<<"${out}"; then
  echo "ok   missing PR_NUMBER aborts with a clear message"
  pass=$((pass + 1))
else
  echo "FAIL missing PR_NUMBER: exit ${rc}"
  indent "${out}"
  fail=$((fail + 1))
fi

rm -rf "${BIN}"
echo
echo "---"
echo "pass ${pass}  fail ${fail}"
[[ "${fail}" -eq 0 ]]
