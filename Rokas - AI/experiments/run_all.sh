#!/usr/bin/env bash
# Rebuilds the project and regenerates everything in results/.
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
repo="$(git -C "$here" rev-parse --show-toplevel)"
data="$repo/Joringis-no AI/data"
build="$here/build/exp"
results="$here/results"
raw="$results/raw"

[[ -d "$data/exp1" && -f "$data/konstitucija.txt" ]] || { echo "nerasta bendrų duomenų: $data" >&2; exit 1; }
python3 "$here/tests/check_fixtures.py" "$data" > /dev/null || { echo "blogi bendri duomenys (tests/check_fixtures.py)" >&2; exit 1; }

mkdir -p "$build"
cmake -S "$here" -B "$build" -DCMAKE_BUILD_TYPE=Release > /dev/null
cmake --build "$build" -j > /dev/null
"$build/sanity-checks" > /dev/null || { echo "nepraėjo sanity-checks" >&2; exit 1; }
"$here/tests/run_sanity.sh" "$build" > /dev/null || { echo "nepraėjo run_sanity.sh" >&2; exit 1; }
python3 "$here/tests/reference_check.py" "$build" > /dev/null || { echo "nepraėjo reference_check.py" >&2; exit 1; }
python3 "$here/tests/attack_v012.py" "$build" "$data" > /dev/null || { echo "nepraėjo attack_v012.py" >&2; exit 1; }

rm -rf "$results"
mkdir -p "$raw"

pin=()
command -v taskset > /dev/null && pin=(taskset -c 2)

# A timing run is repeated if other programs were using the CPU (over 12 % in total; our run
# alone is about 5 %) or, for speed, if any size varies by more than 10 %.
cpu() { read -r _ a b c d e f g h _ < /proc/stat; echo "$((a + b + c + f + g + h)) $((a + b + c + d + e + f + g + h))"; }
spread() {
  awk -F, '{ s[$3] += $8; n[$3]++; if (!($3 in lo) || $8 < lo[$3]) lo[$3] = $8; if ($8 > hi[$3]) hi[$3] = $8 }
           END { w = 0; for (k in s) { v = 100 * (hi[k] - lo[k]) / (s[k] / n[k]); if (v > w) w = v }; printf "%d", w }' "$1"
}
worst=0
repeated=0
timed() {
  local out="$1" busy1 total1 busy2 total2 load
  shift
  for try in $(seq 20); do
    read -r busy1 total1 <<< "$(cpu)"
    "$@" > "$out"
    read -r busy2 total2 <<< "$(cpu)"
    load=$(( 100 * (busy2 - busy1) / (total2 - total1) ))
    if (( load <= 12 )) && [[ "$out" != */speed.csv || $(spread "$out") -le 10 ]]; then
      (( load > worst )) && worst=$load
      return 0
    fi
    echo "$(basename "$out"): CPU apkrova ${load} % – kartojama po 20 s" >&2
    repeated=$((repeated + 1))
    sleep 20
  done
  echo "CPU nuolat apkrautas, $(basename "$out") nematuota" >&2
  exit 1
}

exp="$build/experiments"
"$exp" inputs "$data/exp1" > "$raw/inputs.csv"
timed "$raw/speed.csv" "${pin[@]}" "$exp" speed "$data/konstitucija.txt"
"$exp" collisions > "$raw/collisions.csv"
"$exp" structured > "$raw/structured.csv"
"$exp" avalanche > "$raw/avalanche.csv"
timed "$raw/guess.csv" "${pin[@]}" "$exp" guess

cli="$build/hash-generator"
{
  for f in "$data"/exp1/*; do
    name="$(basename "$f")"
    first="$("$cli" --file "$f" 2>/dev/null)"
    second="$("$cli" --file "$f" 2>/dev/null)"
    echo "runs,$name,$first,$([[ "$first" == "$second" ]] && echo 1 || echo 0)"
    if tr -d '\n\r\000' < "$f" | cmp -s - "$f"; then
      typed="$({ cat "$f"; printf '\n'; } | "$cli" 2>/dev/null)"
      text="$("$cli" --text "$(cat "$f")" 2>/dev/null)"
      echo "typed,$name,$([[ "$typed" == "$first" ]] && echo 1 || echo 0)"
      echo "text,$name,$([[ "$text" == "$first" ]] && echo 1 || echo 0)"
    fi
  done
} > "$raw/cli.csv"

cache="$build/CMakeCache.txt"
compiler="$(sed -n 's/^CMAKE_CXX_COMPILER:[A-Z]*=//p' "$cache")"
flags="$(sed -n 's/^CMAKE_CXX_FLAGS_RELEASE:[A-Z]*=//p' "$cache")"
cat > "$results/aplinka.md" <<EOF
# Vykdymo aplinka

| | |
|---|---|
| Data | $(date '+%Y-%m-%d %H:%M') |
| Procesorius | $(grep -m1 'model name' /proc/cpuinfo | cut -d: -f2 | sed 's/^ *//'), $(nproc) loginiai branduoliai |
| Atmintis | $(free -g | awk '/^Mem:/{print $2}') GB |
| OS | $(. /etc/os-release && echo "$PRETTY_NAME"), $(uname -sr) |
| Kompiliatorius | $("$compiler" --version | head -1) |
| Parinktys | \`-std=c++20 $flags\` (CMake Release), viena gija |
| Spartos matavimai | prisegti prie branduolio 2 (\`taskset -c 2\`), dažnio valdiklis \`$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor 2>/dev/null || echo nežinomas)\` |
| Generatorius | \`std::mt19937_64\`, simbolis = \`'!' + (x mod 94)\`, bazinis seed 20260920 |
| CPU apkrova matuojant laiką | ≤ ${worst} % (dėl kitų procesų pakartotų paleidimų: ${repeated}) |
| Duomenys | bendras poros rinkinys \`Joringis-no AI/data/\` (\`exp1/\`, \`konstitucija.txt\`) |
| Duomenų SHA-256 | \`$(cd "$data" && find exp1 konstitucija.txt -type f | LC_ALL=C sort | xargs sha256sum | sha256sum | cut -c1-16)…\` |
| Realizacija | commit $(git -C "$repo" log -1 --format=%h -- "Rokas - AI/src") |
| Eksperimentų programa | commit $(git -C "$repo" log -1 --format=%h -- "Rokas - AI/experiments") |
| Darbo medis | $(git -C "$repo" status --porcelain -- "Rokas - AI/src" "Rokas - AI/include" "Rokas - AI/experiments" "Rokas - AI/CMakeLists.txt" "Joringis-no AI/data" | grep -q . && echo "yra neįrašytų pakeitimų" || echo "švarus") |
EOF

python3 "$here/experiments/report.py" "$raw" "$results"
