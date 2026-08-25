#!/bin/bash
# Hardware counters and profile breakdown, single-threaded and deterministic.
#
#   ci/perfstat.sh BIN                                  # counters for one build
#   ci/perfstat.sh BASE_BIN HEAD_BIN                    # counter table with deltas
#   ci/perfstat.sh --profile BIN                        # where the time goes, by symbol
#   ci/perfstat.sh --profile BASE HEAD                  # both profiles, base then head
#   ci/perfstat.sh -s dragon BIN                        # one scene (repeatable)
#   ci/perfstat.sh --runs 5 BASE HEAD                   # N runs, with stddev to judge noise
#   ci/perfstat.sh --callgrind BASE HEAD                # simulated, no PMU -- for CI
#   ci/perfstat.sh --callgrind --profile BIN            # profile without a PMU
#   ci/perfstat.sh --base main                          # build both refs and compare
#   ci/perfstat.sh --args "--width 320 --spp 32" BIN
#
#   docker build -t aktis-ci ci/
#   docker run --rm -u "$(id -u):$(id -g)" -v "$PWD":/repo aktis-ci \
#       ci/perfstat.sh --callgrind --base main

set -euo pipefail
cd "$(dirname "$0")/.."

PROFILE=0
CALLGRIND=0
BASE=""
RUNS=1
THREADS=1
LIMIT=1.0
FREQ=999
SEED="${SEED:-1}"
SCENE_ARGS=""
WANT=()
BINS=()

PERF_SCENES=(
    "cornell:scenes/cornell.test:--width 200 --spp 16"
    "dragon:scenes/dragon.test:--width 200 --spp 16"
)

# <=4 general-purpose events per pass, or the PMU multiplexes and extrapolates
EVENT_GROUPS=(
    "cycles,instructions,branches,branch-misses"
    "cycles,instructions,L1-dcache-loads,L1-dcache-load-misses"
    "cycles,instructions,cache-references,cache-misses,dTLB-load-misses"
)

METRICS=(
    cycles instructions
    branches branch-misses
    L1-dcache-loads L1-dcache-load-misses
    cache-references cache-misses
    dTLB-load-misses
)

while [ $# -gt 0 ]; do
    case "$1" in
    --profile) PROFILE=1; shift ;;
    --callgrind) CALLGRIND=1; shift ;;
    --base) BASE="$2"; shift 2 ;;
    --runs) RUNS="$2"; shift 2 ;;
    --threads) THREADS="$2"; shift 2 ;;
    --limit) LIMIT="$2"; shift 2 ;;
    --freq) FREQ="$2"; shift 2 ;;
    --args) SCENE_ARGS="$2"; shift 2 ;;
    -s | --scene) WANT+=("$2"); shift 2 ;;
    -*)
        echo "perfstat: unknown argument '$1'" >&2
        exit 2
        ;;
    *) BINS+=("$1"); shift ;;
    esac
done

if [ -n "$BASE" ] && [ "${#BINS[@]}" -gt 0 ]; then
    echo "perfstat: --base builds both binaries; do not also name them" >&2
    exit 2
fi
if [ -z "$BASE" ] && { [ "${#BINS[@]}" -eq 0 ] || [ "${#BINS[@]}" -gt 2 ]; }; then
    echo "usage: ci/perfstat.sh [--profile] [--callgrind] [-s NAME] BIN [HEAD_BIN]" >&2
    echo "       ci/perfstat.sh [--profile] [--callgrind] [-s NAME] --base REF" >&2
    exit 2
fi
for bin in ${BINS[@]+"${BINS[@]}"}; do
    if [ ! -x "$bin" ]; then
        echo "perfstat: '$bin' is not an executable (run ci/build.sh first)" >&2
        exit 2
    fi
done

if [ "$CALLGRIND" -eq 1 ]; then
    if ! command -v valgrind >/dev/null; then
        echo "perfstat: --callgrind needs valgrind (absent on macOS; use the ci image)" >&2
        exit 2
    fi
    if [ "$PROFILE" -eq 1 ] && ! command -v callgrind_annotate >/dev/null; then
        echo "perfstat: --callgrind --profile needs callgrind_annotate (ships with valgrind)" >&2
        exit 2
    fi
else
    if ! command -v perf >/dev/null; then
        echo "perfstat: needs linux perf; use --callgrind for a PMU-free equivalent" >&2
        exit 2
    fi
    if ! perf stat -x, -e cycles true >/dev/null 2>&1; then
        echo "perfstat: perf cannot read counters -- either" >&2
        echo "  sudo sysctl kernel.perf_event_paranoid=1   (--privileged in docker), or" >&2
        echo "  re-run with --callgrind (no PMU needed; works on CI runners)" >&2
        exit 2
    fi
fi

WORK=$(mktemp -d)
REFWORK=""
cleanup() {
    if [ -n "$REFWORK" ]; then
        ci/refbuild.sh --clean "$REFWORK" >/dev/null 2>&1 || true
    fi
    rm -rf "$WORK"
}
trap cleanup EXIT

# --base builds the pair via ci/refbuild.sh, shared with ci/compare.sh
if [ -n "$BASE" ]; then
    REFWORK="$WORK/refbuild"
    base_bin='' head_bin='' base_ref='' base_sha='' head_sha=''
    build=$(ci/refbuild.sh --base "$BASE" --out "$REFWORK") || exit $?
    eval "$build"
    BINS=("$base_bin" "$head_bin")
    echo "== base $base_ref ($base_sha) vs head ($head_sha)"
    echo
fi

# perf stat -x, fields: value,unit,event,stddev%,runtime,enabled%
declare -A VAL DEV
collect() {
    local idx="$1" bin="$2" scene="$3" extra="$4" group value event dev pct
    for group in "${EVENT_GROUPS[@]}"; do
        # shellcheck disable=SC2086  # $extra is a deliberate argument list
        perf stat -x, -r "$RUNS" -e "$group" -- \
            "$bin" --quiet --seed "$SEED" --threads "$THREADS" $extra \
            --out "$WORK/render" "$scene" 2>"$WORK/stat" >/dev/null || {
            echo "perfstat: run failed for $bin on $scene" >&2
            cat "$WORK/stat" >&2
            exit 1
        }
        while IFS=, read -r value _ event dev _ pct _; do
            [ -n "${event:-}" ] || continue
            case "$value" in '<not supported>' | '<not counted>' | '') continue ;; esac
            event="${event%:u}"
            if [ -n "${pct:-}" ] && awk "BEGIN{exit !($pct < 99.5)}" 2>/dev/null; then
                echo "perfstat: warning: $event multiplexed at ${pct}%" >&2
            fi
            VAL["$idx:$event"]="$value"
            DEV["$idx:$event"]="${dev:-0}"
        done <"$WORK/stat"
    done
}

# Same table, simulated: no PMU needed, so this is the CI path. No cycle
# model, hence no IPC -- compares counts, does not diagnose stalls.
collect_callgrind() {
    local idx="$1" bin="$2" scene="$3" extra="$4" out metric value
    # keep the output file: --profile annotates this run, not a second one
    # shellcheck disable=SC2086  # $extra is a deliberate argument list
    out=$(valgrind --tool=callgrind --callgrind-out-file="$WORK/cg-$idx.out" \
        --cache-sim=yes --branch-sim=yes \
        "$bin" --quiet --seed "$SEED" --threads 1 $extra \
        --out "$WORK/render" "$scene" 2>&1 >/dev/null) || true
    while read -r metric value; do
        VAL["$idx:$metric"]="$value"
        DEV["$idx:$metric"]=0
    done < <(printf '%s\n' "$out" | awk '
        { gsub(/[(),]/, ""); $0 = $0 }
        /I +refs:/     { print "instructions", $4 }
        /D +refs:/     { print "L1-dcache-loads", $5 }
        /D1 +misses:/  { print "L1-dcache-load-misses", $5 }
        /LL +refs:/    { print "cache-references", $4 }
        /LL +misses:/  { print "cache-misses", $4 }
        /Branches:/    { print "branches", $3 }
        /Mispredicts:/ { print "branch-misses", $3 }')
    if [ -z "${VAL[$idx:instructions]:-}" ]; then
        echo "perfstat: callgrind produced no counts for $bin" >&2
        printf '%s\n' "$out" | tail -5 >&2
        exit 1
    fi
}

counters() {
    local name="$1" scene="$2" extra="$3" idx metric rows=""
    if [ "$CALLGRIND" -eq 1 ]; then
        echo "== counters: $name ($extra, callgrind -- simulated, deterministic)"
    else
        echo "== counters: $name ($extra, ${THREADS}t, $RUNS run(s))"
    fi
    for idx in "${!BINS[@]}"; do
        if [ "$CALLGRIND" -eq 1 ]; then
            collect_callgrind "$idx" "${BINS[$idx]}" "$scene" "$extra"
        else
            collect "$idx" "${BINS[$idx]}" "$scene" "$extra"
        fi
    done

    for metric in "${METRICS[@]}"; do
        [ -n "${VAL[0:$metric]:-}" ] || continue
        rows+="$metric ${VAL[0:$metric]} ${DEV[0:$metric]:-0}"
        rows+=" ${VAL[1:$metric]:--} ${DEV[1:$metric]:--}"$'\n'
    done
    # ratios identify the bound; raw counts do not
    derive() {
        local label="$1" num="$2" den="$3" scale="$4" idx out=""
        for idx in 0 1; do
            if [ -n "${VAL[$idx:$num]:-}" ] && [ -n "${VAL[$idx:$den]:-}" ]; then
                out+=" $(awk -v n="${VAL[$idx:$num]}" -v d="${VAL[$idx:$den]}" \
                    -v s="$scale" 'BEGIN{ printf "%.4f", d ? n / d * s : 0 }') -"
            else
                out+=" - -"
            fi
        done
        [ "$out" = " - - - -" ] || rows+="$label$out"$'\n'
    }
    derive "IPC" instructions cycles 1
    derive "branch-miss%" branch-misses branches 100
    derive "L1-miss%" L1-dcache-load-misses L1-dcache-loads 100
    derive "LLC-miss%" cache-misses cache-references 100

    printf '%s' "$rows" | awk -v two="${#BINS[@]}" -v runs="$RUNS" '
        function comma(v,   s, out, n) {
            if (v ~ /\./ || v == "-") return v
            s = sprintf("%d", v); n = length(s)
            while (n > 3) { out = "," substr(s, n - 2, 3) out; s = substr(s, 1, n - 3); n -= 3 }
            return s out
        }
        function dev(d) { return (runs > 1 && d != "-" && d != "0" && d != "") \
            ? sprintf(" +-%.1f%%", d) : "" }
        BEGIN {
            if (two > 1) printf "%-22s %20s %20s %9s\n", "metric", "base", "head", "delta"
            else         printf "%-22s %20s\n", "metric", "value"
        }
        {
            b = comma($2) dev($3)
            if (two < 2) { printf "%-22s %20s\n", $1, b; next }
            h = comma($4) dev($5)
            d = ($2 + 0 && $4 != "-") ? sprintf("%+8.2f%%", ($4 - $2) / $2 * 100) : "        -"
            printf "%-22s %20s %20s %9s\n", $1, b, h, d
        }'
    echo
}

# Collapses 900-char template/lambda manglings. Shared by both profilers;
# reads "percent<TAB>raw-symbol" on stdin.
prettify_profile() {
    awk -F'\t' -v limit="$LIMIT" -v unit="$1" '
        function squash(s,   prev) {
            sub(/\[clone [^]]*\]/, "", s)
            do { prev = s; gsub(/<[^<>]*>/, "", s) } while (s != prev)
            gsub(/\{lambda[^{}]*#[0-9]+\}/, "lambda", s)
            # innermost parens to a sentinel first, so outer lists still match
            do { prev = s; gsub(/\([^()]*\)/, "\001", s) } while (s != prev)
            gsub(/\001/, "()", s)
            gsub(/aktis::/, "", s)
            gsub(/BoundingVolumeHierarchy/, "BVH", s)
            sub(/^void /, "", s); gsub(/  +/, " ", s)
            sub(/^ +/, "", s); sub(/ +$/, "", s)
            return s
        }
        # C++20 constrained-template manglings defeat valgrind and c++filt alike
        function shorten(s) {
            return s ~ /^_Z/ ? substr(s, 1, 44) "...[mangled]" : s
        }
        # the query survives only inside the template args -- tag before squashing
        function tag(raw) {
            if (raw ~ /::occluded\(/)  return " [occluded]"
            if (raw ~ /::intersect\(/) return " [intersect]"
            return ""
        }
        # findSplit/boundsOf/onAxis run only under build(); callgrind lists them
        # separately, unlike perf, which folds them in via the call graph
        function bucket(raw, short) {
            if (raw ~ /::occluded\(/)  return "occluded  (shadow rays)"
            if (raw ~ /::intersect\(/) return "intersect (camera + bounce rays)"
            if (raw ~ /::build\(/ || raw ~ /::BoundingVolumeHierarchy\(/ \
                || raw ~ /::findSplit\(/ || raw ~ /::boundsOf\(/ \
                || raw ~ /::onAxis\(/ || (raw ~ /^_Z/ && raw ~ /[0-9]build/))
                return "BVH build (one-off, amortizes with spp)"
            return short
        }
        {
            pct = $1 + 0
            if (pct < limit + 0) next
            raw = $2
            short = shorten(squash(raw)) tag(raw)
            b = bucket(raw, short)
            if (!(b in seen)) { order[++n] = b; seen[b] = 1 }
            roll[b] += pct
            flat[++m] = sprintf("  %5.1f%%  %s", pct, short)
            total += pct
        }
        END {
            if (!m) { print "  (nothing above the " limit "% cutoff)"; exit }
            print "  -- by bucket (" unit ")"
            for (i = 1; i <= n; i++) printf "  %5.1f%%  %s\n", roll[order[i]], order[i]
            printf "  %5.1f%%  (accounted for; rest is below the %s%% cutoff)\n", total, limit
            print "  -- by symbol"
            for (i = 1; i <= m; i++) print flat[i]
        }'
}

profile() {
    local name="$1" scene="$2" extra="$3" idx bin data
    for idx in "${!BINS[@]}"; do
        bin="${BINS[$idx]}"
        data="$WORK/perf-$idx.data"
        echo "== profile: $name ($extra) -- $bin"
        # shellcheck disable=SC2086  # $extra is a deliberate argument list
        perf record -q -F "$FREQ" -o "$data" -- \
            "$bin" --quiet --seed "$SEED" --threads "$THREADS" $extra \
            --out "$WORK/render" "$scene" >/dev/null 2>&1 || {
            echo "perfstat: perf record failed for $bin" >&2
            exit 1
        }
        perf report -i "$data" --no-children -s symbol -q --stdio \
            --percent-limit "$LIMIT" 2>/dev/null |
            awk '/^ *[0-9.]+%/ {
                pct = $1 + 0
                raw = $0
                sub(/^ *[0-9.]+% +/, "", raw)
                sub(/^\[[^]]*\] +/, "", raw)
                # perf pads flat reports with trailing placeholder columns
                sub(/[ \t]+-([ \t]+-)*[ \t]*$/, "", raw)
                printf "%s\t%s\n", pct, raw
            }' | prettify_profile "cycles sampled at ${FREQ}Hz"
        echo
    done
}

# Annotates the run collect_callgrind already did, so --profile is near free.
# Ranks by instructions, not time: deterministic, but blind to stalls.
profile_callgrind() {
    local name="$1" scene="$2" extra="$3" idx bin out
    for idx in "${!BINS[@]}"; do
        bin="${BINS[$idx]}"
        out="$WORK/cg-$idx.out"
        echo "== profile: $name ($extra) -- $bin"
        if [ ! -s "$out" ]; then
            echo "perfstat: no callgrind profile for $bin" >&2
            exit 1
        fi
        callgrind_annotate --show=Ir --threshold=99.9 "$out" 2>/dev/null |
            awk '/^ *[0-9,]+ \([0-9. ]+%\)/ {
                line = $0
                pct = line
                sub(/^[^(]*\(/, "", pct); sub(/%.*$/, "", pct)
                if (line ~ /PROGRAM TOTALS/) next
                raw = line
                sub(/^ *[0-9,]+ \([0-9. ]+%\) +/, "", raw)
                # annotate wraps the name: "file:" prefix (??? if unknown) ...
                sub(/^[^:]*:/, "", raw)
                # ... and "[object]" suffix
                sub(/ +\[[^]]*\]$/, "", raw)
                printf "%s\t%s\n", pct + 0, raw
            }' | prettify_profile "instructions retired"
        echo
    done
}

for entry in "${PERF_SCENES[@]}"; do
    name="${entry%%:*}"
    rest="${entry#*:}"
    scene="${rest%%:*}"
    extra="${SCENE_ARGS:-${rest#*:}}"

    if [ "${#WANT[@]}" -gt 0 ]; then
        match=0
        for want in "${WANT[@]}"; do
            [ "$want" = "$name" ] && match=1
        done
        [ "$match" -eq 1 ] || continue
    fi
    if [ ! -f "$scene" ]; then
        echo "perfstat: skip $name (no $scene)"
        continue
    fi

    if [ "$CALLGRIND" -eq 1 ]; then
        counters "$name" "$scene" "$extra"
        if [ "$PROFILE" -eq 1 ]; then
            profile_callgrind "$name" "$scene" "$extra"
        fi
    elif [ "$PROFILE" -eq 1 ]; then
        profile "$name" "$scene" "$extra"
    else
        counters "$name" "$scene" "$extra"
    fi
done
