#!/bin/bash
# Render the regression scene set with a fixed seed.
#
#   ci/render.sh BINARY OUTDIR                # render every scene
#   ci/render.sh BINARY OUTDIR cornell ggx    # render some, by name
#   ci/render.sh -C /tmp/base BIN OUTDIR      # resolve scenes in another tree
#   ci/render.sh -t 1 BIN OUTDIR              # pin thread count
#
set -euo pipefail
cd "$(dirname "$0")/.."

TREE="$PWD"
THREADS=""

while [ $# -gt 0 ]; do
    case "$1" in
    -C | --tree)
        TREE="$2"
        shift 2
        ;;
    -t | --threads)
        THREADS="$2"
        shift 2
        ;;
    -*)
        echo "render: unknown argument '$1'" >&2
        exit 2
        ;;
    *) break ;;
    esac
done

BIN="${1:?usage: ci/render.sh [-C TREE] [-t N] BINARY OUTDIR [scene...]}"
OUT="${2:?usage: ci/render.sh [-C TREE] [-t N] BINARY OUTDIR [scene...]}"
shift 2

SEED="${SEED:-1}"

THREAD_ARGS=()
[ -n "$THREADS" ] && THREAD_ARGS+=(--threads "$THREADS")

# name:scene:extra-args -- one per integrator and shape family.
SCENES=(
    "cornell:scenes/cornell.test:--width 200 --spp 16"
    "cornell-glass:scenes/cornell-glass.test:--width 200 --spp 16"
    "cornell-mis:scenes/cornell-mis.test:--width 200 --spp 16"
    "cornell-whitted:scenes/cornell-whitted.test:--width 200 --spp 1"
    "analytic:scenes/analytic.test:--width 200 --spp 1"
    "ggx:scenes/ggx.test:--width 200 --spp 16"
    "drei-koerper:scenes/drei-koerper.test:--width 200 --spp 8"
    "dragon:scenes/dragon.test:--width 160 --spp 4"
)

mkdir -p "$OUT"
rendered=0
skipped=0

for entry in "${SCENES[@]}"; do
    name="${entry%%:*}"
    rest="${entry#*:}"
    scene="$TREE/${rest%%:*}"
    extra="${rest#*:}"

    if [ $# -gt 0 ]; then
        match=0
        for want in "$@"; do
            [ "$want" = "$name" ] && match=1
        done
        [ "$match" -eq 1 ] || continue
    fi

    if [ ! -f "$scene" ]; then
        echo "render: skip $name (no $scene)"
        skipped=$((skipped + 1))
        continue
    fi

    # shellcheck disable=SC2086  # $extra is a deliberate argument list
    "$BIN" --quiet --seed "$SEED" ${THREAD_ARGS[@]+"${THREAD_ARGS[@]}"} $extra \
        --out "$OUT/$name" "$scene"
    rendered=$((rendered + 1))
done

echo "render: $rendered rendered, $skipped skipped -> $OUT"
