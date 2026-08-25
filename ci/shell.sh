#!/bin/bash
# Lint shell scripts with shellcheck.
#
#   ci/shell.sh                             # every ci/*.sh
#   ci/shell.sh ci/perfstat.sh              # just the named scripts
#

set -euo pipefail
cd "$(dirname "$0")/.."

if ! command -v shellcheck >/dev/null; then
    echo "shell: needs shellcheck (not in the ci image -- install it locally)" >&2
    exit 2
fi

if [ "$#" -eq 0 ]; then
    FILES=(ci/*.sh)
else
    FILES=("$@")
fi

set +e
out=$(shellcheck -f gcc "${FILES[@]}" 2>&1)
rc=$?
set -e

if [ "$rc" -gt 1 ]; then
    printf '%s\n' "$out" >&2
    echo "shell: ERROR (shellcheck exited $rc)" >&2
    exit 2
fi

n=$(grep -cE ':[0-9]+:[0-9]+: .*SC[0-9]+' <<<"$out" || true)
[ "$n" -gt 0 ] && printf '%s\n' "$out"
echo "-- shellcheck: $n finding(s) in ${#FILES[@]} file(s)"

if [ "$n" -gt 0 ]; then
    echo "shell: FAILED ($n finding(s))"
    exit 1
fi
echo "shell: OK"
