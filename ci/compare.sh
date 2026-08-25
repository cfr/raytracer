#!/bin/bash
# Render the scene set at HEAD and at a base ref, and compare the images.
#
#   ci/compare.sh                           # against main
#   ci/compare.sh --base v1.0               # against any ref
#   ci/compare.sh --rmse 0.5 --bias 0.2     # see ppmdiff.py
#
# Requires full history: actions/checkout@v4 with fetch-depth: 0.
#
#   docker build -t aktis-ci ci/
#   docker run --rm -u "$(id -u):$(id -g)" -v "$PWD":/repo aktis-ci ci/compare.sh

set -euo pipefail
cd "$(dirname "$0")/.."

BASE=""
TOL=()

while [ $# -gt 0 ]; do
    case "$1" in
    --base) BASE="$2"; shift 2 ;;
    --rmse | --bias) TOL+=("$1" "$2"); shift 2 ;;
    -*)
        echo "compare: unknown argument '$1'" >&2
        exit 2
        ;;
    *) BASE="$1"; shift ;;
    esac
done

work=$(mktemp -d)
cleanup() { ci/refbuild.sh --clean "$work" >/dev/null 2>&1 || true; }
trap cleanup EXIT

BUILD_ARGS=(--out "$work")
[ -n "$BASE" ] && BUILD_ARGS+=(--base "$BASE")

base_bin='' head_bin='' base_tree='' base_ref=''
build=$(ci/refbuild.sh "${BUILD_ARGS[@]}") || exit $?
eval "$build"

echo "== rendering"
ci/render.sh "$head_bin" "$work/img-head"
ci/render.sh -C "$base_tree" "$base_bin" "$work/img-base"

echo "== comparing"
fail=0
compared=0
for img in "$work/img-head"/*.ppm; do
    name=$(basename "$img")
    if [ ! -f "$work/img-base/$name" ]; then
        echo "new        $name (not present at base)"
        continue
    fi
    compared=$((compared + 1))
    ci/ppmdiff.py ${TOL[@]+"${TOL[@]}"} "$work/img-base/$name" "$img" || fail=$((fail + 1))
done

echo "-- compare: $compared image(s), $fail failure(s)"
if [ "$fail" -gt 0 ]; then
    echo "compare: FAILED ($fail check(s) failed against ${base_ref})"
    exit 1
fi
echo "compare: OK"
