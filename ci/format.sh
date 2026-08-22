#!/bin/bash
# clang-format check (config: ci/.clang-format).
#
#   ci/format.sh          # report what's unformatted (CI mode)
#   ci/format.sh --fix    # reformat files in place
#
#   docker build -t raytracer-ci ci/
#   docker run --rm -u "$(id -u):$(id -g)" -v "$PWD":/repo raytracer-ci ci/format.sh
set -euo pipefail
cd "$(dirname "$0")/.."

export GIT_CONFIG_COUNT=1 GIT_CONFIG_KEY_0=safe.directory GIT_CONFIG_VALUE_0="$PWD"

list_files() {
    git ls-files -z --cached --others --exclude-standard '*.cpp' '*.cc' '*.cxx' '*.hpp' '*.hh' '*.h'
}

STYLE="file:ci/.clang-format"

if [ "${1:-}" = "--fix" ]; then
    list_files | xargs -0 -r clang-format --style="$STYLE" -i
    echo "format: done"
    exit 0
fi

set +e
out=$(list_files | xargs -0 -r clang-format --style="$STYLE" --dry-run --Werror 2>&1)
rc=$?
set -e

n=$(grep -c 'error:.*\[-Wclang-format-violations\]$' <<<"$out" || true)

if [ "$rc" -gt 1 ] && [ "$n" -eq 0 ]; then
    printf '%s\n' "$out" >&2
    echo "format: ERROR (clang-format exited $rc)" >&2
    exit 2
fi

grep 'error:.*\[-Wclang-format-violations\]$' <<<"$out" || true
echo "-- clang-format: $n violation(s)"

if [ "$n" -gt 0 ]; then
    echo "format: FAILED (run ci/format.sh --fix to fix)"
    exit 1
fi
echo "format: OK"
