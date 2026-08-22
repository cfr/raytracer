#!/bin/bash
# clang-tidy static analysis (config: ci/.clang-tidy).
#
#   ci/lint.sh          # report findings
#   ci/lint.sh --fix    # apply fixits, report what remains
#
#   docker build -t raytracer-ci ci/
#   docker run --rm -u "$(id -u):$(id -g)" -v "$PWD":/repo raytracer-ci ci/lint.sh
set -euo pipefail
cd "$(dirname "$0")/.."

BUILD="build/lint-$(uname -s)"
FETCH_ARGS=()
[ -d /opt/glm ] && FETCH_ARGS+=(-DFETCHCONTENT_SOURCE_DIR_GLM=/opt/glm)
[ -d /opt/pcg-cpp ] && FETCH_ARGS+=(-DFETCHCONTENT_SOURCE_DIR_PCG=/opt/pcg-cpp)

cmake -S . -B "$BUILD" -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug \
    -Wno-deprecated --no-warn-unused-cli \
    ${FETCH_ARGS[@]+"${FETCH_ARGS[@]}"} > /dev/null

ARGS=()
if [ "${1:-}" = "--fix" ]; then
    ARGS+=(-fix -format -style "file:ci/.clang-format")
fi

set +e
out=$(run-clang-tidy -p "$BUILD" -quiet -config-file ci/.clang-tidy \
    ${ARGS[@]+"${ARGS[@]}"} '(^|/)src/.*\.(cpp|cc|cxx)$' 2>&1)
rc=$?
set -e

n=$(grep -cE ':[0-9]+:[0-9]+: (warning|error): .*\[[^]]+\]' <<<"$out" || true)

if [ "$rc" -gt 1 ]; then
    printf '%s\n' "$out" >&2
    echo "lint: ERROR (run-clang-tidy exited $rc)" >&2
    exit 2
fi
if [ "$n" -gt 0 ]; then
    grep -E ':[0-9]+:[0-9]+: (warning|error): ' <<<"$out" || true
    echo "-- clang-tidy: $n finding(s)"
    echo "lint: FAILED ($n finding(s))"
    exit 1
fi
if [ "$rc" -ne 0 ]; then
    printf '%s\n' "$out" >&2
    echo "lint: ERROR (run-clang-tidy exited $rc with no parseable findings)" >&2
    exit 2
fi
echo "lint: OK"
