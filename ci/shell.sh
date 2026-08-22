#!/bin/bash
# Lint the ci shell scripts with shellcheck.
#
#   ci/shell.sh

set -euo pipefail
cd "$(dirname "$0")/.."

set +e
out=$(shellcheck -f gcc ci/*.sh 2>&1)
rc=$?
set -e

if [ "$rc" -gt 1 ]; then
    printf '%s\n' "$out" >&2
    echo "shell: ERROR (shellcheck exited $rc)" >&2
    exit 2
fi

n=$(grep -cE ':[0-9]+:[0-9]+: .*SC[0-9]+' <<<"$out" || true)
[ "$n" -gt 0 ] && printf '%s\n' "$out"
echo "-- shellcheck: $n finding(s)"

if [ "$n" -gt 0 ]; then
    echo "shell: FAILED ($n finding(s))"
    exit 1
fi
echo "shell: OK"
