#!/usr/bin/env bash
# ignored_context_check.sh <runner> <repo>
# Lists every git-ignored untracked file in <repo> that the is_excluded function of
# <runner> (scripts/linux-gate.sh) does NOT exclude, then prints "count: N".
# The function is cut out of the runner and sourced; the runner is never executed.
set -u
if [ "$#" -ne 2 ]; then
  echo "usage: ignored_context_check.sh <runner> <repo>" >&2
  exit 2
fi
runner="$1"
repo="$2"
scratch="$(mktemp -d "${TMPDIR:-/tmp}/ignored-context.XXXXXX")" || exit 2
trap 'rm -rf "$scratch"' EXIT
sed -n '/^is_excluded()/,/^}/p' "$runner" > "$scratch/isx.sh"
if [ ! -s "$scratch/isx.sh" ]; then
  echo "ignored_context_check: is_excluded not found in $runner" >&2
  exit 2
fi
# shellcheck disable=SC1091
. "$scratch/isx.sh"
count=0
while IFS= read -r -d '' p; do
  if ! is_excluded "$p"; then
    printf '%s\n' "$p"
    count=$((count + 1))
  fi
done < <(git -C "$repo" ls-files -z --others --ignored --exclude-standard)
echo "count: $count"
