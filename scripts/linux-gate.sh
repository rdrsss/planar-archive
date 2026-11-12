#!/usr/bin/env bash
# linux-gate.sh <out-dir> — run the Linux gate image and export its evidence.
#
# Usage: scripts/linux-gate.sh <out-dir>
#   <out-dir> must NOT exist; it is created, and it must lie outside the checkout.
#   Exit 2: usage or refusal. Exit 3: under 15 GiB free before the build.
#   Exit 4: another run holds the lock. Exit 5: docker missing or daemon down.
#   Otherwise the exit status of `docker build` is returned.
#
# Steps (DECISIONS rows 5, 9 and 10c):
#   1. Check free disk on /System/Volumes/Data (override PLANAR_GATE_DISK_PATH;
#      falls back to /). Refuse below 15 GiB.
#   2. Record docker usage and images in docker-before.txt.
#   3. Write host-manifest.txt: sorted "sha256  path" of every file the image
#      context will hold (git ls-files tracked plus untracked-unignored), minus the
#      dockerignore exclusions.
#   4. Run exactly one `docker build --target evidence --output type=local` from
#      the repository root, with no image tag and no bind mount. Log and exit
#      status go to docker-build.log and docker-build.exit.
#   5. Write host-manifest-after.txt and record equal/unequal in manifest-compare.txt,
#      comparing it with host-manifest.txt and container/context-manifest.txt.
#   6. Run `docker builder prune -f` only (never system prune, -a or image removal),
#      then record df and docker usage after the run.
#
# Building needs network access to apt.llvm.org, github.com (Kitware CMake) and the
# ubuntu base image registry. Only linux/arm64 is covered; x86_64 is not covered
# (a recorded limit). This runner targets macOS (df -g -P, shasum).

set -u

usage() {
  echo "usage: scripts/linux-gate.sh <out-dir>" >&2
  echo "  <out-dir> must not exist and must lie outside the checkout." >&2
}

if [ "$#" -ne 1 ] || [ -z "$1" ]; then
  usage
  exit 2
fi
case "$1" in
  -*) usage; exit 2 ;;
esac

REPO="$(cd "$(dirname "$0")/.." && pwd -P)"

OUT_PARENT="$(dirname -- "$1")"
OUT_NAME="$(basename -- "$1")"
if [ ! -d "$OUT_PARENT" ]; then
  echo "linux-gate: parent of out-dir does not exist: $OUT_PARENT" >&2
  exit 2
fi
OUT="$(cd -- "$OUT_PARENT" && pwd -P)/$OUT_NAME"
if [ -e "$OUT" ] || [ -L "$OUT" ]; then
  echo "linux-gate: out-dir must be a NEW directory, but it already exists: $OUT" >&2
  exit 2
fi
case "$OUT" in
  "$REPO"|"$REPO"/*)
    echo "linux-gate: out-dir must lie outside the checkout: $OUT" >&2
    exit 2 ;;
esac

# One gate build at a time (DECISIONS row 5). The lock is a directory so mkdir is atomic.
LOCK_DIR="${PLANAR_GATE_LOCK:-${TMPDIR:-/tmp}/planar-linux-gate.lock}"
if ! mkdir "$LOCK_DIR" 2>/dev/null; then
  echo "linux-gate: another run holds $LOCK_DIR; remove it only after confirming no docker build is running" >&2
  exit 4
fi
trap 'rmdir "$LOCK_DIR" 2>/dev/null' EXIT

# Disk safety: at least 15 GiB free on the data volume before each build.
DISK_PATH="${PLANAR_GATE_DISK_PATH:-/System/Volumes/Data}"
if [ ! -e "$DISK_PATH" ]; then
  DISK_PATH=/
fi
free_gib="$(df -g -P "$DISK_PATH" 2>/dev/null | awk 'NR == 2 { print $4 }')"
case "$free_gib" in
  ''|*[!0-9]*)
    echo "linux-gate: cannot read free space on $DISK_PATH" >&2
    exit 3 ;;
esac
if [ "$free_gib" -lt 15 ]; then
  echo "linux-gate: only ${free_gib} GiB free on $DISK_PATH; 15 GiB is required before a build" >&2
  exit 3
fi

if ! docker info >/dev/null 2>&1; then
  echo "linux-gate: docker is unavailable or its daemon is not running" >&2
  exit 5
fi

if ! mkdir -- "$OUT"; then
  echo "linux-gate: cannot create $OUT" >&2
  exit 2
fi

# Mirror of scripts/linux-gate.Dockerfile.dockerignore. Keep the two lists in step.
# Succeeds (returns 0) when the repository-relative path is excluded from the context.
is_excluded() {
  local rel="$1"
  local p="/$rel"
  case "$p" in
    /.git|/.git/*|/build|/build/*|/bin|/bin/*|/dist|/dist/*) return 0 ;;
    /doxygen-lint-output|/doxygen-lint-output/*|/compile_commands.json) return 0 ;;
    /.claude|/.claude/*|/.codex|/.codex/*|/.copilot|/.copilot/*) return 0 ;;
    /.cache|/.cache/*|/.pytest_cache|/.pytest_cache/*|/.worktrees|/.worktrees/*) return 0 ;;
    /.idea|/.idea/*|/.vscode|/.vscode/*) return 0 ;;
    /tests/native/[!/]*-evidence/*) return 0 ;;
    /docs/vernacular/rounds/[!/]*-evidence|/docs/vernacular/rounds/[!/]*-evidence/*) return 0 ;;
    */worktrees|*/worktrees/*|*/__pycache__|*/__pycache__/*|*.pyc) return 0 ;;
    */.DS_Store|*.swp|*.swo|*.user|*.o|*.obj|*.a|*.lib) return 0 ;;
    *.db|*.db-journal|*.db-wal|*.db-shm) return 0 ;;
  esac
  return 1
}

# Writes "sha256  path" lines, sorted bytewise by path, for the image context.
write_manifest() {
  local dest="$1"
  local list
  list="$(mktemp "${TMPDIR:-/tmp}/planar-gate-list.XXXXXX")" || return 1
  git -C "$REPO" ls-files -z --cached --others --exclude-standard > "$list"
  LC_ALL=C sort -z -u "$list" > "$list.sorted"
  : > "$dest"
  local p
  while IFS= read -r -d '' p; do
    if is_excluded "$p"; then
      continue
    fi
    if [ ! -f "$REPO/$p" ]; then
      continue
    fi
    (cd "$REPO" && shasum -a 256 -- "$p") >> "$dest"
  done < "$list.sorted"
  rm -f "$list" "$list.sorted"
}

df -g -P "$DISK_PATH" > "$OUT/df-before.txt" 2>&1
docker system df > "$OUT/docker-before.txt" 2>&1
{
  echo "## docker images --digests"
  docker images --digests
} >> "$OUT/docker-before.txt" 2>&1

write_manifest "$OUT/host-manifest.txt"

cd "$REPO" || exit 2
docker build --progress=plain -f scripts/linux-gate.Dockerfile --target evidence \
  --output type=local,dest="$OUT/container" . > "$OUT/docker-build.log" 2>&1
build_status=$?
echo "$build_status" > "$OUT/docker-build.exit"

write_manifest "$OUT/host-manifest-after.txt"
if cmp -s "$OUT/host-manifest.txt" "$OUT/host-manifest-after.txt"; then
  before_after="equal"
else
  before_after="unequal"
fi
if [ ! -f "$OUT/container/context-manifest.txt" ]; then
  context_vs_host="missing-container-manifest"
elif cmp -s "$OUT/host-manifest.txt" "$OUT/container/context-manifest.txt"; then
  context_vs_host="equal"
else
  context_vs_host="unequal"
fi
{
  echo "host manifest before vs after build: $before_after"
  echo "host manifest vs container context manifest: $context_vs_host"
} > "$OUT/manifest-compare.txt"

docker builder prune -f > "$OUT/builder-prune.log" 2>&1
echo $? > "$OUT/builder-prune.exit"

df -g -P "$DISK_PATH" > "$OUT/df-after.txt" 2>&1
docker system df > "$OUT/docker-after.txt" 2>&1
{
  echo "## docker images --digests"
  docker images --digests
} >> "$OUT/docker-after.txt" 2>&1

printf '%s\n' "$OUT"
exit "$build_status"
