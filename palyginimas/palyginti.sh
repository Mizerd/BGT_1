#!/usr/bin/env bash
# Abi realizacijos toje pačioje aplinkoje.
#   palyginti.sh sparta          – perrašo raw/speed_linux.csv (matuoti tik neapkrautame kompiuteryje);
#                                  Windows matavimai raw/speed.csv nekeičiami
#   palyginti.sh atkartojamumas  – tikrina, ar deterministiniai rezultatai sutampa su įrašytais
#   palyginti.sh                 – abu
set -euo pipefail

mode="${1:-viskas}"
root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ai="$root/Rokas - AI"
noai="$root/Joringis-no AI"
out="$root/palyginimas"
build="$out/build"
raw="$out/raw"
mkdir -p "$build" "$raw"

flags=(-std=c++20 -O3 -DNDEBUG -I"$ai/experiments")
g++ "${flags[@]}" -fno-tree-reassoc -I"$ai/include" "$ai/experiments/impl_eduhash.cpp" "$ai/src/custom_hash.cpp" \
  "$ai/experiments/experiments.cpp" -o "$build/di"
g++ "${flags[@]}" "$noai/experiments/impl_ratas.cpp" "$ai/experiments/experiments.cpp" -o "$build/bedi"

if [[ "$mode" == sparta || "$mode" == viskas ]]; then
  # Kaip standartai.sh: paleidimas kartojamas, jei CPU apkrova > 12 % arba sklaida > 10 %.
  pin=()
  command -v taskset > /dev/null && pin=(taskset -c 2)
  cpu() { read -r _ a b c d e f g h _ < /proc/stat; echo "$((a + b + c + f + g + h)) $((a + b + c + d + e + f + g + h))"; }
  spread() {
    awk -F, '{ s[$3] += $8; n[$3]++; if (!($3 in lo) || $8 < lo[$3]) lo[$3] = $8; if ($8 > hi[$3]) hi[$3] = $8 }
             END { w = 0; for (k in s) { v = 100 * (hi[k] - lo[k]) / (s[k] / n[k]); if (v > w) w = v }; printf "%d", w }' "$1"
  }
  : > "$raw/speed_linux.csv"
  for b in bedi di; do
    for try in $(seq 20); do
      read -r busy1 total1 <<< "$(cpu)"
      "${pin[@]}" "$build/$b" speed "$noai/data/konstitucija.txt" > "$build/$b.speed"
      read -r busy2 total2 <<< "$(cpu)"
      load=$(( 100 * (busy2 - busy1) / (total2 - total1) ))
      (( load <= 12 && $(spread "$build/$b.speed") <= 10 )) && break
      (( try == 20 )) && { echo "$b: CPU nuolat apkrautas, sparta nematuota" >&2; exit 1; }
      echo "$b: CPU apkrova ${load} % – kartojama po 20 s" >&2
      sleep 20
    done
    cat "$build/$b.speed" >> "$raw/speed_linux.csv"
  done
  echo "$(grep -m1 'model name' /proc/cpuinfo | cut -d: -f2 | sed 's/^ *//'), $(uname -sr), $(g++ --version | head -1), -O3" \
    > "$raw/speed_linux.txt"
fi

if [[ "$mode" == atkartojamumas || "$mode" == viskas ]]; then
  : > "$raw/atkartojamumas.csv"
  for pair in "bedi:$noai" "di:$ai"; do
    b="${pair%%:*}"
    dir="${pair#*:}"
    for e in inputs collisions structured avalanche; do
      if [[ "$e" == inputs ]]; then "$build/$b" inputs "$noai/data/exp1" > "$build/$e.csv"; else "$build/$b" "$e" > "$build/$e.csv"; fi
      same=0
      cmp -s "$build/$e.csv" "$dir/results/raw/$e.csv" && same=1
      echo "$b,$e,$same" >> "$raw/atkartojamumas.csv"
    done
  done
fi

python3 "$out/palyginti.py" "$raw" "$out"
