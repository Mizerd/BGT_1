#!/usr/bin/env bash
# Rebuilds all implementations and regenerates everything in results/.
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
repo="$(git -C "$here" rev-parse --show-toplevel)"
partner="$repo/Joringis-no AI"
data="$partner/data"
build="$here/build/exp"
results="$here/results"
raw="$results/raw"
v1_commit=b3d54be

[[ -d "$data/exp1" && -f "$data/konstitucija.txt" ]] || { echo "nerasta bendrų duomenų: $data" >&2; exit 1; }
mkdir -p "$build" "$raw"

git -C "$repo" show "$v1_commit:Rokas - AI/src/custom_hash.cpp" > "$build/custom_hash_v1.cpp"
cmake -S "$here" -B "$build" -DCMAKE_BUILD_TYPE=Release \
  -DV1_SOURCE="$build/custom_hash_v1.cpp" -DRATAS_DIR="$partner" > /dev/null
cmake --build "$build" -j > /dev/null

pin=()
command -v taskset > /dev/null && pin=(taskset -c 2)

for f in inputs speed collisions structured avalanche guess; do : > "$raw/$f.csv"; done
for bin in experiments experiments_v1 experiments_ratas; do
  echo "== $bin"
  "$build/$bin" inputs "$data/exp1" >> "$raw/inputs.csv"
  "${pin[@]}" "$build/$bin" speed "$data/konstitucija.txt" >> "$raw/speed.csv"
  "$build/$bin" collisions >> "$raw/collisions.csv"
  "$build/$bin" structured >> "$raw/structured.csv"
  "$build/$bin" avalanche >> "$raw/avalanche.csv"
  "${pin[@]}" "$build/$bin" guess >> "$raw/guess.csv"
done
"$build/reversal_v1" > "$raw/reversal_v1.csv"

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
| 2 versija | $(git -C "$repo" log -1 --format=%h -- "Rokas - AI/src") |
| 1 versija | $v1_commit |
| Ratas v0.1 | $(git -C "$repo" log -1 --format=%h -- "Joringis-no AI/src") |
EOF

python3 "$here/experiments/report.py" "$raw" "$results"
