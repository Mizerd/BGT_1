#!/usr/bin/env python3
"""Builds the experiment tables (Markdown) and charts (SVG) from results/raw/*.csv."""

import csv
import math
import os
import sys
from collections import defaultdict

RAW, OUT = sys.argv[1], sys.argv[2]

LIGHT = {"bg": "#fcfcfb", "text": "#0b0b0b", "text2": "#52514e", "grid": "#e4e3de", "s": "#2a78d6"}
DARK = {"bg": "#1a1a19", "text": "#ffffff", "text2": "#c3c2b7", "grid": "#353533", "s": "#3987e5"}


def rows(name):
    with open(os.path.join(RAW, name + ".csv"), newline="") as f:
        return [r for r in csv.reader(f) if r]


def num(x, d=2):
    return f"{x:,.{d}f}".replace(",", " ").replace(".", ",")


def whole(x):
    return f"{int(x):,}".replace(",", " ")


def write(name, lines):
    with open(os.path.join(OUT, name), "w") as f:
        f.write("\n".join(lines).rstrip() + "\n")


def table(header, body):
    out = ["| " + " | ".join(header) + " |", "|" + "|".join("---" for _ in header) + "|"]
    return out + ["| " + " | ".join(str(c) for c in r) + " |" for r in body]


def svg_open(w, h, label, title):
    def block(p):
        return (f".bg{{fill:{p['bg']}}} .t{{fill:{p['text']}}} .t2{{fill:{p['text2']}}} .grid{{stroke:{p['grid']}}} "
                f".axis,.ref{{stroke:{p['text2']}}} .fs{{fill:{p['s']}}} .ss{{stroke:{p['s']}}} .ring{{stroke:{p['bg']}}}")
    return [f'<svg xmlns="http://www.w3.org/2000/svg" width="{w}" height="{h}" viewBox="0 0 {w} {h}" role="img" aria-label="{label}">',
            f"<style>text{{font-family:system-ui,-apple-system,'Segoe UI',sans-serif}} {block(LIGHT)} "
            f"@media (prefers-color-scheme: dark){{{block(DARK)}}}</style>",
            f'<rect class="bg" fill="{LIGHT["bg"]}" width="{w}" height="{h}"/>',
            f'<text class="t" x="70" y="28" font-size="16" font-weight="600">{title}</text>']


def svg_save(name, s):
    with open(os.path.join(OUT, name), "w") as f:
        f.write("\n".join(s + ["</svg>"]) + "\n")


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
    hexes, info, fmt, rep, leadex = {}, {}, 0, 0, []
    other = {}
    for r in rows("inputs"):
        if r[0] == "input":
            _, _, name, size, chars, hx, f_ok, r_ok = r
            hexes[name] = hx
            info[name] = (int(size), int(chars))
            fmt += int(f_ok)
            rep += int(r_ok)
        elif r[0] == "aba":
            other["aba"] = r[2] == "1"
        elif r[0] == "repeat":
            other["repeat"] = (r[2], r[4] == "1")
        elif r[0] == "lead":
            other["lead"] = (int(r[3]), int(r[4]))
        elif r[0] == "leadex":
            leadex.append((r[2], r[3]))
    n = len(info)

    out = ["# 1–3 eksperimentai: įvestys, formatas, determinizmas", "",
           "Įvestys – bendras poros rinkinys `Joringis-no AI/data/exp1/` ir papildomas tikras CRLF atvejis, sudarytas atmintyje.", ""]
    names = sorted(info, key=lambda x: (x.startswith("crlf"), x))
    out += table(["Įvestis", "Baitai", "Simboliai (UTF-8)", "Aprašymas", "Maiša (pradžia)"],
                 [[f"`{x}`", info[x][0], info[x][1], DESC.get(x, ""), f"`{hexes[x][:16]}…`"] for x in names])
    out += ["", "## Palyginimai poromis", ""]
    body = []
    for a, b, differ in COMPARE:
        same = hexes[a] == hexes[b]
        body.append([f"`{a}` ↔ `{b}`", "skiriasi" if differ else "sutampa",
                     ("sutampa" if same else "skiriasi") + (" ✓" if same != differ else " ✗")])
    out += table(["Pora", "Tikimasi", "Rezultatas"], body)
    out += ["", "`struct_newline_crlf.txt` bendrame rinkinyje saugomas su LF, todėl jo baitai sutampa su `_lf`. "
            "Tikras CRLF atvejis patikrintas eilute `crlf_atmintyje`.", "", "## Formatas ir determinizmas", ""]
    out += table(["Patikra", "Rezultatas"], [
        ["64 hex simboliai, mažosios raidės, dekoduojasi į tą pačią maišą", f"{fmt}/{n}"],
        ["3 kartotiniai kvietimai duoda tą pačią maišą", f"{rep}/{n}"],
        ["Seka A, B, A (A sutampa, B skiriasi)", "taip" if other["aba"] else "ne"],
        [f"1 000 kvietimų su `{other['repeat'][0]}`", "taip" if other["repeat"][1] else "ne"],
        ["Maišos, prasidedančios `0`, iš `0000`–`9999` (tikėtina ≈ 625)", other["lead"][0]],
        ["Maišos, prasidedančios `00` (tikėtina ≈ 39)", other["lead"][1]],
    ])
    out += ["", "Pradiniai nuliai išsaugomi:", ""] + [f"* `{c}` → `{h}` ({len(h)} simboliai)" for c, h in leadex]

    tally = defaultdict(lambda: [0, 0])
    for kind, name, *rest in rows("cli"):
        tally[kind][0] += rest[-1] == "1"
        tally[kind][1] += 1
        if kind == "runs":
            tally["same"][0] += rest[0] == hexes.get(name)
            tally["same"][1] += 1
    out += ["", "## Komandinė eilutė", ""]
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
    groups, size = defaultdict(list), {}
    for _, _, lines, nbytes, _, _, _, per in rows("speed"):
        groups[int(lines)].append(float(per) / 1000)
        size[int(lines)] = int(nbytes)
    order = sorted(size)
    mean = {k: sum(v) / len(v) for k, v in groups.items()}
    out = ["# 4 eksperimentas: sparta", "",
           "`konstitucija.txt` ištraukos po 1, 2, 4, … eilučių su eilučių skirtukais ir visas failas. Ištrauka paruošiama iš anksto, "
           "matuojamas tik maišos skaičiavimas (be failų I/O ir išvedimo). `std::chrono::steady_clock`, 3 apšilimo ir 10 matavimų "
           "kiekvienam dydžiui; viename matavime maiša kartojama, kol praeina ≥ 20 ms, ir laikas dalijamas iš kvietimų skaičiaus. "
           "Rezultatas naudojamas (`volatile`), o maišos funkcija yra atskirame vertimo vienete, todėl kompiliatorius skaičiavimo neišmeta.", ""]
    out += table(["Eilutės", "Baitai", "Vidurkis, µs", "Min–max, µs", "MB/s"],
                 [[k, whole(size[k]), num(mean[k], 3), f"{num(min(groups[k]), 3)}–{num(max(groups[k]), 3)}",
                   whole(size[k] / mean[k])] for k in order])
    out += ["", "Neapdoroti matavimai: `raw/speed.csv`."]
    write("exp4_sparta.md", out)

    W, H, L, R, T, B = 760, 420, 78, 40, 52, 62
    x0, x1 = 1, 5
    y0, y1 = -2, 2
    px = lambda b: L + (math.log10(b) - x0) / (x1 - x0) * (W - L - R)
    py = lambda us: H - B - (math.log10(us) - y0) / (y1 - y0) * (H - T - B)
    s = svg_open(W, H, "Vienos maišos skaičiavimo laikas pagal įvesties dydį", "Vienos maišos skaičiavimo laikas pagal įvesties dydį")
    for e in range(x0, x1 + 1):
        s.append(f'<line class="grid" stroke="{LIGHT["grid"]}" x1="{px(10**e):.1f}" y1="{T}" x2="{px(10**e):.1f}" y2="{H - B}"/>'
                 f'<text class="t2" x="{px(10**e):.1f}" y="{H - B + 18}" font-size="12" text-anchor="middle">{whole(10**e)}</text>')
    for e in range(y0, y1 + 1):
        s.append(f'<line class="grid" stroke="{LIGHT["grid"]}" x1="{L}" y1="{py(10**e):.1f}" x2="{W - R}" y2="{py(10**e):.1f}"/>'
                 f'<text class="t2" x="{L - 8}" y="{py(10**e) + 4:.1f}" font-size="12" text-anchor="end">{num(10**e, max(0, -e))}</text>')
    s.append(f'<text class="t2" x="{(L + W - R) / 2}" y="{H - 16}" font-size="13" text-anchor="middle">Įvesties dydis, baitai (log. skalė)</text>')
    s.append(f'<text class="t2" transform="translate(20 {(T + H - B) / 2}) rotate(-90)" font-size="13" text-anchor="middle">Laikas vienai maišai, µs (log. skalė)</text>')
    pts = [(px(size[k]), py(mean[k]), k) for k in order]
    s.append(f'<path class="ss" stroke="{LIGHT["s"]}" fill="none" stroke-width="2" stroke-linejoin="round" d="'
             + " ".join(f"{'M' if i == 0 else 'L'}{x:.1f},{y:.1f}" for i, (x, y, _) in enumerate(pts)) + '"/>')
    for x, y, k in pts:
        s.append(f'<circle class="fs ring" fill="{LIGHT["s"]}" stroke="{LIGHT["bg"]}" stroke-width="2" cx="{x:.1f}" cy="{y:.1f}" r="4.5">'
                 f'<title>{whole(size[k])} B – {num(mean[k], 3)} µs</title></circle>')
    svg_save("exp4_sparta.svg", s)


# ---------------------------------------------------------------- 5
def report_collisions():
    pairs, trunc, examples = {}, defaultdict(lambda: [0, 0.0]), []
    for r in rows("collisions"):
        if r[0] == "pairs":
            pairs[int(r[2])] = [int(v) for v in r[3:]]
        elif r[0] == "trunc":
            trunc[int(r[3])][0] += int(r[5])
            trunc[int(r[3])][1] += float(r[6])
        elif r[0] == "example":
            examples.append(r)
    out = ["# 5 eksperimentas: kolizijos", "",
           "Kiekvienam ilgiui 100 000 atsitiktinių porų iš abėcėlės `!`..`~` (94 ASCII simboliai, 1 simbolis = 1 baitas), "
           "`std::mt19937_64`, seed = 20260920 + ilgis, simbolis = `'!' + (x mod 94)`. Jei poros narės sutampa, antroji "
           "generuojama iš naujo. Kolizija skaičiuojama tik tarp skirtingų įvesčių.", ""]
    out += table(["Ilgis", "Porų", "Iš naujo generuota", "Kolizijos porose", "Skirtingų įvesčių", "Kolizijų grupės rinkinyje"],
                 [[L, whole(p[0]), p[1], p[2], whole(p[4]), p[5]] for L, p in sorted(pairs.items())])
    if examples:
        out += ["", "Rastos kolizijos:", ""] + [f"* ilgis {e[2]}: `{e[4]}` ir `{e[5]}` → `{e[3]}`" for e in examples]
    out += ["", "## Kodėl kolizijų nerandama ir ar testas jas pastebėtų", "",
            "Idealiai n bitų maišai vienos poros kolizijos tikimybė ≈ 2^(−n), o m įvesčių rinkinyje yra m(m−1)/2 porų. Kai "
            "m = 200 000 ir n = 256, tikėtina ≈ 2·10^10 · 2^(−256) ≈ 10^(−67) kolizijų, todėl nulis yra įprastas ir apie saugumą "
            "nieko neįrodo. Kad matytųsi, jog testas kolizijas randa, tie patys rinkiniai patikrinti su sutrumpintomis maišomis "
            "(visi keturi ilgiai kartu):", ""]
    out += table(["Sutrumpinta iki", "Tikėtina", "Rasta"],
                 [[f"{b} bitai", num(trunc[b][1], 2), whole(trunc[b][0])] for b in sorted(trunc)])
    out += ["", "## Struktūruotos įvestys", ""]
    out += table(["Rinkinys", "Įvesčių", "Skirtingų", "Kolizijų grupės"],
                 [[r[2], whole(r[3]), whole(r[4]), r[5]] for r in rows("structured")])
    write("exp5_kolizijos.md", out)


# ---------------------------------------------------------------- 6
def report_avalanche():
    stats, hist = defaultdict(dict), defaultdict(dict)
    for r in rows("avalanche"):
        if r[0] == "aval":
            stats[r[2]][r[3]] = [int(r[4])] + [float(v) for v in r[5:]]
        elif r[0] == "hist":
            hist[r[2]][int(r[3])] = int(r[4])
    ch = stats["simbolis"]
    order = [k for k in ch if k != "visi"] + ["visi"]
    out = ["# 6 eksperimentas: lavinos efektas", "",
           "100 000 porų: po 25 000 ilgiams 10, 100, 500 ir 1 000. Kiekvienoje poroje vienas atsitiktinai parinktas simbolis "
           "pakeistas kitu tos pačios abėcėlės `!`..`~` simboliu, ilgis nekinta (`std::mt19937_64`, seed = 20260920 + 100 + ilgis). "
           "Prieš lyginant bitus abi hex maišos dekoduojamos į baitus. Orientyrai: ≈ 50 % bitų ir ≈ 93,75 % hex skaitmenų.", ""]
    out += table(["Ilgis", "Porų", "Bitai: vid. %", "min %", "max %", "st. nuokr. %", "Hex: vid. %", "min %", "max %"],
                 [[k, whole(v[0]), num(v[3]), num(v[1]), num(v[2]), num(v[4]), num(v[7]), num(v[5]), num(v[6])]
                  for k in order for v in [ch[k]]])
    b = stats["bitas"]["visi"]
    out += ["", f"Idealiam atsitiktiniam atvejiui bitų skirtumo standartinis nuokrypis √(256·0,25)/256 = 3,13 %.", "",
            "## Papildomai: apverstas tiksliai vienas įvesties bitas", "",
            f"Tie patys ilgiai ir porų skaičius, bet apverčiamas vienas bitas (baitų režimu; seed = 20260920 + 200 + ilgis): "
            f"bitų skirtumas {num(b[3])} % ({num(b[1])}–{num(b[2])} %), hex skirtumas {num(b[7])} % ({num(b[5])}–{num(b[6])} %).",
            "", "![Bitų skirtumo histograma](exp6_histograma.svg)", "",
            "Pilka linija – binominis pasiskirstymas B(256; 0,5), kurio tikėtųsi iš idealiai atsitiktinės maišos. "
            "Histogramos duomenys: `raw/avalanche.csv` (eilutės `hist`)."]
    write("exp6_lavina.md", out)

    W, H, L, R, T, B, bw = 760, 400, 78, 40, 70, 62, 2
    h = hist["simbolis"]
    k0, k1 = (min(h) // bw - 1) * bw, (max(h) // bw + 2) * bw
    bins = list(range(k0, k1, bw))
    counts = [sum(h.get(k + j, 0) for j in range(bw)) for k in bins]
    total = sum(counts)
    expected = [total * sum(math.comb(256, k + j) for j in range(bw)) / 2 ** 256 for k in bins]
    ymax = max(max(counts), max(expected)) * 1.08
    px = lambda k: L + (k - k0) / (k1 - k0) * (W - L - R)
    py = lambda c: H - B - c / ymax * (H - T - B)
    s = svg_open(W, H, "Bitų skirtumo histograma", "Bitų skirtumo pasiskirstymas (100 000 porų, pakeistas vienas simbolis)")
    s.append(f'<line class="ref" stroke="{LIGHT["text2"]}" x1="70" y1="46" x2="92" y2="46" stroke-width="2"/>'
             f'<text class="t2" x="98" y="50" font-size="12">tikėtina idealiai maišai B(256; 0,5)</text>')
    for y in range(0, int(ymax) + 1, 2000):
        s.append(f'<line class="grid" stroke="{LIGHT["grid"]}" x1="{L}" y1="{py(y):.1f}" x2="{W - R}" y2="{py(y):.1f}"/>'
                 f'<text class="t2" x="{L - 8}" y="{py(y) + 4:.1f}" font-size="12" text-anchor="end">{whole(y)}</text>')
    for pct in range(math.ceil(k0 / 2.56 / 5) * 5, int(k1 / 2.56) + 1, 5):
        s.append(f'<text class="t2" x="{px(pct * 2.56):.1f}" y="{H - B + 18}" font-size="12" text-anchor="middle">{pct} %</text>')
    step = (W - L - R) / len(bins)
    for k, c in zip(bins, counts):
        if c:
            hh = c / ymax * (H - T - B)
            s.append(f'<rect class="fs" fill="{LIGHT["s"]}" x="{px(k) + 1:.1f}" y="{H - B - hh:.1f}" width="{step - 2:.1f}" '
                     f'height="{hh:.1f}" rx="1"><title>{k}–{k + bw - 1} bitų skiriasi – {whole(c)} porų</title></rect>')
    s.append(f'<path class="ref" stroke="{LIGHT["text2"]}" fill="none" stroke-width="2" d="'
             + " ".join(f"{'M' if i == 0 else 'L'}{px(k + bw / 2):.1f},{py(e):.1f}" for i, (k, e) in enumerate(zip(bins, expected))) + '"/>')
    s.append(f'<line class="axis" stroke="{LIGHT["text2"]}" x1="{L}" y1="{H - B}" x2="{W - R}" y2="{H - B}"/>')
    s.append(f'<text class="t2" x="{(L + W - R) / 2}" y="{H - 16}" font-size="13" text-anchor="middle">Pasikeitusių maišos bitų dalis</text>')
    s.append(f'<text class="t2" transform="translate(20 {(T + H - B) / 2}) rotate(-90)" font-size="13" text-anchor="middle">Porų skaičius</text>')
    svg_save("exp6_histograma.svg", s)


# ---------------------------------------------------------------- 7
def report_guess():
    d = defaultdict(list)
    for r in rows("guess"):
        d[r[0]].append(r[2:])
    target, first, total, matches, us = d["nosalt"][0]
    targets, calls, t_us, cracked = d["table"][0]
    out = ["# 7 eksperimentas: spėjimas, vieša druska ir slaptas atsitiktinumas", "",
           "Kandidatai – visos keturių skaitmenų eilutės `0000`–`9999` (10 000, po 4 baitus, be naujos eilutės). Tikslinė įvestis "
           "parinkta `std::mt19937_64` (seed 20260927); atakai pateikiama tik maiša ir kandidatų rinkinys.", "",
           "## Be druskos: H(input)", ""]
    out += table(["", "Rezultatas"], [
        ["Tikslinė įvestis", f"`{target}`"], ["Bandymų iki radimo", whole(first)],
        ["Patikrinta iš viso (visi sutapimai)", whole(total)], ["Sutampantys kandidatai", f"`{matches}`"],
        ["Viso perrinkimo laikas", f"{num(float(us) / 1000)} ms"],
        [f"Viena iš anksto apskaičiuota lentelė {targets} taikiniams", f"{whole(calls)} maišų, {num(float(t_us) / 1000)} ms, atspėta {cracked}/{targets}"],
    ])
    out += ["", "Kandidatų rinkinyje rastas tiksliai vienas sutapimas, todėl čia jis identifikuoja įvestį. Bendru atveju sutapimas to "
            "neįrodo: kolizijos neišvengiamai egzistuoja, o tikroji įvestis gali būti ir už rinkinio ribų.", "",
            "## Vieša druska: H(input || salt)", "",
            "Kiekvienam taikiniui – atskira atsitiktinė 16 baitų druska, pridedama po įvesties kaip tikslūs baitai (ne hex tekstas); "
            "lentelėje parodyta hex. Užpuolikas druską žino.", ""]
    out += table(["#", "Taikinys", "Druska", "Bandymų", "Sutapimų", "Laikas, ms"],
                 [[r[0], f"`{r[1]}`", f"`{r[2]}`", whole(r[3]), r[4], num(float(r[5]) / 1000)] for r in d["salt"]])
    n = len(d["salt"])
    out += ["", f"Su druska kiekvienam taikiniui reikia atskiro perrinkimo: {n} taikiniams – {whole(10000 * n)} maišų vietoj 10 000. "
            "Vienam taikiniui druska darbo nepadidina, bet iš anksto apskaičiuotos lentelės nebegalima panaudoti kitiems taikiniams.", "",
            "## Slaptas atsitiktinumas: H(input || r)", ""]
    c = d["commit"][0]
    out += [f"Įsipareigojimas `c = H(\"{c[0]}\" || r)`, kur r – 16 slaptų baitų. Kol r nežinomas, paieškos erdvė – 10 000 · 2^128 "
            "variantų, todėl perrinkimas neatliekamas. Atskleidus r, patikrai užtenka vienos maišos: teisinga įvestis patvirtinta – "
            f"{'taip' if c[2] == '1' else 'ne'}, kita įvestis atmesta – {'taip' if c[3] == '1' else 'ne'}.", "",
            "Tai iliustruoja įsipareigojimo (commitment) idėją, bet neįrodo, kad konstrukcija saugiai paslepia pranešimą ar neleidžia "
            "jo pakeisti. Tai ir ne darbo įrodymo (proof-of-work) galvosūkis, kuriame ieškoma sąlygą tenkinančios nonce."]
    write("exp7_spejimas.md", out)


report_inputs()
report_speed()
report_collisions()
report_avalanche()
report_guess()
print("ataskaitos:", ", ".join(sorted(f for f in os.listdir(OUT) if f.endswith((".md", ".svg")))))
