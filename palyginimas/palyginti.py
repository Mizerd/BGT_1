#!/usr/bin/env python3
"""Bendra spartos lentelė ir grafikas abiem realizacijoms iš raw/speed.csv."""

import csv
import math
import os
import sys
from collections import defaultdict

RAW, OUT = sys.argv[1], sys.argv[2]
IMPLS = ["beDI", "di"]
NAMES = {"beDI": "Ratas-256 (be DI)", "di": "DI maiša"}
LIGHT = {"bg": "#fcfcfb", "text": "#0b0b0b", "text2": "#52514e", "grid": "#e4e3de", "di": "#2a78d6", "beDI": "#eb6834"}
DARK = {"bg": "#1a1a19", "text": "#ffffff", "text2": "#c3c2b7", "grid": "#353533", "di": "#3987e5", "beDI": "#d95926"}


def num(x, d):
    return f"{x:,.{d}f}".replace(",", " ").replace(".", ",")


times, size = defaultdict(list), {}
with open(os.path.join(RAW, "speed.csv"), newline="") as f:
    for _, impl, lines, nbytes, _, _, _, per in csv.reader(f):
        times[(impl, int(lines))].append(float(per) / 1000)
        size[int(lines)] = int(nbytes)
order = sorted(size)
mean = {k: sum(v) / len(v) for k, v in times.items()}

rows = ["| Baitai | " + " | ".join(f"{NAMES[i]}, µs" for i in IMPLS) + " |", "|---|---|---|"]
for n in order:
    rows.append(f"| {num(size[n], 0)} | " + " | ".join(
        f"{num(mean[(i, n)], 3)} ({num(min(times[(i, n)]), 3)}–{num(max(times[(i, n)]), 3)})" for i in IMPLS) + " |")
with open(os.path.join(OUT, "sparta.md"), "w") as f:
    f.write("Laikas vienai maišai, µs: vidurkis (min–max), 10 matavimų.\n\n" + "\n".join(rows) + "\n")

W, H, L, R, T, B = 760, 430, 78, 150, 64, 62
x0, x1, y0, y1 = 1, 5, -2, 2
px = lambda b: L + (math.log10(b) - x0) / (x1 - x0) * (W - L - R)
py = lambda us: H - B - (math.log10(us) - y0) / (y1 - y0) * (H - T - B)


def block(p):
    return (f".bg{{fill:{p['bg']}}} .t{{fill:{p['text']}}} .t2{{fill:{p['text2']}}} .grid{{stroke:{p['grid']}}} "
            + " ".join(f".f-{i}{{fill:{p[i]}}} .s-{i}{{stroke:{p[i]}}}" for i in IMPLS) + f" .ring{{stroke:{p['bg']}}}")


title = "Vienos maišos laikas pagal įvesties dydį (tas pats kompiuteris)"
s = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" viewBox="0 0 {W} {H}" role="img" aria-label="{title}">',
     f"<style>text{{font-family:system-ui,-apple-system,'Segoe UI',sans-serif}} {block(LIGHT)} "
     f"@media (prefers-color-scheme: dark){{{block(DARK)}}}</style>",
     f'<rect class="bg" fill="{LIGHT["bg"]}" width="{W}" height="{H}"/>',
     f'<text class="t" x="{L}" y="26" font-size="16" font-weight="600">{title}</text>']
lx = L
for i in IMPLS:
    s.append(f'<rect class="f-{i}" fill="{LIGHT[i]}" x="{lx}" y="38" width="12" height="12" rx="3"/>'
             f'<text class="t2" x="{lx + 17}" y="48" font-size="12">{NAMES[i]}</text>')
    lx += 170
for e in range(x0, x1 + 1):
    s.append(f'<line class="grid" stroke="{LIGHT["grid"]}" x1="{px(10**e):.1f}" y1="{T}" x2="{px(10**e):.1f}" y2="{H - B}"/>'
             f'<text class="t2" x="{px(10**e):.1f}" y="{H - B + 18}" font-size="12" text-anchor="middle">{num(10**e, 0)}</text>')
for e in range(y0, y1 + 1):
    s.append(f'<line class="grid" stroke="{LIGHT["grid"]}" x1="{L}" y1="{py(10**e):.1f}" x2="{W - R}" y2="{py(10**e):.1f}"/>'
             f'<text class="t2" x="{L - 8}" y="{py(10**e) + 4:.1f}" font-size="12" text-anchor="end">{num(10**e, max(0, -e))}</text>')
s.append(f'<text class="t2" x="{(L + W - R) / 2}" y="{H - 16}" font-size="13" text-anchor="middle">Įvesties dydis, baitai (log. skalė)</text>')
s.append(f'<text class="t2" transform="translate(20 {(T + H - B) / 2}) rotate(-90)" font-size="13" text-anchor="middle">Laikas vienai maišai, µs (log. skalė)</text>')
for i in IMPLS:
    pts = [(px(size[n]), py(mean[(i, n)]), n) for n in order]
    s.append(f'<path class="s-{i}" stroke="{LIGHT[i]}" fill="none" stroke-width="2" stroke-linejoin="round" d="'
             + " ".join(f"{'M' if k == 0 else 'L'}{x:.1f},{y:.1f}" for k, (x, y, _) in enumerate(pts)) + '"/>')
    for x, y, n in pts:
        s.append(f'<circle class="f-{i} ring" fill="{LIGHT[i]}" stroke="{LIGHT["bg"]}" stroke-width="2" cx="{x:.1f}" cy="{y:.1f}" r="4.5">'
                 f'<title>{NAMES[i]}: {num(size[n], 0)} B – {num(mean[(i, n)], 3)} µs</title></circle>')
    s.append(f'<text class="t" x="{pts[-1][0] + 10:.1f}" y="{pts[-1][1] + 4:.1f}" font-size="12">{NAMES[i]}</text>')
with open(os.path.join(OUT, "sparta.svg"), "w") as f:
    f.write("\n".join(s + ["</svg>"]) + "\n")

print("\n".join(rows))
rep_path = os.path.join(RAW, "atkartojamumas.csv")
if os.path.exists(rep_path):
    with open(rep_path, newline="") as f:
        print("atkartojamumas:", ", ".join(f"{r[0]}/{r[1]}={'taip' if r[2] == '1' else 'NE'}" for r in csv.reader(f)))
