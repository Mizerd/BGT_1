#!/usr/bin/env python3
"""Palyginimas su MD5, SHA-1, SHA-256: lentelės (standartai.md) ir spartos grafikas (standartai.svg).

Naudojimas: standartai.py <raw katalogas> <išvesties katalogas> <exp1 katalogas>
"""

import csv
import hashlib
import math
import os
import sys
from collections import defaultdict

RAW, OUT, EXP1 = sys.argv[1], sys.argv[2], sys.argv[3]
IMPLS = ["beDI", "di", "md5", "sha1", "sha256"]
NAMES = {"beDI": "Ratas-256", "di": "DI maiša", "md5": "MD5", "sha1": "SHA-1", "sha256": "SHA-256"}
BITS = {"beDI": 256, "di": 256, "md5": 128, "sha1": 160, "sha256": 256}
STANDARD = {"md5", "sha1", "sha256"}
LIGHT = {"bg": "#fcfcfb", "text": "#0b0b0b", "text2": "#52514e", "grid": "#e4e3de",
         "beDI": "#eb6834", "di": "#2a78d6", "md5": "#1baf7a", "sha1": "#8a63d2", "sha256": "#c43d7a"}
DARK = {"bg": "#1a1a19", "text": "#ffffff", "text2": "#c3c2b7", "grid": "#353533",
        "beDI": "#d95926", "di": "#3987e5", "md5": "#2fc48d", "sha1": "#a585e6", "sha256": "#e0619a"}


def num(x, d):
    return f"{x:,.{d}f}".replace(",", " ").replace(".", ",")


# Standartinių adapterių patikra: exp1 failų maišos turi sutapti su Python hashlib.
checked = matched = 0
with open(os.path.join(RAW, "std_inputs.csv"), newline="") as f:
    for row in csv.reader(f):
        if row[0] == "input":
            with open(os.path.join(EXP1, row[2]), "rb") as g:
                matched += hashlib.new(row[1], g.read()).hexdigest() == row[5]
            checked += 1
if checked == 0 or matched != checked:
    sys.exit(f"adapteriai nesutampa su hashlib: {matched}/{checked}")

times, size = defaultdict(list), {}
with open(os.path.join(RAW, "std_speed.csv"), newline="") as f:
    for _, impl, lines, nbytes, _, _, _, per in csv.reader(f):
        times[(impl, int(lines))].append(float(per) / 1000)
        size[int(lines)] = int(nbytes)
order = sorted(size)
mean = {k: sum(v) / len(v) for k, v in times.items()}
last = order[-1]

aval = {}
with open(os.path.join(RAW, "std_avalanche.csv"), newline="") as f:
    for row in csv.reader(f):
        if row[0] == "aval":
            aval[(row[1], row[2], row[3])] = [float(x) for x in row[5:]]

with open(os.path.join(RAW, "std_aplinka.txt")) as f:
    env = f.read().strip()


def ideal_bits_sd(bits):
    return 50 / math.sqrt(bits)


def ideal_hex_sd(bits):
    return 100 * math.sqrt(15 / 256 / (bits // 4))


head = "| " + " | ".join(NAMES[i] for i in IMPLS) + " |"
sep = "|---|" + "---|" * len(IMPLS)
md = ["# Palyginimas su MD5, SHA-1, SHA-256", "",
      f"Aplinka: {env}, viena gija. Visoms 5 maišoms – ta pati eksperimentų programa",
      "(`Rokas - AI/experiments/experiments.cpp`) ir tos pačios įvestys; standartinės maišos – OpenSSL `EVP` realizacijos.",
      f"Adapterių patikra: {matched}/{checked} `exp1` failų maišų sutampa su Python `hashlib`.", "",
      "## Sparta (4 eksperimento sąlygos)", "",
      "Laikas vienai maišai, µs: vidurkis (min–max), 3 apšilimai + 10 matavimų, be I/O.", "",
      "| Baitai " + head, sep]
for n in order:
    md.append(f"| {num(size[n], 0)} | " + " | ".join(
        f"{num(mean[(i, n)], 3)} ({num(min(times[(i, n)]), 3)}–{num(max(times[(i, n)]), 3)})" for i in IMPLS) + " |")
md.append(f"| **MB/s, {num(size[last], 0)} B** | " + " | ".join(
    f"**{num(size[last] / mean[(i, last)], 0)}**" for i in IMPLS) + " |")

md += ["", "## Lavinos efektas (6 eksperimento sąlygos)", "",
       "100 000 porų (po 25 000 ilgiams 10, 100, 500, 1 000). Procentai normalizuoti pagal maišos ilgį:",
       "bitų skirtumas / bitų skaičius, hex skirtumas / hex skaitmenų skaičius. Idealus standartinis nuokrypis",
       "priklauso nuo ilgio: bitams 50 / √n %, hex skaitmenims 100 · √(15/256 / (n/4)) %.", "",
       "| Maiša | Bitai | Bitų skirtumas, % | Min–max, % | Std. nuokrypis (idealus), % | Hex skirtumas, % | Hex std. (idealus), % | Apverstas 1 bitas, % |",
       "|---|---|---|---|---|---|---|---|"]
for i in IMPLS:
    bmin, bmax, bmean, bsd, hmin, hmax, hmean, hsd = aval[(i, "simbolis", "visi")]
    flip = aval[(i, "bitas", "visi")][2]
    md.append(f"| {NAMES[i]} | {BITS[i]} | {num(bmean, 2)} | {num(bmin, 1)}–{num(bmax, 1)} | "
              f"{num(bsd, 2)} ({num(ideal_bits_sd(BITS[i]), 2)}) | {num(hmean, 2)} | {num(hsd, 2)} ({num(ideal_hex_sd(BITS[i]), 2)}) | "
              f"{num(flip, 2)} |")
md += ["", "Bitų skirtumo vidurkis pagal įvesties ilgį, %:", "", "| Ilgis " + head, sep]
for length in ("10", "100", "500", "1000"):
    md.append(f"| {num(int(length), 0)} | " + " | ".join(num(aval[(i, 'simbolis', length)][2], 2) for i in IMPLS) + " |")
with open(os.path.join(OUT, "standartai.md"), "w") as f:
    f.write("\n".join(md) + "\n")

# Spartos grafikas: dydis (baitai) ir laikas vienai maišai, abi ašys logaritminės.
W, H, L, R, T, B = 760, 440, 78, 110, 74, 62
x0, x1 = math.floor(math.log10(min(size.values()))), math.ceil(math.log10(max(size.values())))
y0, y1 = math.floor(math.log10(min(mean.values()))), math.ceil(math.log10(max(mean.values())))
px = lambda b: L + (math.log10(b) - x0) / (x1 - x0) * (W - L - R)
py = lambda us: H - B - (math.log10(us) - y0) / (y1 - y0) * (H - T - B)


def block(p):
    return (f".bg{{fill:{p['bg']}}} .t{{fill:{p['text']}}} .t2{{fill:{p['text2']}}} .grid{{stroke:{p['grid']}}} "
            + " ".join(f".f-{i}{{fill:{p[i]}}} .s-{i}{{stroke:{p[i]}}}" for i in IMPLS) + f" .ring{{stroke:{p['bg']}}}")


title = "Vienos maišos laikas: abi poros maišos ir standartinės (tas pats kompiuteris)"
s = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" viewBox="0 0 {W} {H}" role="img" aria-label="{title}">',
     f"<style>text{{font-family:system-ui,-apple-system,'Segoe UI',sans-serif}} {block(LIGHT)} "
     f"@media (prefers-color-scheme: dark){{{block(DARK)}}}</style>",
     f'<rect class="bg" fill="{LIGHT["bg"]}" width="{W}" height="{H}"/>',
     f'<text class="t" x="{L}" y="26" font-size="16" font-weight="600">{title}</text>']
lx = L
for i in IMPLS:
    dash = ' stroke-dasharray="5 3"' if i in STANDARD else ""
    s.append(f'<line class="s-{i}" stroke="{LIGHT[i]}" stroke-width="2.5"{dash} x1="{lx}" y1="46" x2="{lx + 18}" y2="46"/>'
             f'<text class="t2" x="{lx + 23}" y="50" font-size="12">{NAMES[i]}</text>')
    lx += 125
s.append(f'<text class="t2" x="{L}" y="66" font-size="11">punktyras – standartinės realizacijos (OpenSSL)</text>')
for e in range(x0, x1 + 1):
    s.append(f'<line class="grid" stroke="{LIGHT["grid"]}" x1="{px(10**e):.1f}" y1="{T}" x2="{px(10**e):.1f}" y2="{H - B}"/>'
             f'<text class="t2" x="{px(10**e):.1f}" y="{H - B + 18}" font-size="12" text-anchor="middle">{num(10**e, 0)}</text>')
for e in range(y0, y1 + 1):
    s.append(f'<line class="grid" stroke="{LIGHT["grid"]}" x1="{L}" y1="{py(10**e):.1f}" x2="{W - R}" y2="{py(10**e):.1f}"/>'
             f'<text class="t2" x="{L - 8}" y="{py(10**e) + 4:.1f}" font-size="12" text-anchor="end">{num(10**e, max(0, -e))}</text>')
s.append(f'<text class="t2" x="{(L + W - R) / 2}" y="{H - 16}" font-size="13" text-anchor="middle">Įvesties dydis, baitai (log. skalė)</text>')
s.append(f'<text class="t2" transform="translate(20 {(T + H - B) / 2}) rotate(-90)" font-size="13" text-anchor="middle">Laikas vienai maišai, µs (log. skalė)</text>')
ends = []
for i in IMPLS:
    pts = [(px(size[n]), py(mean[(i, n)]), n) for n in order]
    dash = ' stroke-dasharray="5 3"' if i in STANDARD else ""
    s.append(f'<path class="s-{i}" stroke="{LIGHT[i]}" fill="none" stroke-width="2"{dash} stroke-linejoin="round" d="'
             + " ".join(f"{'M' if k == 0 else 'L'}{x:.1f},{y:.1f}" for k, (x, y, _) in enumerate(pts)) + '"/>')
    for x, y, n in pts:
        s.append(f'<circle class="f-{i} ring" fill="{LIGHT[i]}" stroke="{LIGHT["bg"]}" stroke-width="1.5" cx="{x:.1f}" cy="{y:.1f}" r="3.5">'
                 f'<title>{NAMES[i]}: {num(size[n], 0)} B – {num(mean[(i, n)], 3)} µs</title></circle>')
    ends.append([pts[-1][1], pts[-1][0], i])
ends.sort()
for _ in range(100):  # galų užrašai neturi užlipti vienas ant kito
    for k in range(1, len(ends)):
        gap = ends[k][0] - ends[k - 1][0]
        if gap < 15:
            ends[k - 1][0] -= (15 - gap) / 2
            ends[k][0] += (15 - gap) / 2
for y, x, i in ends:
    s.append(f'<text class="t" x="{x + 10:.1f}" y="{y + 4:.1f}" font-size="12">{NAMES[i]}</text>')
with open(os.path.join(OUT, "standartai.svg"), "w") as f:
    f.write("\n".join(s + ["</svg>"]) + "\n")

print("\n".join(md))
