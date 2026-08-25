#!/bin/bash
# Build the working tree and a base ref, for scripts that compare the two.
#
#   eval "$(ci/refbuild.sh)"                # base = $GITHUB_BASE_REF, else main
#   eval "$(ci/refbuild.sh --base v1.0)"
#   eval "$(ci/refbuild.sh --out /tmp/rb)"  # default: a fresh mktemp -d
#   ci/refbuild.sh --clean /tmp/rb          # drop the worktree when done
#
# Requires full history: actions/checkout@v4 with fetch-depth: 0.

set -euo pipefail
cd "$(dirname "$0")/.."

export GIT_CONFIG_COUNT=1 GIT_CONFIG_KEY_0=safe.directory GIT_CONFIG_VALUE_0="$PWD"

BASE=""
OUT=""

if [ "${1:-}" = "--clean" ]; then
    [ -n "${2:-}" ] || { echo "refbuild: --clean needs a directory" >&2; exit 2; }
    git worktree remove --force "$2/base-tree" 2>/dev/null || true
    git worktree prune
    rm -rf "${2:?}"
    exit 0
fi

while [ $# -gt 0 ]; do
    case "$1" in
    --base) BASE="$2"; shift 2 ;;
    --out) OUT="$2"; shift 2 ;;
    -*)
        echo "refbuild: unknown argument '$1'" >&2
        exit 2
        ;;
    *) BASE="$1"; shift ;;
    esac
done

: "${BASE:=${GITHUB_BASE_REF:-main}}"

resolve_ref() {
    local r
    for r in "$1" "origin/$1" "refs/remotes/origin/$1"; do
        if git rev-parse --verify --quiet "$r^{commit}" >/dev/null; then
            printf '%s\n' "$r"
            return 0
        fi
    done
    return 1
}

BASE_REF=$(resolve_ref "$BASE") || {
    echo "refbuild: ref '$BASE' not found (need fetch-depth: 0 in CI)" >&2
    exit 2
}

BASE_SHA=$(git rev-parse --short "$BASE_REF")
HEAD_SHA=$(git rev-parse --short HEAD)

: "${OUT:=$(mktemp -d)}"
mkdir -p "$OUT"
TREE="$OUT/base-tree"

echo "== building head ($HEAD_SHA)" >&2
ci/build.sh -b "$OUT/build-head" >&2

echo "== building base ($BASE $BASE_SHA)" >&2
if [ ! -d "$TREE" ]; then
    git worktree add --detach --quiet "$TREE" "$BASE_REF"
fi
ci/build.sh -s "$TREE" -b "$OUT/build-base" >&2

printf 'base_bin=%q\n'  "$OUT/build-base/aktis"
printf 'head_bin=%q\n'  "$OUT/build-head/aktis"
printf 'base_tree=%q\n' "$TREE"
printf 'work=%q\n'      "$OUT"
printf 'base_sha=%q\n'  "$BASE_SHA"
printf 'head_sha=%q\n'  "$HEAD_SHA"
printf 'base_ref=%q\n'  "$BASE_REF"
