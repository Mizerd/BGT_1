#!/usr/bin/env bash
# Papildoma užduotis: abi poros maišos ir standartinės MD5, SHA-1, SHA-256 (OpenSSL) tomis pačiomis
# sąlygomis – ta pati eksperimentų programa, tos pačios įvestys, tas pats kompiuteris.
# Reikia OpenSSL 3 (pkg-config libcrypto); NixOS: nix-shell -p openssl pkg-config --run ./palyginimas/standartai.sh
# Sparta matuojama – leisti tik neapkrautame kompiuteryje.
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ai="$root/Rokas - AI"
noai="$root/Joringis-no AI"
out="$root/palyginimas"
build="$out/build"
raw="$out/raw"
mkdir -p "$build" "$raw"

ssl="$(pkg-config --cflags --libs libcrypto)" || { echo "nerastas OpenSSL (pkg-config libcrypto)" >&2; exit 1; }
read -r -a ssl <<< "$ssl"
flags=(-std=c++20 -O3 -DNDEBUG -I"$ai/experiments")
g++ "${flags[@]}" -fno-tree-reassoc -I"$ai/include" "$ai/experiments/impl_eduhash.cpp" "$ai/src/custom_hash.cpp" \
  "$ai/experiments/experiments.cpp" -o "$build/di"
g++ "${flags[@]}" "$noai/experiments/impl_ratas.cpp" "$ai/experiments/experiments.cpp" -o "$build/bedi"
for spec in md5:MD5:16 sha1:SHA1:20 sha256:SHA256:32; do
  IFS=: read -r name algo bytes <<< "$spec"
  g++ "${flags[@]}" -DIMPL_NAME="\"$name\"" -DALGO="\"$algo\"" -DDIGEST_BYTES="$bytes" \
    "$out/impl_openssl.cpp" "$ai/experiments/experiments.cpp" "${ssl[@]}" -o "$build/$name"
done

: > "$raw/std_avalanche.csv"
: > "$raw/std_inputs.csv"
for b in bedi di md5 sha1 sha256; do
  "$build/$b" avalanche >> "$raw/std_avalanche.csv"
done
for b in md5 sha1 sha256; do
  "$build/$b" inputs "$noai/data/exp1" >> "$raw/std_inputs.csv"
done

# Sparta. Kiti procesai iškraipo matavimus, todėl paleidimas kartojamas, jei jo metu visas CPU buvo apkrautas
# > 12 % (vienas matavimas – ≈ 5 %) arba kurio nors dydžio sklaida (max − min) viršijo 10 % vidurkio.
pin=()
command -v taskset > /dev/null && pin=(taskset -c 2)
cpu() { read -r _ a b c d e f g h _ < /proc/stat; echo "$((a + b + c + f + g + h)) $((a + b + c + d + e + f + g + h))"; }
spread() {
  awk -F, '{ s[$3] += $8; n[$3]++; if (!($3 in lo) || $8 < lo[$3]) lo[$3] = $8; if ($8 > hi[$3]) hi[$3] = $8 }
           END { w = 0; for (k in s) { v = 100 * (hi[k] - lo[k]) / (s[k] / n[k]); if (v > w) w = v }; printf "%d", w }' "$1"
}
: > "$raw/std_speed.csv"
worst=0
repeated=0
for b in bedi di md5 sha1 sha256; do
  for try in $(seq 20); do
    read -r busy1 total1 <<< "$(cpu)"
    "${pin[@]}" "$build/$b" speed "$noai/data/konstitucija.txt" > "$build/$b.speed"
    read -r busy2 total2 <<< "$(cpu)"
    load=$(( 100 * (busy2 - busy1) / (total2 - total1) ))
    width=$(spread "$build/$b.speed")
    (( load <= 12 && width <= 10 )) && break
    (( try == 20 )) && { echo "$b: CPU nuolat apkrautas, sparta nematuota" >&2; exit 1; }
    echo "$b: CPU apkrova ${load} %, sklaida ${width} % – kartojama po 20 s" >&2
    repeated=$((repeated + 1))
    sleep 20
  done
  (( load > worst )) && worst=$load
  cat "$build/$b.speed" >> "$raw/std_speed.csv"
done
echo "$(grep -m1 'model name' /proc/cpuinfo | cut -d: -f2 | sed 's/^ *//'), $(uname -sr), $(g++ --version | head -1) -O3," \
  "OpenSSL $(pkg-config --modversion libcrypto); CPU apkrova matuojant ≤ ${worst} %, sklaida ≤ 10 %," \
  "dėl kitų procesų pakartotų paleidimų: ${repeated}" > "$raw/std_aplinka.txt"

python3 "$out/standartai.py" "$raw" "$out" "$noai/data/exp1"
