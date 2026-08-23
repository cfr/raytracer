#!/bin/bash
# Render the scene set at HEAD and at a base ref, and compare.
#
#   ci/compare.sh                           # images only, against main
#   ci/compare.sh v1.0                      # against any ref
#   ci/compare.sh --perf                    # wall clock (noisy, report only)
#   ci/compare.sh --icount                  # callgrind instruction counts
#   ci/compare.sh --icount --icount-max 2   # max instruction inc percents
#   ci/compare.sh --rmse 0.5 --bias 0.2     # see ppmdiff.py
#
# Requires full history: actions/checkout@v4 with fetch-depth: 0.
#
#   docker build -t aktis-ci ci/
#   docker run --rm -u "$(id -u):$(id -g)" -v "$PWD":/repo aktis-ci ci/compare.sh

set -euo pipefail
cd "$(dirname "$0")/.."

export GIT_CONFIG_COUNT=1 GIT_CONFIG_KEY_0=safe.directory GIT_CONFIG_VALUE_0="$PWD"

BASE=""
PERF=0
ICOUNT=0
ICOUNT_MAX=""
RUNS=3
TOL=()

PERF_ARGS=(--width 320 --spp 32)
PERF_SCENES=(scenes/cornell.test scenes/dragon.test)

while [ $# -gt 0 ]; do
    case "$1" in
    --perf) PERF=1; shift ;;
    --icount) ICOUNT=1; shift ;;
    --icount-max) ICOUNT_MAX="$2"; shift 2 ;;
    --runs) RUNS="$2"; shift 2 ;;
    --rmse | --bias) TOL+=("$1" "$2"); shift 2 ;;
    -*)
        echo "compare: unknown argument '$1'" >&2
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
    echo "compare: ref '$BASE' not found (need fetch-depth: 0 in CI)" >&2
    exit 2
}

if [ "$ICOUNT" -eq 1 ] && ! command -v valgrind >/dev/null; then
    echo "compare: --icount needs valgrind (absent on macOS; use the ci image)" >&2
    exit 2
fi

BASE_SHA=$(git rev-parse --short "$BASE_REF")
HEAD_SHA=$(git rev-parse --short HEAD)

WORK=$(mktemp -d)
TREE="$WORK/base"
cleanup() {
    git worktree remove --force "$TREE" 2>/dev/null || true
    rm -rf "$WORK"
}
trap cleanup EXIT

echo "== building head ($HEAD_SHA)"
ci/build.sh -b "$WORK/build-head"
HEAD_BIN="$WORK/build-head/aktis"

echo "== building base ($BASE $BASE_SHA)"
git worktree add --detach --quiet "$TREE" "$BASE_REF"
ci/build.sh -s "$TREE" -b "$WORK/build-base"
BASE_BIN="$WORK/build-base/aktis"

echo "== rendering"
ci/render.sh "$HEAD_BIN" "$WORK/img-head"
ci/render.sh -C "$TREE" "$BASE_BIN" "$WORK/img-base"

echo "== comparing"
fail=0
compared=0
for img in "$WORK/img-head"/*.ppm; do
    name=$(basename "$img")
    if [ ! -f "$WORK/img-base/$name" ]; then
        echo "new        $name (not present at base)"
        continue
    fi
    compared=$((compared + 1))
    ci/ppmdiff.py ${TOL[@]+"${TOL[@]}"} "$WORK/img-base/$name" "$img" || fail=$((fail + 1))
done

run_wall() {
    local bin="$1" scene="$2" t times=()
    for _ in $(seq "$RUNS"); do
        TIMEFORMAT=%R
        t=$( { time "$bin" --quiet --seed 1 "${PERF_ARGS[@]}" \
            --out "$WORK/perf" "$scene" >/dev/null; } 2>&1 )
        times+=("$t")
    done
    printf '%s\n' "${times[@]}" | sort -n | awk "NR==int(($RUNS+1)/2)"
}

run_icount() {
    local bin="$1" scene="$2" out n
    out=$(valgrind --tool=callgrind --callgrind-out-file=/dev/null \
        "$bin" --quiet --seed 1 --threads 1 \
        "${PERF_ARGS[@]}" --out "$WORK/perf" "$scene" 2>&1 >/dev/null) || true
    n=$(printf '%s\n' "$out" | awk '/I +refs:/ { gsub(/,/, "", $NF); print $NF }' | tail -1)
    if [ -z "$n" ]; then
        printf '%s\n' "$out" | tail -5 >&2
    fi
    printf '%s\n' "$n"
}

perf_report() {
    local label="$1" fn="$2" unit="$3" gate="$4" scene h b
    echo "== $label"
    for scene in "${PERF_SCENES[@]}"; do
        if [ ! -f "$scene" ] || [ ! -f "$TREE/$scene" ]; then
            continue
        fi
        h=$("$fn" "$HEAD_BIN" "$scene")
        b=$("$fn" "$BASE_BIN" "$TREE/$scene")
        if [ -z "$h" ] || [ -z "$b" ] || [ "$b" = "0" ]; then
            echo "$scene: no measurement" >&2
            continue
        fi
        awk -v h="$h" -v b="$b" -v s="$scene" -v u="$unit" -v g="$gate" '
            BEGIN {
                d = (h - b) / b * 100
                printf "%-24s base %12.4g%s  head %12.4g%s  %+6.2f%%\n", s, b, u, h, u, d
                if (g != "" && d > g + 0) exit 1
            }' || fail=$((fail + 1))
    done
}

if [ "$PERF" -eq 1 ]; then
    perf_report "timing (median of $RUNS, wall clock -- noisy, not gated)" run_wall "s" ""
fi
if [ "$ICOUNT" -eq 1 ]; then
    perf_report "instructions (callgrind, single-threaded, deterministic)" \
        run_icount "" "$ICOUNT_MAX"
fi

echo "-- compare: $compared image(s), $fail failure(s)"
if [ "$fail" -gt 0 ]; then
    echo "compare: FAILED ($fail check(s) failed against $BASE)"
    exit 1
fi
echo "compare: OK"
