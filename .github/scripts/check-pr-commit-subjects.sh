#!/usr/bin/env bash
#
# Check that a pull request to a percona-server version branch follows the
# commit-subject convention: the title and every commit subject name a ticket
# key and carry the base branch tag, for example "PS-11435 [8.4] Add ...".
# The tag is what survives the upward null-merges (8.0 -> 8.4 -> 9.7 -> trunk)
# and what release tooling greps for, so a squash merged under a rewritten
# subject loses the change for both.
#
# Inputs (environment):
#   REPO       owner/name, default percona/percona-server
#   PR_NUMBER  pull request number
#   GH_TOKEN   GitHub API token, read access is enough
#
# The base branch, the title and the commit count are read from the API rather
# than from the triggering event, so re-running an old run judges the pull
# request as it is now instead of as it was.
#
# Exit 0 when the pull request conforms or is out of scope (non-version base
# branch, or a pull request that carries a merge commit). Exit 1 with one
# ::error:: line per violation otherwise.

# has_key is a predicate, so it runs inside an if condition on purpose.
# shellcheck disable=SC2310
set -euo pipefail

# A ticket key with word boundaries. LC_ALL=C at the call site keeps [:alnum:]
# meaning ASCII, so the runner's locale cannot change the verdict.
readonly KEY_RE='(^|[^[:alnum:]_])(PS|PXB|PXC|DISTMYSQL)-[0-9]+([^[:alnum:]_]|$)'
readonly REPO="${REPO:-percona/percona-server}"
: "${PR_NUMBER:?PR_NUMBER is required}"

# The tag a PR to this base branch must carry, empty when the base is out of scope.
tag_for_base() {
  case "$1" in
    8.0) echo '[8.0]' ;;
    8.4) echo '[8.4]' ;;
    9.7) echo '[9.7]' ;;
    trunk) echo '[trunk]' ;;
    *) ;;
  esac
}

# Every version tag a subject might carry. [10.x] is listed so a trunk change
# still tagged that way is told what replaced it.
readonly -a VERSION_TAGS=('[8.0]' '[8.4]' '[9.7]' '[trunk]' '[10.x]')

# Prints the version tag a subject carries when it is not the expected one, so
# a forward port still tagged [8.4] or a trunk change still tagged [10.x] is
# told what to replace rather than what it lacks. Prints nothing otherwise.
other_tag() {
  local subject="$1" expected="$2" candidate
  for candidate in "${VERSION_TAGS[@]}"; do
    if [[ "${candidate}" != "${expected}" && "${subject}" == *"${candidate}"* ]]; then
      printf '%s' "${candidate}"
      return
    fi
  done
}

# A workflow command ends at a carriage return or a newline and is percent
# decoded, so a commit subject would otherwise be able to open one of its own.
escape_annotation() {
  local text="$1"
  text="${text//'%'/%25}"
  text="${text//$'\r'/%0D}"
  text="${text//$'\n'/%0A}"
  printf '%s' "${text}"
}

note() { printf '::notice::%s\n' "$(escape_annotation "$*")"; }
warn() { printf '::warning::%s\n' "$(escape_annotation "$*")"; }
fail() { printf '::error::%s\n' "$(escape_annotation "$*")"; }

has_key() { LC_ALL=C grep -Eq "${KEY_RE}" <<<"$1"; }

# Reports a title or subject that does not carry the expected tag.
fail_tag() {
  local what="$1" subject="$2" base_ref="$3" tag="$4" found
  found="$(other_tag "${subject}" "${tag}")"
  if [[ -n "${found}" ]]; then
    fail "${what} carries ${found} but a pull request to ${base_ref} needs ${tag}: ${subject}"
  else
    fail "${what} lacks the ${tag} branch tag: ${subject}"
  fi
}

main() {
  # One call for the three pieces of metadata, unit-separator joined because a
  # title may hold anything else.
  local meta
  meta="$(gh api "repos/${REPO}/pulls/${PR_NUMBER}" \
    --jq '[.base.ref, (.commits | tostring), .title] | join("\u001f")')"
  if [[ -z "${meta}" ]]; then
    fail "could not read pull request ${PR_NUMBER} from ${REPO}"
    return 1
  fi
  local base_ref total title
  IFS=$'\x1f' read -r base_ref total title <<<"${meta}"
  if [[ -z "${base_ref}" || ! "${total}" =~ ^[0-9]+$ ]]; then
    fail "could not read the base branch and commit count of PR ${PR_NUMBER}, got: ${meta//$'\x1f'/ | }"
    return 1
  fi

  local tag
  tag="$(tag_for_base "${base_ref}")"
  if [[ -z "${tag}" ]]; then
    note "base branch ${base_ref} is not a version branch, nothing to check"
    return 0
  fi
  local expected="<KEY>-<n> ${tag} <what changed>"

  # One line per commit, unit-separator delimited so tabs in a subject survive:
  # sha<US>parent count<US>subject.
  local commits
  commits="$(gh api --paginate "repos/${REPO}/pulls/${PR_NUMBER}/commits" \
    --jq '.[] | "\(.sha[0:12])\u001f\(.parents | length)\u001f\(.commit.message | split("\n")[0])"')"
  if [[ -z "${commits}" ]]; then
    fail "no commits found on PR ${PR_NUMBER}"
    return 1
  fi

  # A merge commit is what marks an upstream merge or a null merge, whose
  # subjects come from another branch and are not this convention's to enforce.
  # The commit graph decides that, never the title: fewer than half of the
  # merged null-merge pull requests in this repository open with "Merge" or
  # "Null-merge", so a title pattern would reject the real ones, while a merge
  # commit is the one mark every one of them carries. The cost is that merging
  # the base branch into a working branch also exempts a pull request, so the
  # exemption is a warning rather than a silent pass.
  if awk -F$'\x1f' '$2 > 1 { found = 1 } END { exit !found }' <<<"${commits}"; then
    warn "PR contains a merge commit (upstream or null merge), subjects not checked"
    return 0
  fi

  # The listing endpoint stops at 250 commits, so compare against the PR's own
  # count. Checked after the merge exemption because a null merge legitimately
  # runs to hundreds of commits.
  local fetched
  fetched="$(wc -l <<<"${commits}")"
  if (( fetched < 10#${total} )); then
    fail "PR has ${total} commits but only ${fetched} could be listed, split it or check the subjects by hand"
    return 1
  fi

  local violations=0
  # A squash merge of several commits takes the PR title as its subject, and the
  # title is the convention's own record either way, so it is checked too.
  if ! has_key "${title}"; then
    fail "PR title lacks a ticket key (PS-<n>, PXB-<n>, PXC-<n> or DISTMYSQL-<n>): ${title}"
    violations=$((violations + 1))
  fi
  if [[ "${title}" != *"${tag}"* ]]; then
    fail_tag "PR title" "${title}" "${base_ref}" "${tag}"
    violations=$((violations + 1))
  fi

  local sha subject
  while IFS=$'\x1f' read -r sha _ subject; do
    # A revert keeps the reverted subject verbatim, and reverting a subject that
    # broke this convention is exactly how the convention gets repaired, so a
    # revert cannot be required to conform. Announced, so it is visible in review.
    if [[ "${subject}" == 'Revert "'* ]]; then
      note "${sha}: revert, subject checks skipped: ${subject}"
      continue
    fi
    if ! has_key "${subject}"; then
      fail "${sha}: subject lacks a ticket key: ${subject}"
      violations=$((violations + 1))
      continue
    fi
    if [[ "${subject}" != *"${tag}"* ]]; then
      fail_tag "${sha}: subject" "${subject}" "${base_ref}" "${tag}"
      violations=$((violations + 1))
    fi
  done <<<"${commits}"

  if (( violations > 0 )); then
    echo
    echo "Expected subject form: ${expected}"
    echo "Reword the commits (git rebase -i, reword) and force-push the PR branch."
    return 1
  fi
  note "PR title and every commit subject carry a ticket key and the ${tag} tag"
}

main "$@"
