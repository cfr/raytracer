#!/bin/bash
# Build the raytracer.
#
#   ci/build.sh                     # build repo root into build/ci-<os>-<arch>
#   ci/build.sh -s DIR -b DIR       # build another source tree elsewhere
#   ci/build.sh --native            # add -march=native (NOT reproducible)
#

set -euo pipefail
cd "$(dirname "$0")/.."

SRC="$PWD"
BUILD=""
TYPE="RelWithDebInfo"
FLAGS="-O2 -DNDEBUG"

while [ $# -gt 0 ]; do
    case "$1" in
    -s | --source)
        SRC="$2"
        shift 2
        ;;
    -b | --build)
        BUILD="$2"
        shift 2
        ;;
    --native)
        TYPE="Release"
        FLAGS=""
        shift
        ;;
    *)
        echo "build: unknown argument '$1'" >&2
        exit 2
        ;;
    esac
done

: "${BUILD:=build/ci-$(uname -s)-$(uname -m)}"

FETCH_ARGS=()
[ -d /opt/glm ] && FETCH_ARGS+=(-DFETCHCONTENT_SOURCE_DIR_GLM=/opt/glm)
[ -d /opt/pcg-cpp ] && FETCH_ARGS+=(-DFETCHCONTENT_SOURCE_DIR_PCG=/opt/pcg-cpp)

TYPE_ARGS=(-DCMAKE_BUILD_TYPE="$TYPE")
[ -n "$FLAGS" ] && TYPE_ARGS+=(-DCMAKE_CXX_FLAGS_RELWITHDEBINFO="$FLAGS")

cmake -S "$SRC" -B "$BUILD" -G Ninja \
    "${TYPE_ARGS[@]}" \
    -Wno-deprecated --no-warn-unused-cli \
    ${FETCH_ARGS[@]+"${FETCH_ARGS[@]}"} >/dev/null

cmake --build "$BUILD" --parallel >/dev/null

echo "build: OK ($TYPE) -> $BUILD/raytracer"
