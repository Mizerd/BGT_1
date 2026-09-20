#!/usr/bin/env python3
"""Braižo grafikus iš results/*.csv į SVG (be papildomų bibliotekų).

  python tools/plot.py

Sukuria:
  results/exp4_speed.svg     – vienos maišos laikas (µs) pagal įvesties dydį (baitais), log-log
  results/exp6_histogram.svg – bitų skirtumo % histogramos (Ratas v0.1: simbolio pakeitimas / vieno bito apvertimas)
"""
import csv
import math
import os

RESULTS = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "results")

# Kategorinės spalvos fiksuota tvarka (viena spalva – viena funkcija).
SERIES_ORDER = ["Ratas v0.1", "MD5", "SHA-1", "SHA-256"]
COLORS = {
    "Ratas v0.1": "#2a78d6",
    "MD5": "#eb6834",
    "SHA-1": "#1baf7a",
    "SHA-256": "#eda100",
}
MODE_COLORS = {"simbolis": "#2a78d6", "bitas": "#eb6834"}
SURFACE, INK, INK2, GRID = "#fcfcfb", "#0b0b0b", "#52514e", "#e6e5e1"
FONT = "font-family='Segoe UI, Helvetica, Arial, sans-serif'"


def esc(s):
    return s.replace("&", "&amp;").replace("<", "&lt;")


def speed_chart():
    rows = list(csv.DictReader(open(os.path.join(RESULTS, "exp4_bench.csv"), encoding="utf-8")))
    series = {}
    for r in rows:
        series.setdefault(r["hash"], []).append((int(r["bytes"]), float(r["mean_ns"]) / 1000.0,
                                                 float(r["min_ns"]) / 1000.0, float(r["max_ns"]) / 1000.0))
    names = [n for n in SERIES_ORDER if n in series]

    W, H = 760, 440
    L, R, T, B = 70, 150, 52, 60
    xs = [p[0] for s in series.values() for p in s]
    ys = [p[1] for s in series.values() for p in s]
    x0, x1 = math.log10(min(xs)) - 0.1, math.log10(max(xs)) + 0.1
    y0, y1 = math.floor(math.log10(min(ys))), math.ceil(math.log10(max(ys)))

    def X(v): return L + (math.log10(v) - x0) / (x1 - x0) * (W - L - R)
    def Y(v): return T + (y1 - math.log10(v)) / (y1 - y0) * (H - T - B)

    out = [f"<svg xmlns='http://www.w3.org/2000/svg' width='{W}' height='{H}' viewBox='0 0 {W} {H}' {FONT} font-size='12'>",
           f"<rect width='{W}' height='{H}' fill='{SURFACE}'/>",
           f"<text x='{L}' y='22' font-size='15' font-weight='600' fill='{INK}'>Vienos maišos skaičiavimo laikas pagal įvesties dydį</text>"]
    # tinklelis ir ašys (log10)
    for e in range(y0, y1 + 1):
        y = Y(10 ** e)
        out.append(f"<line x1='{L}' x2='{W-R}' y1='{y:.1f}' y2='{y:.1f}' stroke='{GRID}'/>")
        out.append(f"<text x='{L-8}' y='{y+4:.1f}' text-anchor='end' fill='{INK2}'>{10**e:g}</text>")
    for e in range(2, 6):
        for m in (1, 2, 5):
            v = m * 10 ** e
            if 10 ** x0 <= v <= 10 ** x1:
                x = X(v)
                out.append(f"<line y1='{T}' y2='{H-B}' x1='{x:.1f}' x2='{x:.1f}' stroke='{GRID}'/>")
                out.append(f"<text x='{x:.1f}' y='{H-B+18}' text-anchor='middle' fill='{INK2}'>{v:,}</text>".replace(",", " "))
    out.append(f"<text x='{(L+W-R)/2:.0f}' y='{H-14}' text-anchor='middle' fill='{INK}'>Įvesties dydis, baitais (log skalė)</text>")
    out.append(f"<text transform='translate(18 {(T+H-B)/2:.0f}) rotate(-90)' text-anchor='middle' fill='{INK}'>Laikas vienai maišai, µs (log skalė)</text>")
    # serijos: min–max juosta (plona) ir vidurkio linija su taškais
    for n in names:
        pts = sorted(series[n])
        c = COLORS[n]
        band = " ".join(f"{X(b):.1f},{Y(mx):.1f}" for b, _, _, mx in pts) + " " + " ".join(f"{X(b):.1f},{Y(mn):.1f}" for b, _, mn, _ in reversed(pts))
        out.append(f"<polygon points='{band}' fill='{c}' fill-opacity='0.15' stroke='none'/>")
        path = " ".join(f"{X(b):.1f},{Y(m):.1f}" for b, m, _, _ in pts)
        out.append(f"<polyline points='{path}' fill='none' stroke='{c}' stroke-width='2' stroke-linejoin='round'/>")
        for b, m, _, _ in pts:
            out.append(f"<circle cx='{X(b):.1f}' cy='{Y(m):.1f}' r='3.5' fill='{c}' stroke='{SURFACE}' stroke-width='1.5'/>")
    # tiesioginės etiketės linijų galuose + legenda
    ends = sorted(((Y(max(series[n])[1]), n) for n in names))
    last_y = -100
    for y, n in ends:
        y = max(y, last_y + 15)
        last_y = y
        out.append(f"<text x='{W-R+8}' y='{y+4:.1f}' fill='{INK}'>{esc(n)}</text>")
        out.append(f"<line x1='{W-R+2}' x2='{W-R+6}' y1='{y:.1f}' y2='{y:.1f}' stroke='{COLORS[n]}' stroke-width='2'/>")
    out.append(f"<text x='{L}' y='38' fill='{INK2}' font-size='11'>vidurkis – linija, min–max – juosta; 7 matavimai, MSVC /O2</text>")
    out.append("</svg>")
    open(os.path.join(RESULTS, "exp4_speed.svg"), "w", encoding="utf-8").write("\n".join(out))


def histogram_chart():
    rows = list(csv.DictReader(open(os.path.join(RESULTS, "exp6_hist.csv"), encoding="utf-8")))
    panels = ["simbolis", "bitas"]
    titles = {"simbolis": "Pakeistas vienas simbolis", "bitas": "Apverstas vienas bitas"}
    data = {p: {} for p in panels}
    bits = 256
    for r in rows:
        if r["hash"] == "Ratas v0.1" and r["mode"] in data:
            data[r["mode"]][int(r["bin_bits"])] = int(r["count"])
            bits = int(r["bits"])
    lo, hi = 90, 166  # bitų (35 %..65 %)
    total = {p: sum(data[p].values()) for p in panels}
    ymax = max(v for p in panels for v in data[p].values()) * 1.15
    # Idealios (binominės, p = 0.5) tikimybės: C(n,k) / 2^n
    binom = [math.comb(bits, k) / 2 ** bits for k in range(bits + 1)]

    W, H = 760, 320
    PW = (W - 60) // 2
    out = [f"<svg xmlns='http://www.w3.org/2000/svg' width='{W}' height='{H}' viewBox='0 0 {W} {H}' {FONT} font-size='12'>",
           f"<rect width='{W}' height='{H}' fill='{SURFACE}'/>",
           f"<text x='40' y='22' font-size='15' font-weight='600' fill='{INK}'>Ratas v0.1: bitų skirtumo pasiskirstymas (100 000 porų kiekvienam būdui)</text>",
           f"<text x='40' y='38' font-size='11' fill='{INK2}'>brūkšninė linija – ideali binominė B(256, ½)</text>"]
    for i, p in enumerate(panels):
        L = 40 + i * (PW + 20)
        T, B, R = 62, 50, 10
        pw, ph = PW - R, H - T - B
        def X(v): return L + 50 + (v - lo) / (hi - lo) * (pw - 50)
        def Y(v): return T + (1 - v / ymax) * ph
        out.append(f"<text x='{L+50}' y='{T-8}' fill='{INK}' font-weight='600'>{esc(titles[p])}</text>")
        step = max(1, int(ymax // 5 // 500) * 500)
        v = 0
        while v <= ymax:
            out.append(f"<line x1='{L+50}' x2='{L+pw}' y1='{Y(v):.1f}' y2='{Y(v):.1f}' stroke='{GRID}'/>")
            out.append(f"<text x='{L+44}' y='{Y(v)+4:.1f}' text-anchor='end' fill='{INK2}' font-size='11'>{v:,}</text>".replace(",", " "))
            v += step
        bw = (pw - 50) / (hi - lo)
        for b in range(lo, hi + 1):
            cnt = data[p].get(b, 0)
            if cnt == 0:
                continue
            x, y = X(b) - bw / 2 + 0.5, Y(cnt)
            out.append(f"<rect x='{x:.1f}' y='{y:.1f}' width='{max(bw-1, 1):.1f}' height='{Y(0)-y:.1f}' fill='{MODE_COLORS[p]}'/>")
        ideal = " ".join(f"{X(k):.1f},{Y(binom[k] * total[p]):.1f}" for k in range(lo, hi + 1))
        out.append(f"<polyline points='{ideal}' fill='none' stroke='{INK}' stroke-width='1.5' stroke-dasharray='4 3'/>")
        for pct in (40, 45, 50, 55, 60):
            k = pct / 100 * bits
            out.append(f"<text x='{X(k):.1f}' y='{H-B+16}' text-anchor='middle' fill='{INK2}'>{pct} %</text>")
        mean = sum(b * c for b, c in data[p].items()) / max(1, total[p]) / bits * 100
        out.append(f"<text x='{L+pw}' y='{T-8}' text-anchor='end' fill='{INK2}' font-size='11'>vid. {mean:.2f} %</text>")
        out.append(f"<text x='{(L+50+L+pw)/2:.0f}' y='{H-10}' text-anchor='middle' fill='{INK}'>Besiskiriančių bitų dalis iš 256, %</text>")
    out.append("</svg>")
    open(os.path.join(RESULTS, "exp6_histogram.svg"), "w", encoding="utf-8").write("\n".join(out))


if __name__ == "__main__":
    speed_chart()
    histogram_chart()
    print("SVG: results/exp4_speed.svg, results/exp6_histogram.svg")
