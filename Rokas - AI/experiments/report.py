#!/usr/bin/env python3
"""Builds the experiment tables (Markdown) and charts (SVG) from results/raw/*.csv."""

import csv
import math
import os
import re
import sys
from collections import defaultdict

RAW, OUT = sys.argv[1], sys.argv[2]
IMPLS = ["v2", "v1", "ratas"]
NAMES = {"v2": "2 versija", "v1": "1 versija", "ratas": "Ratas v0.1"}
SHORT = "2 v. / 1 v. / Ratas"

LIGHT = {"bg": "#fcfcfb", "text": "#0b0b0b", "text2": "#52514e", "grid": "#e4e3de",
         "v2": "#2a78d6", "v1": "#eb6834", "ratas": "#1baf7a"}
DARK = {"bg": "#1a1a19", "text": "#ffffff", "text2": "#c3c2b7", "grid": "#353533",
        "v2": "#3987e5", "v1": "#d95926", "ratas": "#199e70"}


def rows(name):
    with open(os.path.join(RAW, name + ".csv"), newline="") as f:
        return [r for r in csv.reader(f) if r]


def num(x, d=2):
    s = f"{x:,.{d}f}".replace(",", " ").replace(".", ",")
    return s


def whole(x):
    return f"{int(x):,}".replace(",", " ")


def write(name, lines):
    with open(os.path.join(OUT, name), "w") as f:
        f.write("\n".join(lines).rstrip() + "\n")


def table(header, body):
    out = ["| " + " | ".join(header) + " |", "|" + "|".join("---" for _ in header) + "|"]
    out += ["| " + " | ".join(str(c) for c in r) + " |" for r in body]
    return out


def svg_style():
    def block(p):
        return (f".bg{{fill:{p['bg']}}} .t{{fill:{p['text']}}} .t2{{fill:{p['text2']}}} "
                f".grid{{stroke:{p['grid']}}} .axis{{stroke:{p['text2']}}} "
                + " ".join(f".f-{k}{{fill:{p[k]}}} .s-{k}{{stroke:{p[k]}}}" for k in IMPLS)
                + f" .ring{{stroke:{p['bg']}}} .ref{{stroke:{p['text2']}}}")
    return (f"<style>text{{font-family:system-ui,-apple-system,'Segoe UI',sans-serif}} {block(LIGHT)} "
            f"@media (prefers-color-scheme: dark){{{block(DARK)}}}</style>")


# ---------------------------------------------------------------- 1-3
DESC = {
    "empty.bin": "tuščias failas",
    "a.bin": "vienas baitas `a`, be naujos eilutės",
    "b.bin": "vienas baitas `b`, be naujos eilutės",
    "struct_len15.txt": "`x` × 15",
    "struct_len16.txt": "`x` × 16",
    "struct_len17.txt": "`x` × 17",
    "struct_newline_lf.txt": "`tekstas` + LF",
    "struct_newline_crlf.txt": "pagal pavadinimą CRLF, bet faile LF (baitai kaip `_lf`)",
    "crlf_atmintyje": "`tekstas` + CRLF (sudaryta atmintyje)",
    "struct_order_abc.txt": "`abc`",
    "struct_order_cba.txt": "`cba`",
    "struct_order_words1.txt": "`labas rytas`",
    "struct_order_words2.txt": "`rytas labas`",
    "struct_pad_ab.txt": "`ab`",
    "struct_pad_ab0.txt": "`ab` + nulinis baitas",
    "struct_repeat_a.txt": "`a` × 32",
    "struct_repeat_ab.txt": "`ab` × 16",
    "struct_space_none.txt": "`tekstas`",
    "struct_space_none_copy.txt": "`tekstas`, kitas failo vardas",
    "struct_space_lead.txt": "tarpas pradžioje",
    "struct_space_trail.txt": "tarpas gale",
    "utf8_lt.txt": "lietuviškos raidės (UTF-8)",
    "utf8_mixed.txt": "ASCII, brūkšnys ir € (UTF-8)",
}
for k in (1, 2, 3):
    DESC[f"random_{k}.txt"] = "atsitiktinis ASCII `!`..`~`"
    for p, lt in (("start", "pradžioje"), ("middle", "viduryje"), ("end", "gale")):
        DESC[f"random_{k}_{p}.txt"] = f"`random_{k}`, pakeistas 1 baitas {lt}"

COMPARE = [(f"random_{k}_{p}.txt", f"random_{k}.txt", True) for k in (1, 2, 3) for p in ("start", "middle", "end")] + [
    ("b.bin", "a.bin", True),
    ("struct_order_cba.txt", "struct_order_abc.txt", True),
    ("struct_order_words2.txt", "struct_order_words1.txt", True),
    ("struct_pad_ab0.txt", "struct_pad_ab.txt", True),
    ("struct_space_lead.txt", "struct_space_none.txt", True),
    ("struct_space_trail.txt", "struct_space_none.txt", True),
    ("struct_newline_lf.txt", "struct_space_none.txt", True),
    ("crlf_atmintyje", "struct_newline_lf.txt", True),
    ("struct_len16.txt", "struct_len15.txt", True),
    ("struct_len17.txt", "struct_len16.txt", True),
    ("struct_repeat_ab.txt", "struct_repeat_a.txt", True),
    ("struct_space_none_copy.txt", "struct_space_none.txt", False),
    ("struct_newline_crlf.txt", "struct_newline_lf.txt", False),
]


def report_inputs():
    data = rows("inputs")
    hexes = defaultdict(dict)
    info, fmt, rep, other = {}, defaultdict(int), defaultdict(int), defaultdict(dict)
    count = defaultdict(int)
    leadex = []
    for r in data:
        if r[0] == "input":
            _, impl, name, size, chars, hx, f_ok, r_ok = r
            hexes[impl][name] = hx
            info[name] = (int(size), int(chars))
            count[impl] += 1
            fmt[impl] += int(f_ok)
            rep[impl] += int(r_ok)
        elif r[0] == "aba":
            other[r[1]]["aba"] = r[2] == "1"
        elif r[0] == "repeat":
            other[r[1]]["repeat"] = (r[2], r[4] == "1")
        elif r[0] == "lead":
            other[r[1]]["lead"] = (int(r[3]), int(r[4]))
        elif r[0] == "leadex" and r[1] == "v2":
            leadex.append((r[2], r[3]))

    out = ["# 1–3 eksperimentai: įvestys, formatas, determinizmas", "",
           "Įvestys – bendras poros rinkinys `Joringis-no AI/data/exp1/` (seed 20260920, abėcėlė `!`..`~`) "
           "ir papildomas tikras CRLF atvejis, sudarytas atmintyje.", ""]
    names = sorted(info, key=lambda n: (n.startswith("crlf"), n))
    out += table(["Įvestis", "Baitai", "Simboliai (UTF-8)", "Aprašymas", "2 versijos maiša (pradžia)"],
                 [[f"`{n}`", info[n][0], info[n][1], DESC.get(n, ""), f"`{hexes['v2'][n][:16]}…`"] for n in names])
    out += ["", "## Palyginimai poromis", ""]
    body = []
    for a, b, differ in COMPARE:
        cells = []
        for impl in IMPLS:
            same = hexes[impl][a] == hexes[impl][b]
            ok = same != differ
            cells.append(("skiriasi" if not same else "sutampa") + (" ✓" if ok else " ✗"))
        body.append([f"`{a}` ↔ `{b}`", "skiriasi" if differ else "sutampa"] + cells)
    out += table(["Pora", "Tikimasi"] + [NAMES[i] for i in IMPLS], body)
    out += ["", "`struct_newline_crlf.txt` repozitorijoje saugomas su LF, todėl jo baitai sutampa su `_lf` ir "
            "maišos turi sutapti. Tikras CRLF atvejis patikrintas eilute `crlf_atmintyje`.", "",
            "## Formatas ir determinizmas", ""]
    body = [
        ["64 hex simboliai, vienodas raidžių dydis, dekoduojasi į tą pačią maišą"] + [f"{fmt[i]}/{count[i]}" for i in IMPLS],
        ["3 kartotiniai kvietimai duoda tą pačią maišą"] + [f"{rep[i]}/{count[i]}" for i in IMPLS],
        ["Seka A, B, A (A sutampa, B skiriasi)"] + ["taip" if other[i]["aba"] else "ne" for i in IMPLS],
        [f"1 000 kvietimų su `{other['v2']['repeat'][0]}`"] + ["taip" if other[i]["repeat"][1] else "ne" for i in IMPLS],
        ["Maišos, prasidedančios `0`, iš `0000`–`9999` (tikėtina ≈ 625)"] + [str(other[i]["lead"][0]) for i in IMPLS],
        ["Maišos, prasidedančios `00` (tikėtina ≈ 39)"] + [str(other[i]["lead"][1]) for i in IMPLS],
    ]
    out += table(["Patikra"] + [NAMES[i] for i in IMPLS], body)
    if leadex:
        out += ["", "Pradiniai nuliai išsaugomi (2 versija):", ""]
        out += [f"* `{c}` → `{h}` ({len(h)} simboliai)" for c, h in leadex]

    cli = rows("cli")
    tally = defaultdict(lambda: [0, 0])
    for kind, name, *rest in cli:
        tally[kind][0] += rest[-1] == "1"
        tally[kind][1] += 1
        if kind == "runs":
            tally["same"][0] += rest[0] == hexes["v2"].get(name)
            tally["same"][1] += 1
    out += ["", "## Komandinė eilutė (2 versija)", ""]
    out += table(["Patikra", "Rezultatas"], [
        ["Du atskiri programos paleidimai su `--file` duoda tą pačią maišą", "{}/{}".format(*tally["runs"])],
        ["`--file` maiša sutampa su maiša, apskaičiuota programos viduje", "{}/{}".format(*tally["same"])],
        ["Ranka įvestas tekstas (+ Enter) sutampa su failo maiša", "{}/{}".format(*tally["typed"])],
        ["`--text` sutampa su failo maiša", "{}/{}".format(*tally["text"])],
    ])
    out += ["", "Ranka ir `--text` tikrinami failai be naujos eilutės ir nulinių baitų – tik tokius galima įvesti viena eilute."]
    write("exp1_3_teisingumas.md", out)


# ---------------------------------------------------------------- 4
def report_speed():
    groups = defaultdict(list)
    size, reps = {}, {}
    for _, impl, lines, nbytes, run, rp, total, per in rows("speed"):
        groups[(impl, int(lines))].append(float(per) / 1000)
        size[int(lines)] = int(nbytes)
        reps[(impl, int(lines))] = int(rp)
    lines_list = sorted(size)
    out = ["# 4 eksperimentas: sparta", "",
           "Failas `konstitucija.txt` (bendras poros failas), ištraukos po 1, 2, 4, … eilučių su eilučių skirtukais ir visas failas. "
           "Kiekviena ištrauka paruošiama iš anksto; matuojamas tik maišos skaičiavimas (be failų I/O ir išvedimo). "
           "`std::chrono::steady_clock`, 3 apšilimo matavimai, tada 10 matavimų kiekvienam dydžiui. Viename matavime maiša "
           "kviečiama tiek kartų, kad jis truktų ≥ 20 ms, ir laikas dalijamas iš kvietimų skaičiaus. Rezultatas naudojamas "
           "(`volatile`), o maišos funkcija yra atskirame vertimo vienete, todėl kompiliatorius skaičiavimo neišmeta.", "",
           "Laikas vienai maišai, µs: vidurkis (min–max).", ""]
    body = []
    for n in lines_list:
        cells = []
        for impl in IMPLS:
            v = groups[(impl, n)]
            cells.append(f"{num(sum(v) / len(v), 3)} ({num(min(v), 3)}–{num(max(v), 3)})")
        body.append([n, whole(size[n])] + cells)
    out += table(["Eilutės", "Baitai"] + [NAMES[i] for i in IMPLS], body)
    full = lines_list[-1]
    out += ["", "Pralaidumas visam failui: " + ", ".join(
        f"{NAMES[i]} – {num(size[full] / (sum(groups[(i, full)]) / len(groups[(i, full)])) , 0)} MB/s" for i in IMPLS) + "."]
    out += ["", "Neapdoroti matavimai: `raw/speed.csv` (kiekvienas matavimas, kvietimų skaičius, bendras laikas)."]
    write("exp4_sparta.md", out)
    speed_svg(groups, size, lines_list)


def speed_svg(groups, size, lines_list):
    W, H, L, R, T, B = 760, 440, 78, 120, 64, 62
    xs = [size[n] for n in lines_list]
    ys = [sum(v) / len(v) for v in groups.values()]
    x0, x1 = math.floor(math.log10(min(xs))), math.ceil(math.log10(max(xs)))
    y0, y1 = math.floor(math.log10(min(ys))), math.ceil(math.log10(max(ys)))
    px = lambda b: L + (math.log10(b) - x0) / (x1 - x0) * (W - L - R)
    py = lambda us: H - B - (math.log10(us) - y0) / (y1 - y0) * (H - T - B)
    s = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" viewBox="0 0 {W} {H}" role="img" '
         f'aria-label="Vienos maišos skaičiavimo laikas pagal įvesties dydį">', svg_style(),
         f'<rect class="bg" fill="{LIGHT["bg"]}" width="{W}" height="{H}"/>',
         f'<text class="t" x="{L}" y="26" font-size="16" font-weight="600">Vienos maišos skaičiavimo laikas pagal įvesties dydį</text>']
    lx = L
    for impl in IMPLS:
        s.append(f'<rect class="f-{impl}" fill="{LIGHT[impl]}" x="{lx}" y="38" width="12" height="12" rx="3"/>'
                 f'<text class="t2" x="{lx + 17}" y="48" font-size="12">{NAMES[impl]}</text>')
        lx += 110
    for e in range(x0, x1 + 1):
        x = px(10 ** e)
        s.append(f'<line class="grid" stroke="{LIGHT["grid"]}" x1="{x:.1f}" y1="{T}" x2="{x:.1f}" y2="{H - B}"/>'
                 f'<text class="t2" x="{x:.1f}" y="{H - B + 18}" font-size="12" text-anchor="middle">{whole(10 ** e)}</text>')
    for e in range(y0, y1 + 1):
        y = py(10 ** e)
        label = num(10 ** e, max(0, -e))
        s.append(f'<line class="grid" stroke="{LIGHT["grid"]}" x1="{L}" y1="{y:.1f}" x2="{W - R}" y2="{y:.1f}"/>'
                 f'<text class="t2" x="{L - 8}" y="{y + 4:.1f}" font-size="12" text-anchor="end">{label}</text>')
    s.append(f'<text class="t2" x="{(L + W - R) / 2}" y="{H - 16}" font-size="13" text-anchor="middle">'
             f'Įvesties dydis, baitai (log. skalė)</text>')
    s.append(f'<text class="t2" transform="translate(20 {(T + H - B) / 2}) rotate(-90)" font-size="13" text-anchor="middle">'
             f'Laikas vienai maišai, µs (log. skalė)</text>')
    for impl in IMPLS:
        pts = [(px(size[n]), py(sum(groups[(impl, n)]) / len(groups[(impl, n)])), n) for n in lines_list]
        path = " ".join(f"{'M' if i == 0 else 'L'}{x:.1f},{y:.1f}" for i, (x, y, _) in enumerate(pts))
        s.append(f'<path class="s-{impl}" stroke="{LIGHT[impl]}" d="{path}" fill="none" stroke-width="2" stroke-linejoin="round"/>')
        for x, y, n in pts:
            v = groups[(impl, n)]
            s.append(f'<circle class="f-{impl} ring" fill="{LIGHT[impl]}" stroke="{LIGHT["bg"]}" stroke-width="2" cx="{x:.1f}" cy="{y:.1f}" r="4.5">'
                     f'<title>{NAMES[impl]}: {whole(size[n])} B – {num(sum(v) / len(v), 3)} µs</title></circle>')
        lx_, ly_, _ = pts[-1]
        s.append(f'<text class="t" x="{lx_ + 10:.1f}" y="{ly_ + 4:.1f}" font-size="12">{NAMES[impl]}</text>')
    s.append("</svg>")
    with open(os.path.join(OUT, "exp4_sparta.svg"), "w") as f:
        f.write("\n".join(s) + "\n")


# ---------------------------------------------------------------- 5
def report_collisions():
    pairs, trunc, examples = {}, defaultdict(lambda: [0, 0.0]), []
    for r in rows("collisions"):
        if r[0] == "pairs":
            _, impl, L, n, regen, pc, count, distinct, groups = r
            pairs[(impl, int(L))] = (int(n), int(regen), int(pc), int(count), int(distinct), int(groups))
        elif r[0] == "trunc":
            _, impl, L, bits, distinct, found, expected = r
            trunc[(impl, int(bits))][0] += int(found)
            trunc[(impl, int(bits))][1] += float(expected)
        elif r[0] == "example":
            examples.append(r)
    struct = defaultdict(dict)
    order = []
    for _, impl, cat, total, distinct, groups in rows("structured"):
        if cat not in order:
            order.append(cat)
        struct[cat][impl] = (int(total), int(distinct), int(groups))

    lengths = sorted({k[1] for k in pairs})
    out = ["# 5 eksperimentas: kolizijos", "",
           "Kiekvienam ilgiui 100 000 atsitiktinių porų iš abėcėlės `!`..`~` (94 ASCII simboliai, 1 simbolis = 1 baitas), "
           "`std::mt19937_64`, seed = 20260920 + ilgis, simbolis = `'!' + (x mod 94)`. Jei poros narės sutampa, antroji "
           "generuojama iš naujo. Kolizija skaičiuojama tik tarp skirtingų įvesčių. Visos trys realizacijos maišo tas pačias įvestis.", "",
           f"Reikšmės langeliuose: {SHORT}.", ""]
    body = []
    for L in lengths:
        n, regen, _, count, distinct, _ = pairs[("v2", L)]
        body.append([L, whole(n), whole(distinct),
                     " / ".join(str(pairs[(i, L)][2]) for i in IMPLS),
                     " / ".join(str(pairs[(i, L)][5]) for i in IMPLS)])
    out += table(["Ilgis", "Porų", "Skirtingų įvesčių rinkinyje", "Kolizijos porose", "Kolizijų grupės visame rinkinyje"], body)
    if examples:
        out += ["", "Rastos kolizijos:", ""] + [f"* {NAMES[e[1]]}, ilgis {e[2]}: `{e[4]}` ir `{e[5]}` → `{e[3]}`" for e in examples]

    out += ["", "## Kodėl kolizijų nerandama ir ar testas jas pastebėtų", "",
            "Idealiai n bitų maišai vienos poros kolizijos tikimybė ≈ 2^(−n), o m įvesčių rinkinyje yra m(m−1)/2 porų. "
            "Kai m = 200 000 ir n = 256, tikėtinas kolizijų skaičius ≈ 2·10^10 · 2^(−256) ≈ 10^(−67), todėl nulis yra "
            "įprastas rezultatas ir apie saugumą nieko neįrodo. Kad matytųsi, jog pats testas kolizijas randa, tie patys "
            "rinkiniai patikrinti su iki 24, 32 ir 40 bitų sutrumpintomis maišomis (sumuota per visus keturis ilgius):", ""]
    body = []
    for bits in (24, 32, 40):
        body.append([f"{bits} bitai", num(trunc[("v2", bits)][1], 2)] + [whole(trunc[(i, bits)][0]) for i in IMPLS])
    out += table(["Sutrumpinta iki", "Tikėtina"] + [NAMES[i] for i in IMPLS], body)
    out += ["", "Sutrumpintų maišų kolizijų skaičius atitinka gimtadienio paradokso įvertį, t. y. pagal šį matą maišos "
            "elgiasi kaip atsitiktinės.", "", "## Struktūruotos įvestys", "",
            f"Kolizijų grupės langeliuose: {SHORT}.", ""]
    body = [[c, whole(struct[c]["v2"][0]), whole(struct[c]["v2"][1]), " / ".join(str(struct[c][i][2]) for i in IMPLS)] for c in order]
    out += table(["Rinkinys", "Įvesčių", "Skirtingų", "Kolizijų grupės"], body)
    write("exp5_kolizijos.md", out)


# ---------------------------------------------------------------- 6
def binom_pmf(k, n=256):
    return math.comb(n, k) / 2 ** n


def report_avalanche():
    stats = defaultdict(dict)
    hist = defaultdict(dict)
    for r in rows("avalanche"):
        if r[0] == "aval":
            _, impl, mode, L, n, bmin, bmax, bmean, bsd, hmin, hmax, hmean, hsd = r
            stats[(impl, mode)][L] = [int(n)] + [float(v) for v in (bmin, bmax, bmean, bsd, hmin, hmax, hmean, hsd)]
        elif r[0] == "hist":
            _, impl, mode, bits, count = r
            hist[(impl, mode)][int(bits)] = int(count)
    lengths = [k for k in stats[("v2", "simbolis")] if k != "visi"]
    out = ["# 6 eksperimentas: lavinos efektas", "",
           "100 000 porų: po 25 000 ilgiams 10, 100, 500 ir 1 000. Kiekvienoje poroje vienas atsitiktinai parinktas simbolis "
           "pakeistas kitu tos pačios abėcėlės `!`..`~` simboliu, ilgis nekinta (`std::mt19937_64`, seed = 20260920 + 100 + ilgis). "
           "Prieš lyginant bitus abi hex maišos dekoduojamos į baitus. Orientyrai: ≈ 50 % bitų ir ≈ 93,75 % hex skaitmenų.", "",
           "## Bitų skirtumas, %: vidurkis (min–max)", ""]

    def cell(v, i):
        return f"{num(v[i + 2], 2)} ({num(v[i], 2)}–{num(v[i + 1], 2)})"

    body = [[L] + [cell(stats[(impl, "simbolis")][L], 1) for impl in IMPLS] for L in lengths + ["visi"]]
    out += table(["Ilgis"] + [NAMES[i] for i in IMPLS], body)
    out += ["", "## Hex skirtumas, %: vidurkis (min–max)", ""]
    body = [[L] + [cell(stats[(impl, "simbolis")][L], 5) for impl in IMPLS] for L in lengths + ["visi"]]
    out += table(["Ilgis"] + [NAMES[i] for i in IMPLS], body)
    out += ["", "Bitų skirtumo standartinis nuokrypis (visos poros): " + ", ".join(
        f"{NAMES[i]} – {num(stats[(i, 'simbolis')]['visi'][4], 2)} %" for i in IMPLS)
        + ". Idealiam atsitiktiniam atvejiui – √(256·0,25)/256 = 3,13 %.", "",
        "## Papildomai: apverstas tiksliai vienas įvesties bitas", "",
        "Tie patys ilgiai ir porų skaičius, bet pakeičiamas vienas bitas (baitų režimu, nebūtinai ASCII; seed = 20260920 + 200 + ilgis).", ""]
    body = [["bitų skirtumas, %"] + [cell(stats[(i, "bitas")]["visi"], 1) for i in IMPLS],
            ["hex skirtumas, %"] + [cell(stats[(i, "bitas")]["visi"], 5) for i in IMPLS]]
    out += table(["Visos 100 000 porų"] + [NAMES[i] for i in IMPLS], body)
    out += ["", "![Bitų skirtumo histograma](exp6_histograma.svg)", "",
            "Pilka linija – binominis pasiskirstymas B(256; 0,5), kurio tikėtųsi iš idealiai atsitiktinės maišos. "
            "Histogramos duomenys: `raw/avalanche.csv` (eilutės `hist`)."]
    write("exp6_lavina.md", out)
    histogram_svg(hist)


def histogram_svg(hist):
    W, H, T, B = 900, 384, 92, 56
    pw, gap, L = 250, 34, 60
    bin_w = 2
    ks = [k for impl in IMPLS for k in hist[(impl, "simbolis")]]
    k0 = (min(ks) // bin_w) * bin_w - bin_w
    k1 = (max(ks) // bin_w + 2) * bin_w
    bins = list(range(k0, k1, bin_w))
    counts = {impl: [sum(hist[(impl, "simbolis")].get(k + j, 0) for j in range(bin_w)) for k in bins] for impl in IMPLS}
    total = sum(counts["v2"])
    expected = [total * sum(binom_pmf(k + j) for j in range(bin_w)) for k in bins]
    ymax = max(max(max(c) for c in counts.values()), max(expected)) * 1.08
    step = 10 ** math.floor(math.log10(ymax / 4))
    step *= 2 if ymax / step > 8 else 1
    s = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" viewBox="0 0 {W} {H}" role="img" '
         f'aria-label="Bitų skirtumo histograma">', svg_style(),
         f'<rect class="bg" fill="{LIGHT["bg"]}" width="{W}" height="{H}"/>',
         f'<text class="t" x="{L}" y="26" font-size="16" font-weight="600">Bitų skirtumo pasiskirstymas '
         f'(100 000 porų, pakeistas vienas simbolis)</text>',
         f'<line class="ref" stroke="{LIGHT["text2"]}" x1="{L}" y1="44" x2="{L + 22}" y2="44" stroke-width="2"/>'
         f'<text class="t2" x="{L + 28}" y="48" font-size="12">tikėtina idealiai maišai B(256; 0,5)</text>']
    for p, impl in enumerate(IMPLS):
        ox = L + p * (pw + gap)
        px = lambda k: ox + (k - k0) / (k1 - k0) * pw
        py = lambda c: H - B - c / ymax * (H - T - B)
        s.append(f'<text class="t" x="{ox}" y="{T - 10}" font-size="13" font-weight="600">{NAMES[impl]}</text>')
        y = 0
        while y <= ymax:
            yy = py(y)
            s.append(f'<line class="grid" stroke="{LIGHT["grid"]}" x1="{ox}" y1="{yy:.1f}" x2="{ox + pw}" y2="{yy:.1f}"/>')
            if p == 0:
                s.append(f'<text class="t2" x="{ox - 6}" y="{yy + 4:.1f}" font-size="11" text-anchor="end">{whole(y)}</text>')
            y += step
        for pct in range(math.ceil(k0 / 2.56 / 5) * 5, int(k1 / 2.56) + 1, 5):
            xx = px(pct * 2.56)
            s.append(f'<text class="t2" x="{xx:.1f}" y="{H - B + 16}" font-size="11" text-anchor="middle">{pct} %</text>')
        bw = pw / len(bins)
        for k, c in zip(bins, counts[impl]):
            if not c:
                continue
            h = c / ymax * (H - T - B)
            x = px(k) + 1
            s.append(f'<rect class="f-{impl}" fill="{LIGHT[impl]}" x="{x:.1f}" y="{H - B - h:.1f}" width="{max(bw - 2, 1):.1f}" '
                     f'height="{h:.1f}" rx="1"><title>{NAMES[impl]}: {k}–{k + bin_w - 1} bitų skiriasi – {whole(c)} porų</title></rect>')
        path = " ".join(f"{'M' if i == 0 else 'L'}{px(k + bin_w / 2):.1f},{py(e):.1f}" for i, (k, e) in enumerate(zip(bins, expected)))
        s.append(f'<path class="ref" stroke="{LIGHT["text2"]}" d="{path}" fill="none" stroke-width="2"/>')
        s.append(f'<line class="axis" stroke="{LIGHT["text2"]}" x1="{ox}" y1="{H - B}" x2="{ox + pw}" y2="{H - B}"/>')
    s.append(f'<text class="t2" x="{W / 2}" y="{H - 14}" font-size="13" text-anchor="middle">Pasikeitusių maišos bitų dalis</text>')
    s.append(f'<text class="t2" transform="translate(16 {(T + H - B) / 2}) rotate(-90)" font-size="13" text-anchor="middle">Porų skaičius</text>')
    s.append("</svg>")
    with open(os.path.join(OUT, "exp6_histograma.svg"), "w") as f:
        f.write("\n".join(s) + "\n")


# ---------------------------------------------------------------- 7
def report_guess():
    d = defaultdict(lambda: defaultdict(list))
    for r in rows("guess"):
        d[r[1]][r[0]].append(r[2:])
    nosalt = {i: d[i]["nosalt"][0] for i in IMPLS}
    tab = {i: d[i]["table"][0] for i in IMPLS}
    out = ["# 7 eksperimentas: spėjimas, vieša druska ir slaptas atsitiktinumas", "",
           "Kandidatai – visos keturių skaitmenų eilutės `0000`–`9999` (10 000, po 4 baitus, be naujos eilutės). "
           "Tikslinė įvestis parinkta `std::mt19937_64` (seed 20260927); atakai pateikiama tik maiša ir kandidatų rinkinys.", "",
           "## Be druskos: H(input)", ""]
    out += table(["", *[NAMES[i] for i in IMPLS]], [
        ["Tikslinė įvestis"] + [f"`{nosalt[i][0]}`" for i in IMPLS],
        ["Bandymų iki radimo"] + [whole(nosalt[i][1]) for i in IMPLS],
        ["Patikrinta iš viso (visi sutapimai)"] + [whole(nosalt[i][2]) for i in IMPLS],
        ["Sutampantys kandidatai"] + [f"`{nosalt[i][3]}`" for i in IMPLS],
        ["Viso perrinkimo laikas, ms"] + [num(float(nosalt[i][4]) / 1000, 2) for i in IMPLS],
    ])
    out += ["", "Kandidatų rinkinyje rastas tiksliai vienas sutapimas, todėl čia jis identifikuoja įvestį. Bendru atveju "
            "sutapimas to neįrodo: kolizijos neišvengiamai egzistuoja, o tikroji įvestis gali būti ir už rinkinio ribų.", "",
            "Be druskos vieną kartą apskaičiuota 10 000 maišų lentelė tinka visiems taikiniams:", ""]
    out += table(["", *[NAMES[i] for i in IMPLS]], [
        ["Taikinių"] + [tab[i][0] for i in IMPLS],
        ["Maišos skaičiavimų"] + [whole(tab[i][1]) for i in IMPLS],
        ["Laikas (lentelė + paieška), ms"] + [num(float(tab[i][2]) / 1000, 2) for i in IMPLS],
        ["Atspėta"] + [f"{tab[i][3]}/{tab[i][0]}" for i in IMPLS],
    ])
    out += ["", "## Vieša druska: H(input || salt)", "",
            "Kiekvienam taikiniui – atskira atsitiktinė 16 baitų druska (`std::mt19937_64`), pridedama po įvesties kaip "
            "tikslūs baitai (ne hex tekstas); lentelėje druska parodyta hex. Užpuolikas druską žino.", ""]
    body = []
    salts = d["v2"]["salt"]
    for j, row in enumerate(salts):
        idx, target, salt, *_ = row
        body.append([idx, f"`{target}`", f"`{salt}`"] + [
            f"{whole(d[i]['salt'][j][3])} / {d[i]['salt'][j][4]} / {num(float(d[i]['salt'][j][5]) / 1000, 2)}" for i in IMPLS])
    out += table(["#", "Taikinys", "Druska"] + [f"{NAMES[i]}: bandymai / sutapimai / ms" for i in IMPLS], body)
    total_salt = 10000 * len(salts)
    out += ["", f"Su druska kiekvienam taikiniui tenka atskiras perrinkimas: {len(salts)} taikiniams – {whole(total_salt)} "
            f"maišos skaičiavimų vietoj {whole(10000)}. Vienam taikiniui druska darbo nepadidina (vis tiek ≤ 10 000 bandymų), "
            "bet iš anksto apskaičiuotos lentelės nebegalima panaudoti kitiems taikiniams su kita druska.", "",
            "## Slaptas atsitiktinumas: H(input || r)", ""]
    c = d["v2"]["commit"][0]
    out += [f"Įsipareigojimas `c = H(\"{c[0]}\" || r)`, kur r – 16 slaptų baitų. Kol r nežinomas, kiekvienam kandidatui "
            "reikėtų perrinkti 2^128 r reikšmių, t. y. 10 000 · 2^128 variantų – tai neatliekama. Atskleidus r, "
            "patikrinti užtenka vieno maišos skaičiavimo:", ""]
    out += table(["", *[NAMES[i] for i in IMPLS]], [
        ["`H(input || r) == c` su teisinga įvestimi"] + ["taip" if d[i]["commit"][0][2] == "1" else "ne" for i in IMPLS],
        ["kita įvestis su tuo pačiu r atmetama"] + ["taip" if d[i]["commit"][0][3] == "1" else "ne" for i in IMPLS],
    ])
    out += ["", "Tai iliustruoja įsipareigojimo (commitment) idėją, bet neįrodo, kad ši konstrukcija saugiai paslepia "
            "pranešimą ar neleidžia jo vėliau pakeisti. Tai ir ne darbo įrodymo (proof-of-work) galvosūkis: ten ieškoma "
            "nonce, su kuria maiša tenkina sąlygą, o čia – konkreti įvestis iš mažo rinkinio."]
    write("exp7_spejimas.md", out)


# ---------------------------------------------------------------- 8
def unescape(s):
    raw = re.sub(r"\\x([0-9a-f]{2})", lambda m: chr(int(m.group(1), 16)), s).encode("latin1")
    try:
        return raw.decode("utf-8")
    except UnicodeDecodeError:
        return s


def report_reversal():
    ex, summary = [], None
    for r in rows("reversal_v1"):
        if r[0] == "example":
            ex.append([unescape(r[1]), r[2], unescape(r[3])])
        elif r[0] == "summary":
            summary = r[1:]
    out = ["# 1 versijos silpnybė: pranešimo atkūrimas iš maišos", "",
           "1 versijoje būsena buvo 256 bitų ir visa išvedama kaip maiša, o kiekvienas maišymo žingsnis yra apverčiamas. "
           "Iki 15 baitų įvestis patenka į vieną likučio žingsnį, todėl iš maišos galima skaičiuoti atgal: atspėjamas ilgis "
           "(16 variantų), atšaukiami 3 tušti žingsniai ir ilgio žingsnis, o likučio žingsnyje nežinomi tik du 64 bitų "
           "žodžiai. Dvi būsenos dalys, į kurias žodžiai nepatenka, turi sutapti su žinomomis pradinėmis reikšmėmis – tai "
           "patikrina spėjimą, o likusios dvi tiesiog atiduoda abu žodžius. Taikinio maišos gautos tikra 1 versijos realizacija "
           "(commit `b3d54be`).", ""]
    out += table(["Pranešimas", "1 versijos maiša", "Atkurta iš maišos"],
                 [[f"`{m}`" if m else "(tuščias)", f"`{h[:24]}…`", f"`{g}`" if g else "(tuščias)"] for m, h, g in ex])
    n, ok, us = summary
    out += ["", f"10 000 atsitiktinių 0–15 baitų pranešimų (bet kokios baitų reikšmės): atkurta **{whole(ok)}/{whole(n)}**, "
            f"vidutiniškai {num(float(us), 2)} µs vienam. Tai ne perrinkimas, o tiesioginis skaičiavimas.", "",
            "2 versijoje tas pats kelias neprasideda: maiša yra 512 bitų būsenos sulenkimas į 256 bitus, todėl skaičiavimui "
            "atgal trūksta 256 būsenos bitų (2^256 variantų). Tai pašalina šį konkretų atkūrimo kelią, bet neįrodo atsparumo "
            "pirmavaizdžio paieškai."]
    write("exp8_atstatymas.md", out)


report_inputs()
report_speed()
report_collisions()
report_avalanche()
report_guess()
report_reversal()
print("ataskaitos:", ", ".join(sorted(f for f in os.listdir(OUT) if f.endswith((".md", ".svg")))))
