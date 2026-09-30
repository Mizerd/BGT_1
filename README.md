# BGT 1 užduotis: dvi 256 bitų maišos funkcijos · V0.2

Porinis darbas: dvi atskiros realizacijos – Ratas-256 ir DI maiša – palygintos tomis pačiomis sąlygomis.

> Abi funkcijos kriptografiškai neanalizuotos – netinka slaptažodžiams ir saugumui.

## 1. Kas ką padarė

| | Ratas-256 | DI maiša |
|---|---|---|
| Katalogas | [`Joringis-no AI/`](Joringis-no%20AI/README.md) | [`Rokas - AI/`](Rokas%20-%20AI/README.md) |
| Kalba | C++20, vienas failas `ratas.cpp` | C++20, CMake |
| Indėlis į bendrą dalį | duomenų rinkinys: `data/exp1`, `konstitucija.txt` | eksperimentų programa: `experiments.cpp`, `report.py`, palyginimai |

Paleidimas, pseudokodas ir sprendimų pagrindimas – kiekvieno kataloge README; palyginimo atkartojimas – [palyginimas/README.md](palyginimas/README.md).

## 2. Du algoritmai (dabartinės versijos)

| | Ratas-256 v0.2 | DI maiša V0.2 |
|---|---|---|
| Būsena | 8 × 32 b = 256 b | 8 × 64 b = 512 b |
| Blokas | 16 B, 3 pasukimai, feed-forward | 32 B, 2 raundai (pirmyn ir atgal), grįžtamasis ryšys |
| Netiesiškumas | nuo duomenų priklausantis posūkis | daugyba iš nelyginių konstantų |
| Pabaiga | PKCS#7, ilgis, 4 pasukimai, feed-forward | likučio ilgis žymėje, ilgis, 3 tušti žingsniai |
| Išvestis | dvi būsenos pusės, tarp jų – 2 pasukimai | 512 → 256 sulenkimas |

Grįžtamasis ryšys (feed-forward) abiem atsirado vėliau: Ratas-256 – v0.12, DI maišai – V0.13 (12 sk.).

## 3. Vienodos sąlygos

* **Ta pati eksperimentų programa** – skiriasi tik adapteris `experiments/impl_*.cpp`.
* **Tie patys duomenys:** `exp1` (34 failai, turinys tikrinamas `check_fixtures.py`), `konstitucija.txt` (14 sk.), seed 20260920, abėcėlė `!`..`~`.
* **Atkartojamumas:** 1–3, 5 ir 6 eksperimentų rezultatai paleidus iš naujo sutampa **baitas į baitą** (Ratas-256 – net Windows / MSVC ir Linux / g++).
* **Sparta:** abi realizacijos tame pačiame kompiuteryje (`palyginimas/`); duomenyse įrašyti abiejų realizacijų commit'ai.

## 4. Teisingumas (1–3)

| Patikra | Ratas-256 v0.2 | DI maiša V0.2 |
|---|---|---|
| 21 įvesčių pora: 1 B pakeitimai, tvarka, tarpai, LF / CRLF, `\0` | 21/21 | 21/21 |
| 64 hex, mažosios raidės, pradiniai nuliai | 34/34 | 34/34 |
| A, B, A ir kartotiniai kvietimai | ✓ | ✓ |
| atskiri paleidimai / ranka = failas | 34/34, 31/31 | 34/34, 31/31 |

## 5. Sparta (4)

![Sparta](palyginimas/sparta.svg)

Abi realizacijos tame pačiame kompiuteryje; 3 apšilimai + 10 matavimų, be I/O. Vidurkis, µs:

| Kompiuteris | Versijos: Ratas-256 / DI | 70 B | 75 595 B |
|---|---|---|---|
| i9-10900K, Linux, g++ 15.2 `-O3` (grafikas) | v0.2 / V0.2 | 0,248 / 0,235 | 166,80 / 71,68 |
| Ryzen 9 7900X, Windows 11, MSVC `/O2` | v0.1 / V0.1 | 0,155 / 0,112 | 74,08 / 32,27 |

* Abiejų laikas auga tiesiškai. Ilgiems failams DI maiša ≈ 2,3 karto greitesnė, trumpoms įvestims – panašios.
* Lyginti galima tik eilutės viduje (skirtingi kompiuteriai). Versijų sulėtėjimas – kiekvienos kompiuteryje: DI maiša
  35,19 → 72,01 µs (Linux), Ratas-256 74,7 → 134,0 µs (Windows, jo rezultatai) – daugiau maišymo bloke.
* Visa programa su failo skaitymu matuojama atskirai (DI maiša: 1 GiB – 1,13 s, 3,7 MB atminties). → [visos lentelės](palyginimas/sparta.md)

## 6. Kolizijos (5)

| | Tikėtina | Ratas-256 v0.2 | DI maiša V0.2 |
|---|---|---|---|
| 4 × 100 000 porų ir 4 × 200 000 įvesčių | ≈ 10^(−67) | 0 | 0 |
| 119 374 struktūruotos įvestys | ≈ 0 | 0 | 0 |
| maiša sutrumpinta iki 24 bitų | 4 768 | 4 771 | 4 718 |
| maiša sutrumpinta iki 32 bitų | 18,6 | 14 | 20 |
| struktūrinė ataka | – | netirta | V0.12: randama akimirksniu; nuo V0.13 šis atakos testas nebeveikia |

* Nulis 256 bitų maišai – įprastas ir **nieko neįrodo**.
* Sutrumpintų maišų kolizijų skaičius neprieštarauja gimtadienio paradokso prognozei.

## 7. Lavinos efektas (6)

| 100 000 porų | Idealu | Ratas-256 v0.2 | DI maiša V0.2 |
|---|---|---|---|
| bitų skirtumas | 50 % | 50,01 % | 50,00 % |
| min–max | – | 37,5–63,3 % | 37,5–63,7 % |
| standartinis nuokrypis | 3,13 % | 3,13 % | 3,11 % |
| hex skirtumas | 93,75 % | 93,75 % | 93,76 % |
| apverstas 1 įvesties bitas | 50 % | 49,99 % | 49,99 % |

Histogramos: [Ratas-256](Joringis-no%20AI/results/exp6_lavina.md) · [DI maiša](Rokas%20-%20AI/results/exp6_lavina.md)

## 8. Spėjimas ir druska (7)

Abiejų rezultatai sutampa:

| Atvejis | Maišų | Rezultatas |
|---|---|---|
| `0000`–`9999` be druskos | 3 984, ≈ 1–2 ms | vienintelis sutapimas `3983` |
| viena lentelė 5 taikiniams | 10 000 | 5/5 |
| vieša druska, atskira kiekvienam | 5 × 10 000 | 5/5 |
| slaptas 16 B `r` | 10 000 · 2^128, jei `r` nežinomas | čia `r` iš viešo seed – tik demonstracija |

## 9. Išvados (8)

* **Statistiškai nesiskiria:** abi praeina 1–3, 0 kolizijų, lavinos efektas ≈ idealus – kiekvienoje versijoje.
* **Struktūrinė analizė:** V0.12 DI maišai rastos akimirksniu veikiančios kolizijos ir pirmavaizdžiai, nors statistika buvo gera;
  nuo V0.13 šis atakos testas nebeveikia (kitos atakos netirtos).
  Abi realizacijos galiausiai pridėjo grįžtamąjį ryšį ir daugiau maišymo bloke – tai lėmė analizė, ne statistiniai testai.
* **Sparta:** pataisymai kainavo spartą; dabar DI maiša ilgiems failams ≈ 2,3 karto greitesnė už Ratas-256.
* **Testai neįrodo** saugumo, atsparumo kolizijoms ar pirmavaizdžiui; geras lavinos efektas galimas ir silpnai funkcijai.
* **Pirmavaizdis:** mažą aibę abi perrenka per ≈ 1–2 ms – sunkumą lemia paieškos erdvė, ne maiša.

## 10. Silpnybės

| Ratas-256 v0.2 | DI maiša V0.2 |
|---|---|
| būsena lygi išvesties dydžiui (256 b) | 2 raundų saugumo atsarga neįvertinta |
| SAC matuoja tik pavienius bitus – diferencialai per kelis pasukimus netirti (jo README) | tiesinis 512 → 256 sulenkimas (prieš jį – neatsukami žingsniai) |

Abi funkcijos nerecenzuotos, neturi rakto ir druskos. Abi failą skaito dalimis (DI maiša – nuo V0.12, Ratas-256 – nuo v0.2).

## 11. Papildoma užduotis: MD5, SHA-1, SHA-256

![Sparta su standartinėmis maišomis](palyginimas/standartai.svg)

Ta pati eksperimentų programa, tos pačios įvestys, tas pats kompiuteris (i9-10900K, Linux, g++ 15.2 `-O3`); Ratas-256 v0.2, DI maiša V0.2;
atskiras paleidimas, todėl laikai gali skirtis nuo 5 sk. keliais procentais.
Standartinės maišos – OpenSSL 3.6 realizacijos, patikrintos su Python `hashlib` (102/102). Procentai – pagal maišos ilgį.

| | Ratas-256 | DI maiša | MD5 | SHA-1 | SHA-256 |
|---|---|---|---|---|---|
| Ilgis, bitai | 256 | 256 | 128 | 160 | 256 |
| 70 B, µs | 0,247 | 0,234 | 0,159 | 0,155 | 0,285 |
| 75 595 B, µs | 162,96 | 71,27 | 74,19 | 55,29 | 121,12 |
| MB/s | 464 | 1 061 | 1 019 | 1 367 | 624 |
| Bitų skirtumas, % | 50,01 | 50,00 | 49,99 | 49,98 | 50,00 |
| Std. nuokrypis (idealus), % | 3,13 (3,12) | 3,11 (3,12) | 4,43 (4,42) | 3,95 (3,95) | 3,12 (3,12) |
| Hex skirtumas, % | 93,75 | 93,76 | 93,74 | 93,75 | 93,74 |

* **Sparta:** SHA-1 greičiausia, DI maiša ≈ MD5, SHA-256 ir Ratas-256 lėčiausios. Šis procesorius neturi SHA instrukcijų (SHA-NI);
  OpenSSL naudoja asemblerį (AVX2), mūsų maišos – paprastas C++.
* **Lavinos efektas:** visų ≈ 50 %, sklaida lygi idealiai savo ilgiui – šis testas maišų **neišskiria**.
* MD5 ir SHA-1 kolizijos randamos praktiškai, nors jų lavinos efektas toks pat geras – tai neįrodo atsparumo kolizijoms.
  → [visos lentelės](palyginimas/standartai.md)

## 12. Versijos

| Versija | DI maiša | Ratas-256 |
|---|---|---|
| `V0.1` | 512 b būsena, 1 raundas bloke | 256 b „ratas“, 2 pasukimai bloke |
| `V0.11` | nauji testai, pataisytas CRLF testinis failas; maišos reikšmės tos pačios | posūkis 0 → 1, ne ASCII failų vardai |
| `V0.12` | failas skaitomas dalimis, GCC optimizavimas; reikšmės tos pačios | feed-forward, 64 b bloko numeris |
| `V0.13` | 2 raundai ir grįžtamasis ryšys – V0.12 ataka nebeveikia | kaip v0.12 |
| `V0.2` | kodo tvarkymas; algoritmas kaip V0.13 | 3 pasukimai, feed-forward ir pabaigoje, failas dalimis |

* Leidimai `V0.1`, `V0.11`, `V0.12`, `V0.13`, `V0.2` – abi realizacijos toje pačioje versijoje. `V0.11`–`V0.13` sudaryti šakose
  `shared-V0.11`…`shared-V0.13`: abiejų autorių tos versijos commit'ai perkelti (cherry-pick) į vieną būseną, todėl jų
  hash'ai skiriasi nuo `main`, kurioje tie patys pakeitimai eina vienas po kito.
* Ankstesnių versijų rezultatai – kiekvieno leidimo `results/` kataloguose.
* V0.1 leidime testinis failas `struct_newline_crlf.txt` faktiškai turėjo LF (tikras CRLF tikrintas tik atmintyje);
  nuo V0.11 faile tikras CRLF, todėl 21 pora ir 34 failai (V0.1 – 22 ir 35).
* `V0.2` – galutinė versija, pažymėta žyme `V0.2` (commit `c73dc9b`); vėlesni `main` commit'ai keičia tik dokumentaciją.

## 13. DI naudojimas

DI maiša, bendra eksperimentų programa, bendras README ir palyginimai – Claude Code (Anthropic), Claude Opus modeliai;
galutinę būseną papildomai peržiūrėjo Claude Sonnet. Užklausos, priimti ir atmesti pasiūlymai, patikra –
[DI maišos README, 18 skyrius](Rokas%20-%20AI/README.md#18-di-naudojimas). Kiekvienos realizacijos kūrimo eiga aprašyta jos kataloge.

## 14. Šaltiniai

* `konstitucija.txt` – Lietuvos Respublikos Konstitucijos tekstas iš kurso medžiagos ([nuoroda užduotyje](https://bit.ly/33nYy2v)),
  75 595 B, SHA-256 `ccd6bc7bf8bf3da2…`.
* Standartinės maišos – OpenSSL 3.6.3, patikra – Python `hashlib`; [NIST Hash Functions](https://csrc.nist.gov/projects/hash-functions).
* VU BGT 1 užduotis ir kontrolinis sąrašas (2026); kiti šaltiniai – kiekvienos realizacijos README.
