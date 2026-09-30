# BGT 1 užduotis: dvi 256 bitų maišos funkcijos · V0.13

Porinis darbas: dvi atskiros realizacijos – Ratas-256 ir DI maiša – palygintos tomis pačiomis sąlygomis.

> Abi funkcijos kriptografiškai neanalizuotos – netinka slaptažodžiams ir saugumui.

## 1. Kas ką padarė

| | Ratas-256 | DI maiša |
|---|---|---|
| Katalogas | [`Joringis-no AI/`](Joringis-no%20AI/README.md) | [`Rokas - AI/`](Rokas%20-%20AI/README.md) |
| Kalba | C++17, vienas failas `ratas.cpp` | C++20, CMake |
| Indėlis į bendrą dalį | duomenų rinkinys: `data/exp1`, `konstitucija.txt` | eksperimentų programa: `experiments.cpp`, `report.py` |

Paleidimas, pseudokodas ir sprendimų pagrindimas – kiekvieno kataloge README.

## 2. Du algoritmai

| | Ratas-256 v0.12 | DI maiša V0.13 |
|---|---|---|
| Būsena | 8 × 32 b = 256 b | 8 × 64 b = 512 b |
| Blokas | 16 B, 2 pasukimai, feed-forward, 64 b bloko numeris | 32 B, 2 raundai (pirmyn ir atgal), grįžtamasis ryšys |
| Netiesiškumas | nuo duomenų priklausantis posūkis (1..31 bitų) | daugyba iš nelyginių konstantų |
| Pabaiga | PKCS#7, ilgis, 4 pasukimai | likučio ilgis žymėje, ilgis, 3 tušti žingsniai |
| Išvestis | dvi būsenos pusės, tarp jų – 2 pasukimai | 512 → 256 sulenkimas |

## 3. Vienodos sąlygos

* **Ta pati eksperimentų programa** – skiriasi tik adapteris `experiments/impl_*.cpp`.
* **Tie patys duomenys:** `exp1` (34 failai, turinys tikrinamas `check_fixtures.py`), `konstitucija.txt`, seed 20260920, abėcėlė `!`..`~`.
* **Atkartojamumas:** 1–3, 5 ir 6 eksperimentų rezultatai paleidus iš naujo sutampa **baitas į baitą** (Ratas-256 – net Windows / MSVC ir Linux / g++).
* **Sparta:** abi realizacijos matuojamos tame pačiame kompiuteryje – atskirai Windows ir Linux (`palyginimas/`).

## 4. Teisingumas (1–3)

| Patikra | Ratas-256 v0.12 | DI maiša V0.13 |
|---|---|---|
| 21 įvesčių pora: 1 B pakeitimai, tvarka, tarpai, LF / CRLF, `\0` | 21/21 | 21/21 |
| 64 hex, mažosios raidės, pradiniai nuliai | 34/34 | 34/34 |
| A, B, A ir kartotiniai kvietimai | ✓ | ✓ |
| atskiri paleidimai / ranka = failas | 34/34, 31/31 | 34/34, 31/31 |

## 5. Sparta (4)

![Sparta](palyginimas/sparta.svg)

Abi realizacijos tame pačiame kompiuteryje; 3 apšilimai + 10 matavimų, be I/O. Vidurkis, µs:

| Kompiuteris | Versijos: Ratas-256 / DI | 70 B: Ratas-256 / DI | 75 595 B: Ratas-256 / DI |
|---|---|---|---|
| Ryzen 9 7900X, Windows 11, MSVC `/O2` (grafikas) | v0.1 / V0.1 | 0,155 / 0,112 | 74,08 / 32,27 |
| i9-10900K, Linux, g++ 15.2 `-O3` | v0.12 / V0.13 | 0,177 / 0,234 | 77,27 / 71,20 |

Abiejų laikas auga tiesiškai. V0.1 DI maiša buvo ≈ 2,3 karto greitesnė; V0.13 turi 2 raundus (13 sk.) –
ilgiems failams DI maiša ≈ 1,1 karto greitesnė, trumpoms ≈ 1,3 karto lėtesnė. → [visos lentelės](palyginimas/sparta.md)

## 6. Kolizijos (5)

| | Tikėtina | Ratas-256 v0.12 | DI maiša V0.13 |
|---|---|---|---|
| 4 × 100 000 porų ir 4 × 200 000 įvesčių | ≈ 10^(−67) | 0 | 0 |
| 119 374 struktūruotos įvestys | ≈ 0 | 0 | 0 |
| maiša sutrumpinta iki 24 bitų | 4 768 | 4 776 | 4 718 |
| maiša sutrumpinta iki 32 bitų | 18,6 | 25 | 20 |
| struktūrinė ataka (13 sk.) | – | netirta | V0.12: akimirksniu; V0.13: nebeveikia |

* Nulis 256 bitų maišai – įprastas ir **nieko neįrodo**.
* Sutrumpintos maišos atitinka gimtadienio paradoksą – abi elgiasi kaip atsitiktinės.

## 7. Lavinos efektas (6)

| 100 000 porų | Idealu | Ratas-256 v0.12 | DI maiša V0.13 |
|---|---|---|---|
| bitų skirtumas | 50 % | 49,99 % | 50,00 % |
| min–max | – | 36,3–64,1 % | 37,5–63,7 % |
| standartinis nuokrypis | 3,13 % | 3,13 % | 3,11 % |
| hex skirtumas | 93,75 % | 93,74 % | 93,76 % |
| apverstas 1 įvesties bitas | 50 % | 50,00 % | 49,99 % |

Histogramos: [Ratas-256](Joringis-no%20AI/results/exp6_lavina.md) · [DI maiša](Rokas%20-%20AI/results/exp6_lavina.md)

## 8. Spėjimas ir druska (7)

Abiejų rezultatai sutampa:

| Atvejis | Maišų | Rezultatas |
|---|---|---|
| `0000`–`9999` be druskos | 3 984, ≈ 1–2 ms | vienintelis sutapimas `3983` |
| viena lentelė 5 taikiniams | 10 000 | 5/5 |
| vieša druska, atskira kiekvienam | 5 × 10 000 | 5/5 |
| slaptas 16 B `r` | 10 000 · 2^128 | neperrenkama |

## 9. Išvados (8)

* **Statistiškai nesiskiria:** abi praeina 1–3, 0 kolizijų, lavinos efektas ≈ idealus.
* **Sparta:** V0.1 DI maiša buvo ≈ 2,3 karto greitesnė; V0.13 (2 raundai) ilgiems failams tik ≈ 1,1 karto greitesnė už Ratas-256 v0.12.
* **Struktūrinė analizė:** V0.12 DI maišai rastos akimirksniu veikiančios kolizijos ir pirmavaizdžiai, nors visi
  statistiniai testai buvo geri; V0.13 šią ataką pašalino. Statistika tokios silpnybės nemato.
* **Testai neįrodo** saugumo, atsparumo kolizijoms ar pirmavaizdžiui; geras lavinos efektas galimas ir silpnai funkcijai.
* **Pirmavaizdis:** mažą aibę abi perrenka per ≈ 1 ms – sunkumą lemia paieškos erdvė, ne maiša.
* **Su standartais (11 sk.):** lavinos efektu MD5, SHA-1, SHA-256 ir abi mūsų maišos nesiskiria.

## 10. Silpnybės

| Ratas-256 v0.12 | DI maiša V0.13 |
|---|---|
| būsena lygi išvesties dydžiui (256 b) | V0.12: vienas raundas – kolizijos ir pirmavaizdžiai akimirksniu (pataisyta V0.13) |
| pabaiga be feed-forward – ją galima atsukti (jo README, v0.2 skyrius) | 2 raundų saugumo atsarga neįvertinta |

Abiem: nerecenzuota, nėra rakto ir druskos. Ratas-256 failą įkelia į atmintį; DI maiša (nuo V0.12) skaito dalimis.

## 11. Papildoma užduotis: MD5, SHA-1, SHA-256

![Sparta su standartinėmis maišomis](palyginimas/standartai.svg)

Ta pati eksperimentų programa, tos pačios įvestys, tas pats kompiuteris (i9-10900K, Linux, g++ 15.2 `-O3`); Ratas-256 – v0.12, DI maiša – V0.13.
Standartinės maišos – OpenSSL 3.6 realizacijos, patikrintos su Python `hashlib` (102/102). Procentai – pagal maišos ilgį.

| | Ratas-256 | DI maiša | MD5 | SHA-1 | SHA-256 |
|---|---|---|---|---|---|
| Ilgis, bitai | 256 | 256 | 128 | 160 | 256 |
| 70 B, µs | 0,183 | 0,232 | 0,158 | 0,155 | 0,282 |
| 75 595 B, µs | 77,57 | 70,79 | 73,61 | 53,78 | 119,71 |
| MB/s | 975 | 1 068 | 1 027 | 1 406 | 631 |
| Bitų skirtumas, % | 49,99 | 50,00 | 49,99 | 49,98 | 50,00 |
| Std. nuokrypis (idealus), % | 3,13 (3,12) | 3,11 (3,12) | 4,43 (4,42) | 3,95 (3,95) | 3,12 (3,12) |
| Hex skirtumas, % | 93,74 | 93,76 | 93,74 | 93,75 | 93,74 |

* **Sparta:** SHA-1 greičiausia; DI maiša, Ratas-256 ir MD5 – panašios; SHA-256 lėčiausia – šis procesorius neturi SHA instrukcijų (SHA-NI).
  OpenSSL naudoja asemblerį (AVX2), mūsų maišos – paprastas C++.
* **Lavinos efektas:** visų ≈ 50 %, sklaida lygi idealiai savo ilgiui – šis testas maišų **neišskiria**.
* MD5 ir SHA-1 kolizijos randamos praktiškai, nors jų lavinos efektas toks pat geras – tai neįrodo atsparumo kolizijoms.
  → [visos lentelės](palyginimas/standartai.md)

## 12. DI naudojimas

DI naudota **DI maišai**, bendrai eksperimentų programai, bendram README, spartos ir standartinių maišų palyginimui: Claude Code (Anthropic), Claude Opus modeliai.
Užklausos, priimti ir atmesti pasiūlymai, patikra – [DI maišos README, 18 skyrius](Rokas%20-%20AI/README.md#18-di-naudojimas).

## 13. Versija

* `V0.1` – Ratas-256 ir DI maiša, palygintos šiame README.
* `V0.11` – pataisytas bendras CRLF testinis failas (anksčiau jame buvo LF), abiejų 1–3 eksperimentų rezultatai sugeneruoti iš naujo;
  DI maišos testai ir eksperimentų patikros sugriežtinti, maišos reikšmės nepakito. Ratas-256 v0.11: posūkis 0 → 1, ne ASCII failų vardai.
* `V0.12` – DI maiša: failai skaitomi dalimis, iki 3 % greitesnė, nauji testai; maišos reikšmės nepakito.
  Spartos palyginimas pakartotas Linux (Windows matavimai palikti). Ratas-256 v0.12: feed-forward, 64 b bloko numeris.
* `V0.13` – DI maiša: 2 raundai ir grįžtamasis ryšys, nes V0.12 rastos akimirksniu veikiančios kolizijos ir pirmavaizdžiai;
  maišos reikšmės pasikeitė, visi palyginimai pakartoti (su Ratas-256 v0.12). Ratas-256 lieka v0.12.
* Leidimai `V0.11`–`V0.13` – abi realizacijos toje pačioje versijoje (šakos `shared-V0.11`…`shared-V0.13`);
  DI maišos versijos atskirai – šakos `AI-V0.11`, `AI-V0.12`, `AI-V0.13`.
