#!/usr/bin/env bash
# Abi realizacijos toje pačioje aplinkoje: sparta ir deterministinių rezultatų atkartojamumas.
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ai="$root/Rokas - AI"
noai="$root/Joringis-no AI"
out="$root/palyginimas"
build="$out/build"
raw="$out/raw"
mkdir -p "$build" "$raw"

flags=(-std=c++20 -O3 -DNDEBUG -I"$ai/experiments")
g++ "${flags[@]}" -I"$ai/include" "$ai/experiments/impl_eduhash.cpp" "$ai/src/custom_hash.cpp" \
  "$ai/experiments/experiments.cpp" -o "$build/di"
g++ "${flags[@]}" "$noai/experiments/impl_ratas.cpp" "$ai/experiments/experiments.cpp" -o "$build/bedi"

pin=()
command -v taskset > /dev/null && pin=(taskset -c 2)

: > "$raw/speed.csv"
for b in bedi di; do
  "${pin[@]}" "$build/$b" speed "$noai/data/konstitucija.txt" >> "$raw/speed.csv"
done

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

python3 "$out/palyginti.py" "$raw" "$out"
