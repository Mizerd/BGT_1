# BGT 1 užduotis: dvi 256 bitų maišos funkcijos · v0.1

Porinis darbas: dvi atskiros realizacijos – viena be DI, kita su DI – palygintos tomis pačiomis sąlygomis.

> Abi funkcijos kriptografiškai neanalizuotos – netinka slaptažodžiams ir saugumui.

## 1. Kas ką padarė

| | Ratas-256 | DI maiša |
|---|---|---|
| Katalogas | [`Joringis-no AI/`](Joringis-no%20AI/README.md) | [`Rokas - AI/`](Rokas%20-%20AI/README.md) |
| DI | nenaudotas | naudotas nuo pradžių |
| Kalba | C++17, vienas failas `ratas.cpp` | C++20, CMake |
| Indėlis į bendrą dalį | duomenų rinkinys: `data/exp1`, `konstitucija.txt` | eksperimentų programa: `experiments.cpp`, `report.py` |

Paleidimas, pseudokodas ir sprendimų pagrindimas – kiekvieno kataloge README.

## 2. Du algoritmai

| | Ratas-256 | DI maiša |
|---|---|---|
| Būsena | 8 × 32 b = 256 b | 8 × 64 b = 512 b |
| Blokas | 16 B, 2 pasukimai | 32 B, perėjimas pirmyn ir atgal |
| Netiesiškumas | nuo duomenų priklausantis posūkis | daugyba iš nelyginių konstantų |
| Pabaiga | PKCS#7, ilgis, 4 pasukimai | likučio ilgis žymėje, ilgis, 3 tušti žingsniai |
| Išvestis | dvi būsenos pusės, tarp jų – 2 pasukimai | 512 → 256 sulenkimas |

## 3. Vienodos sąlygos

* **Ta pati eksperimentų programa** – skiriasi tik adapteris `experiments/impl_*.cpp`.
* **Tie patys duomenys:** `exp1` (34 failai), `konstitucija.txt`, seed 20260920, abėcėlė `!`..`~`.
* **Atkartojamumas:** 1–3, 5 ir 6 eksperimentų rezultatai paleidus iš naujo sutampa **baitas į baitą** (Ratas-256 – net Windows / MSVC ir Linux / g++).
* **Sparta** matuojama abiem viename kompiuteryje (`palyginimas/`).

## 4. Teisingumas (1–3)

| Patikra | Ratas-256 | DI maiša |
|---|---|---|
| 22 įvesčių poros: 1 B pakeitimai, tvarka, tarpai, LF / CRLF, `\0` | 22/22 | 22/22 |
| 64 hex, mažosios raidės, pradiniai nuliai | 35/35 | 35/35 |
| A, B, A ir kartotiniai kvietimai | ✓ | ✓ |
| atskiri paleidimai / ranka = failas | 34/34, 31/31 | 34/34, 31/31 |

## 5. Sparta (4)

_Laukiama spartos matavimų abiem realizacijoms viename kompiuteryje._

## 6. Kolizijos (5)

| | Tikėtina | Ratas-256 | DI maiša |
|---|---|---|---|
| 4 × 100 000 porų ir 4 × 200 000 įvesčių | ≈ 10^(−67) | 0 | 0 |
| 119 374 struktūruotos įvestys | ≈ 0 | 0 | 0 |
| maiša sutrumpinta iki 24 bitų | 4 768 | 4 743 | 4 654 |
| maiša sutrumpinta iki 32 bitų | 18,6 | 13 | 19 |

* Nulis 256 bitų maišai – įprastas ir **nieko neįrodo**.
* Sutrumpintos maišos atitinka gimtadienio paradoksą – abi elgiasi kaip atsitiktinės.

## 7. Lavinos efektas (6)

| 100 000 porų | Idealu | Ratas-256 | DI maiša |
|---|---|---|---|
| bitų skirtumas | 50 % | 50,01 % | 50,00 % |
| min–max | – | 36,3–63,3 % | 36,7–64,5 % |
| standartinis nuokrypis | 3,13 % | 3,12 % | 3,12 % |
| hex skirtumas | 93,75 % | 93,75 % | 93,75 % |
| apverstas 1 įvesties bitas | 50 % | 49,99 % | 50,01 % |

Histogramos: [Ratas-256](Joringis-no%20AI/results/exp6_lavina.md) · [DI maiša](Rokas%20-%20AI/results/exp6_lavina.md)

## 8. Spėjimas ir druska (7)

Abiejų rezultatai sutampa:

| Atvejis | Maišų | Rezultatas |
|---|---|---|
| `0000`–`9999` be druskos | 3 984, ≈ 1 ms | vienintelis sutapimas `3983` |
| viena lentelė 5 taikiniams | 10 000 | 5/5 |
| vieša druska, atskira kiekvienam | 5 × 10 000 | 5/5 |
| slaptas 16 B `r` | 10 000 · 2^128 | neperrenkama |

## 9. Išvados (8)

* **Statistiškai nesiskiria:** abi praeina 1–3, 0 kolizijų, lavinos efektas ≈ idealus.
* **Sparta:** _laukiama matavimų._
* **Testai neįrodo** saugumo, atsparumo kolizijoms ar pirmavaizdžiui; geras lavinos efektas galimas ir silpnai funkcijai.
* **Pirmavaizdis:** mažą aibę abi perrenka per ≈ 1 ms – sunkumą lemia paieškos erdvė, ne maiša.

## 10. Silpnybės

| Ratas-256 | DI maiša |
|---|---|
| būsena lygi išvesties dydžiui (256 b) | vienas maišymo žingsnis 32 B blokui |
| `rotl(a, d)` nieko nesuka, kai d mod 32 = 0 | tiesinis 512 → 256 sulenkimas |

Abiem: nerecenzuota, nėra rakto ir druskos, failas įkeliamas į atmintį.

## 11. DI naudojimas

DI naudota tik **DI maišai** ir bendrai eksperimentų programai: Claude Code (Anthropic), Claude Opus modeliai.
Užklausos, priimti ir atmesti pasiūlymai, patikra – [DI maišos README, 18 skyrius](Rokas%20-%20AI/README.md#18-di-naudojimas).
Ratas-256 sukurta be DI.

## 12. Versija

`v0.1` – be DI sukurta Ratas-256 ir DI maiša, palygintos šiame README.
