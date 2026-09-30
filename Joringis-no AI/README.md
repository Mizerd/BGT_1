# Ratas-256 v0.2

Mokomoji 256 bitų maišos funkcija (BGT 1 užduotis, porinio darbo pusė be DI pagalbos).

> **DI naudojimas.** v0.1-0.11 sukurta be DI pagalbos. v0.2 sukurta su DI
> (Claude Code, modelis Claude Opus 5.5) – pasiūlymai, sprendimai ir jų patikra
> aprašyti skyriuje „v0.2“. Geriausia be DI versija – v0.12 (commit `9b3a3ca`).

Maišos funkcija ir programa yra viename faile `ratas.cpp`. Kode komentarų nėra –
visi paaiškinimai yra šiame README. Algoritmo schemos: `docs/algoritmo-schema-v0.12.*`
ir `docs/algoritmo-schema-v0.2.*` (`.drawio` atidaromas diagrams.net / draw.io).

Funkcija nėra kriptografiškai analizuota ir netinka slaptažodžiams, pinigams
ar realioms sistemoms, tik mokymuisi.

## Kompiliavimas

**Visual Studio 2022.** Atidaryk `ratas.sln`, pasirink konfigūraciją `Release`
ir platformą `x64`, spausk Ctrl+F5. Rezultatas: `x64\Release\ratas.exe`.

Arba iš komandinės eilutės:

```
:: „x64 Native Tools Command Prompt for VS 2022“
cl /nologo /std:c++17 /O2 /EHsc /W4 /utf-8 ratas.cpp /Fe:ratas.exe
```

g++ (Linux / macOS / MSYS2):

```
g++ -std=c++17 -O2 -Wall -Wextra -o ratas ratas.cpp
```

## Paleidimas

Programą galima paleisti dvigubu pelės spustelėjimu, iš Visual Studio (F5 arba
Ctrl+F5) arba iš terminalo. 

Failo vardą Visual Studio perduosi per
*Project → Properties → Debugging → Command Arguments*.

Du režimai. Naudojamas režimas visada atspausdinamas.

| Režimas | Ką maišo |
|---|---|
| rankinis įvedimas | ranka įvestą tekstą (viena eilutė iki Enter) |
| failas | failo **turinį**, o ne pavadinimą |

Failą galima nurodyti komandinės eilutės argumentu:

```
ratas pavyzdys.txt
```

Tą patį padaro failo užtempimas ant `ratas.exe`: Explorer perduoda jo kelią
kaip argumentą.

Paleidus be argumento (pvz., dvigubu spustelėjimu) programa pirmiausia paklausia
režimo:

```
Pasirinkite rezima:
  1 - ivesti teksta ranka
  2 - maisyti faila
Pasirinkimas: 2
Failo kelias: pavyzdys.txt
```

Pasirinkus `2` įvedamas failo kelias; jei failas yra šalia `ratas.exe`, užtenka
vardo. Aplink kelią esančios kabutės (Explorer „Copy as path“) nuimamos.

Bandymams kataloge yra `pavyzdys.txt`:

```
$ ratas pavyzdys.txt
Rezimas: failas: pavyzdys.txt
Ivesties baitu: 38
Ratas-256: 47bce48a13ee6869699f2727d91df5244aca7dc0c03b4e7424d2482b42d84823
```

Jame yra `Labas, Lietuva! Ąžuolas prie ežero.`: 35 simbolių, bet 38 baitai, nes
`Ą` ir abu `ž` UTF-8 koduotėje užima po du baitus. Failas baigiasi be naujos
eilutės simbolio, todėl tą pačią eilutę įvedus ranka gaunama ta pati maiša.

Išėjimo kodai: `0` pavyko, `1` blogi argumentai arba neteisingas režimo
pasirinkimas, `2` failo nepavyko perskaityti. Neperskaitytas failas yra
klaida, o ne tuščia įvestis.

## Įvestis ir išvestis

- **Kodavimas.** Ranka įvestas tekstas koduojamas UTF-8 (Windows konsolė
  perskaitoma UTF-16 ir konvertuojama). Failas skaitomas dvejetainiu režimu,
  maišomi tikslūs jo baitai, eilučių pabaigos nekeičiamos. Failo kelias gali
  turėti ne ASCII raidžių (pvz., `ąžuolas.txt`) – nuo v0.11.
- **Enter.** Ranka įvedant naujos eilutės simbolis **neįtraukiamas**. Todėl
  `ratas` ir `ratas failas.txt` duoda tą pačią maišą tik tada, kai faile nėra
  eilutės pabaigos simbolio.
- **Jokio normalizavimo.** Tarpai nešalinami, raidžių registras ir Unicode
  forma nekeičiami.
- **Išvestis.** 256 bitai = 32 baitai = 64 hex skaitmenys, mažosiomis raidėmis,
  su visais pradiniais nuliais.
- **Dydžio riba.** Nuo v0.2 failas skaitomas ir maišomas 64 KiB dalimis, todėl
  atmintis nuo failo dydžio nepriklauso (1 GiB failas – ≈ 6 MB). Ilgis ir bloko
  numeris maišomi kaip 64 bitų skaičiai, todėl teorinė riba – 2⁶⁴ − 1 baitas.
  Ranka įvedama viena eilutė.

Tuščia įvestis leidžiama: 0 baitų → `dc978b5d0c615428b6647100b74330e6f27aad48bd15aff768e5c6308f2a4a64`.

## Algoritmo idėja

Būsena yra „ratas“ iš 8 stipinų (8 × 32 bitų žodžiai = 256 bitai). Įvestis
absorbuojama po 16 baitų, po kiekvieno bloko ratas pasukamas. Rezultatas
nuskaitomas dviem pusėmis su pasukimu tarp jų, todėl santrauka niekada nėra
visa vidinė būsena.

```
būsena[0..7] = frazė "Vilniaus universitetas, BGT 2026" kaip 8 LE žodžiai

kiekvienam pilnam 16 baitų blokui (numeris 1, 2, …, 64 bitų):
    h = būsena
    būsena[0..3] ^= bloko žodžiai
    būsena[6]    += bloko numeris (aukštieji 32 bitai)
    būsena[7]    += bloko numeris (žemieji 32 bitai)
    sukti 3 kartus               // v0.2: buvo 2
    būsena[0..7] += h[0..7]      // v0.12: feed-forward

paskutinis blokas užpildomas PKCS#7 būdu ir absorbuojamas taip pat

būsena[4] ^= ilgis (žemieji 32 bitai)
būsena[5] ^= ilgis (aukštieji 32 bitai)
būsena[6] ^= 0xFFFFFFFF
f = būsena
sukti 4 kartus
būsena[0..7] += f[0..7]          // v0.2: feed-forward ir pabaigoje

santrauka = būsena[0..3];  sukti 2 kartus;  santrauka += būsena[0..3]
```

Vienas pasukimas (`turn`), kiekvienam stipinui `i`:

```
b = būsena[i+1];  c = būsena[i+3];  d = būsena[i+6]
a = būsena[i] + (b ^ c)
a = rotl(a, d mod 32)     // posūkio dydis priklauso nuo duomenų; v0.11: 0 → 1
a = a ^ (d + konstanta(r))
būsena[i]   = a
būsena[i+1] += rotl(a, 9)  // pokytis iškart perduodamas toliau
```

Sprendimų pagrindimas:

- **Nuo duomenų priklausantis posūkis** (`rotl(a, d)`): pagrindinis
  netiesiškumo šaltinis šalia sudėties moduliu 2³². Nuo v0.11 posūkis visada
  1..31 bitų (žr. „v0.11“).
- **Naujas stipinas iškart perduodamas kitam**, todėl vieno bito pokytis per
  vieną pasukimą apeina visą ratą.
- **Feed-forward** (`būsena += h` po kiekvieno bloko): nuo v0.12 bloko
  apdorojimas nebėra apgręžiamas (žr. „v0.12“). Nuo v0.2 – ir pabaigoje.
- **3 pasukimai bloke** (nuo v0.2): 1 pasukimo neužtenka, 2 – tik minimumas,
  3 palieka atsargą (žr. „v0.2“).
- **64 bitų bloko numeris**: nuo v0.12 numeris nepersisuka net labai
  dideliems failams.
- **Užpildymas ir ilgio įmaišymas**, kad skirtingo ilgio įvestys nesutaptų.
- **Savos konstantos** iš frazės, o ne paimtos iš žinomos maišos funkcijos.

## v0.11

Su DI pagalba buvo diagnozuotos problemos.

Svarstytos keturios v0.1 problemos; įtrauktos dvi.

| # | Problema v0.1 | Sprendimas | v0.11 |
|---|---|---|---|
| 1 | Windows'e failas su ne ASCII vardu (pvz., `ąžuolas.txt`) neatsidaro nei argumentu, nei per meniu `2` (klaida, išėjimo kodas 2): kelias skaitomas ANSI koduote | argumentas imamas iš UTF-16 komandinės eilutės, kelias atidaromas kaip UTF-16 | įtraukta; maišos reikšmės dėl to nesikeičia |
| 2 | `rotl(a, d)` su `d mod 32 = 0` (kas 32-as žingsnis) visai nesuka | posūkis 0 pakeičiamas 1, todėl visada 1..31 bitų | įtraukta; pasikeičia visos maišos reikšmės |
| 3 | Bloko apdorojimas apgręžiamas (permutacija), įvestis XOR'inama į pusę būsenos → teorinė riba ≈ 2⁶⁴ | pridėti būseną prieš bloką (feed-forward) | neįtraukta |
| 4 | 32 bitų bloko numeris persisuka po 64 GiB | 64 bitų numeris | neįtraukta (ilgis ir taip įmaišomas) |

Posūkiui išbandytas ir `d mod 31 + 1` (tolygiai 1..31), bet dalyba sulėtino maišą
≈ 40 % (1 084 → 651 MB/s tame pačiame kompiuteryje), todėl pasirinktas „0 → 1“
(posūkis 1 pasitaiko 2/32, kiti – po 1/32).

### v0.1 ir v0.11 palyginimas

Tie patys duomenys, seed'ai ir kompiuteris (`results/aplinka.md`). v0.1 rezultatai
išsaugoti `results/v0.1/`, v0.11 – `results/`. Viską iš naujo sugeneruoja
`python -X utf8 experiments\run_all.py`.

| Rodiklis | v0.1 | v0.11 |
|---|---|---|
| Teisingumo palyginimai poromis (1–3 eksp.) | 21/21 ✓ | 21/21 ✓ |
| Formatas, determinizmas, A-B-A | 34/34, taip | 34/34, taip |
| Komandinė eilutė (argumentas, ranka, meniu 2) | 34/34, 31/31, 31/31 | 34/34, 31/31, 31/31 |
| Maišos, prasidedančios `0` / `00` (≈ 625 / 39) | 608 / 30 | 626 / 50 |
| Sparta, visas `konstitucija.txt` (75 595 B) | 74,7 µs, 1 011 MB/s | 78,0 µs, 969 MB/s |
| Kolizijos: 4 × 100 000 porų ir visas rinkinys | 0 | 0 |
| Kolizijos sutrumpinus iki 24 / 32 bitų (≈ 4 768 / 18,6) | 4 743 / 13 | 4 750 / 14 |
| Struktūruotos įvestys (119 374 skirtingos) | 0 kolizijų | 0 kolizijų |
| Lavina, simbolio pakeitimas: bitai vid. (min–max) | 50,01 % (36,33–63,28) | 49,98 % (35,94–64,06) |
| Lavina: hex vid. (min–max) | 93,75 % (78,12–100) | 93,75 % (78,12–100) |
| Lavina, vieno bito apvertimas: bitai vid. | 49,99 % | 50,00 % |
| Spėjimas be druskos: `0000`–`9999` perrinkimas | 1,00 ms, 1 sutapimas | 0,97 ms, 1 sutapimas |

Išvados:

- Ne ASCII failų vardų klaida ištaisyta. Tai vienintelis pakeitimas, kurį matyti
  testais: anksčiau buvo klaida, dabar gaunama ta pati maiša kaip ir ASCII vardu.
- Posūkio pakeitimas statistiškai nepastebimas: lavinos, kolizijų ir pradinių
  nulių rezultatai abiem versijoms atitinka atsitiktinės funkcijos lūkesčius
  (skirtumai – atsitiktinių svyravimų ribose). Jis pašalina tik struktūrinį
  trūkumą, kurio šie testai neaptinka.
- Sparta sumažėjo ≈ 4 %.
- Kaip ir v0.1 atveju, geri statistiniai rezultatai nėra saugumo įrodymas;
  3 problema (apgręžiamas bloko apdorojimas) liko.

## v0.12

v0.12 sprendžia abi v0.11 neįtrauktas problemas (3 ir 4).

| # | Problema v0.11 | Sprendimas | v0.12 |
|---|---|---|---|
| 1 | Ne ASCII failų vardai | – | ištaisyta v0.11 |
| 2 | Posūkis 0 | – | ištaisyta v0.11 |
| 3 | Bloko apdorojimas apgręžiamas: žinant būseną po bloko, galima atsukti pasukimus atgal ir rasti būseną prieš jį, todėl galima susitikimo per vidurį paieška (teorinė riba ≈ 2⁶⁴) | prieš bloką būsena išsaugoma (`h`), po pasukimų pridedama: `būsena[i] += h[i]` (feed-forward, kaip Davies–Meyer) | įtraukta; pasikeičia visos maišos reikšmės |
| 4 | 32 bitų bloko numeris persisuka po 2³² blokų (64 GiB), todėl blokai `i` ir `i + 2³²` gauna tą patį numerį | numeris 64 bitų: žemieji 32 bitai pridedami prie stipino 7, aukštieji – prie stipino 6 | įtraukta; failams iki 64 GiB aukštieji bitai lygūs 0 |

Kodėl feed-forward padeda: anksčiau vieno bloko žingsnis `būsena → būsena'` buvo
permutacija, todėl iš bet kokios norimos `būsena'` buvo galima apskaičiuoti
atgal. Dabar `būsena' = F(būsena, blokas) + būsena`, ir norint ją apgręžti
reikia rasti `būsena`, kuriai ši lygybė galioja – tam nebeužtenka tiesiog
sukti atgal.

Kodas taip pat išvalytas: visi komentarai pašalinti iš `ratas.cpp`,
`experiments/impl_ratas.cpp` ir `experiments/run_all.py`; jų turinys yra šiame
README. `experiments/impl_ratas.cpp` įtraukia `ratas.cpp` tiesiogiai (jo `main`
laikinai pervadinamas), todėl eksperimentai visada tikrina tą pačią algoritmo
kopiją kaip ir programa.

### v0.11 ir v0.12 palyginimas

Tie patys duomenys, seed'ai ir kompiuteris. v0.11 rezultatai išsaugoti
`results/v0.11/`, v0.12 – `results/`.

| Rodiklis | v0.11 | v0.12 |
|---|---|---|
| Teisingumo palyginimai poromis (1–3 eksp.) | 21/21 ✓ | 21/21 ✓ |
| Formatas, determinizmas, A-B-A | 34/34, taip | 34/34, taip |
| Komandinė eilutė (argumentas, ranka, meniu 2) | 34/34, 31/31, 31/31 | 34/34, 31/31, 31/31 |
| Maišos, prasidedančios `0` / `00` (≈ 625 / 39) | 626 / 50 | 616 / 48 |
| Sparta, visas `konstitucija.txt` (75 595 B), `run_all.py` | 78,0 µs, 969 MB/s | 98,7 µs, 765 MB/s |
| Sparta, abi versijos paleistos paeiliui, 3 kartus | ≈ 84 µs | ≈ 97 µs |
| Kolizijos: 4 × 100 000 porų ir visas rinkinys | 0 | 0 |
| Kolizijos sutrumpinus iki 24 / 32 bitų (≈ 4 768 / 18,6) | 4 750 / 14 | 4 776 / 25 |
| Struktūruotos įvestys (119 374 skirtingos) | 0 kolizijų | 0 kolizijų |
| Lavina, simbolio pakeitimas: bitai vid. (min–max) | 49,98 % (35,94–64,06) | 49,99 % (36,33–64,06) |
| Lavina: hex vid. (min–max) | 93,75 % (78,12–100) | 93,74 % (76,56–100) |
| Lavina, vieno bito apvertimas: bitai vid. | 50,00 % | 50,00 % |
| Spėjimas be druskos: `0000`–`9999` perrinkimas | 0,97 ms, 1 sutapimas | 1,07 ms, 1 sutapimas |

Išvados:

- Visi teisingumo ir komandinės eilutės testai praeina kaip ir anksčiau.
- Statistiniai rodikliai (lavina, kolizijos, pradiniai nuliai) lieka
  atsitiktinės funkcijos lūkesčių ribose; skirtumai tarp versijų – atsitiktiniai
  svyravimai. Kaip ir v0.11 posūkio pataisa, feed-forward šalina struktūrinį
  trūkumą, kurio šie testai neaptinka.
- Sparta sumažėjo ≈ 15 % (palyginus abi versijas paleistas paeiliui tame
  pačiame kompiuteryje): kiekvienam blokui reikia nukopijuoti ir vėl pridėti
  8 būsenos žodžius. `run_all.py` matavimų skirtumas (≈ 21 %) didesnis, nes
  v0.11 matuota ramesniame kompiuteryje.
- 4 problemos pataisa testais nepatikrinama: tam reikėtų didesnio nei 64 GiB
  failo. Mažesniems failams maiša skiriasi tik dėl feed-forward.
- Geri statistiniai rezultatai vis dar nėra saugumo įrodymas.

## v0.2 (su DI)

**Įrankis:** Claude Code (Anthropic), modelis Claude Opus 5.5. Užklausa: „patobulink
v0.12 pagal užduotį (v0.2), kode be komentarų, paaiškinimai tik README“. DI pirmiausia
išmatavo v0.12 silpnąsias vietas, tada pasiūlė pakeitimus; kiekvienas pasiūlymas
priimtas arba atmestas pagal matavimą.

Algoritmo schema: `docs/algoritmo-schema-v0.2.drawio` (`.png` – ta pati schema paveikslu).

### DI pasiūlymai ir sprendimai

| Pasiūlymas | Sprendimas ir priežastis | Patikra |
|---|---|---|
| 3 pasukimai bloke vietoj 2 | **Priimta.** Bloko žingsnio SAC matavimas (žemiau): su 1 pasukimu 15 752 iš 32 768 langelių aiškiai šališki, su 2 – nė vieno. Vadinasi, 2 pasukimai yra tik minimumas, be jokios atsargos. 3 pasukimai – 1,5 karto daugiau nei minimumas | `tests/sac.cpp` → `results/raw/sac.csv`; sparta – žemiau |
| 4 pasukimai bloke | **Atmesta.** Statistiškai niekuo nesiskiria nuo 3, o sparta krenta proporcingai pasukimų skaičiui (2 → 3 jau kainavo ≈ 29 %) | `results/raw/sac.csv` |
| Feed-forward ir pabaigoje: `f = s; 4 pasukimai; s += f` | **Priimta.** Be jo pabaigos 4 pasukimai yra apgręžiami: kas sužinotų galutinę būseną, galėtų juos atsukti, gauti būseną prieš ilgio įmaišymą ir tęsti maišą (ilgio pratęsimo idėja). Išvestis – 256 bitai iš 256 bitų būsenos, todėl informacijos prasme būseną ji nusako; ar ją galima praktiškai atkurti – netirta. Su feed-forward net atkūrus galutinę būseną ankstesnės nebegalima gauti. Kaina – 16 operacijų vienai maišai | Python realizacija (žemiau) |
| Srautinis maišymas: `Ratas256` su `update` / `finish`, failas skaitomas 64 KiB dalimis | **Priimta.** v0.12 visą failą sudėdavo į atmintį. 1 GiB atsitiktinių baitų failas: v0.12 – 2,61 s ir 1 879 MB atminties, v0.2 – 2,38 s ir 6 MB | `tests/stream_test.cpp`: 608 ilgiai po 5 atsitiktinius skaidymus (dalys 0–40 B) – 0 nesutapimų iš 3 040 |
| Nepriklausoma Python realizacija `tests/ratas_ref.py` | **Priimta** kaip patikra. Parašyta pagal šio README pseudokodą, ne verčiant C++ eilutė po eilutės | `tests/check.py`: programa = Python 36/36 failų (`data/exp1`, `pavyzdys.txt`, `konstitucija.txt`), C++ = Python 608/608 ilgių (0–600 B, 4 KiB, 64 KiB, ≈ 1 MB) |
| Posūkis `(d · 31 >> 32) + 1` – tolygiai 1..31 be dalybos (vietoj v0.11 „0 → 1“) | **Atmesta.** SAC ir lavinos rezultatai nepasikeitė, o sparta sumažėjo ≈ 4 % (829 → 796 MB/s, 1 MiB įvestis) | tas pats SAC matavimas |
| 32 baitų blokai (įvestis į visus 8 stipinus, ≈ 2 kartus greičiau) | **Atmesta.** Tada įvestis valdo visą būseną prieš pasukimus: norimam bloko rezultatui t pakanka x = T⁻¹(t − s), m = x ⊕ s – suspaudimo funkcijos pirmavaizdis be jokios paieškos. Su 16 baitų blokais stipinai 4–7 lieka nevaldomi | analizė |
| 512 bitų būsena (kaip poros partnerio algoritme) | **Atmesta.** Tai partnerio sprendimas; poros realizacijos turi likti atskiros, be to reikėtų perrašyti visą funkciją | – |

### Bloko žingsnio SAC

Vienas bloko žingsnis (įterpimas, pasukimai, feed-forward) iš atsitiktinės būsenos ir
atsitiktinio bloko. Kiekvienam iš 128 bloko bitų – 4 000 porų, kuriose apverstas tas
bitas; kiekvienam iš 256 būsenos bitų skaičiuojama, kaip dažnai jis pasikeitė.
Idealiai – 0,5; „šališkas“ langelis nukrypsta daugiau nei 5σ (0,0395). `turn` funkcija
ta pati kaip programoje, keičiamas tik pasukimų skaičius.

| Pasukimai bloke | Vid. pasikeitusių būsenos bitų (iš 256) | Mažiausiai | Šališki langeliai (iš 32 768) | Didžiausias nuokrypis |
|---|---|---|---|---|
| 1 | 98,84 | 12 | 15 752 | 0,4728 |
| 2 (v0.12) | 128,01 | 90 | 0 | 0,0330 |
| 3 (v0.2) | 128,00 | 90 | 0 | 0,0325 |
| 4 | 128,00 | 90 | 0 | 0,0300 |

### v0.12 ir v0.2 palyginimas

Tie patys duomenys, seed'ai ir kompiuteris. v0.12 rezultatai išsaugoti
`results/v0.12/`, v0.2 – `results/`.

| Rodiklis | v0.12 (be DI) | v0.2 (su DI) |
|---|---|---|
| Teisingumo palyginimai poromis (1–3 eksp.) | 21/21 ✓ | 21/21 ✓ |
| Formatas, determinizmas, A-B-A | 34/34, taip | 34/34, taip |
| Komandinė eilutė (argumentas, ranka, meniu 2) | 34/34, 31/31, 31/31 | 34/34, 31/31, 31/31 |
| Maišos, prasidedančios `0` / `00` (≈ 625 / 39) | 616 / 48 | 637 / 42 |
| Sparta, visas `konstitucija.txt` (75 595 B), `run_all.py` | 98,7 µs, 765 MB/s | 134,0 µs, 564 MB/s |
| Sparta, abi versijos paleistos paeiliui, 3 kartus | ≈ 97 µs | ≈ 137 µs |
| 1 GiB failas per programą: laikas, atmintis | 2,61 s, 1 879 MB | 2,38 s, 6 MB |
| Kolizijos: 4 × 100 000 porų ir visas rinkinys | 0 | 0 |
| Kolizijos sutrumpinus iki 24 / 32 bitų (≈ 4 768 / 18,6) | 4 776 / 25 | 4 771 / 14 |
| Struktūruotos įvestys (119 374 skirtingos) | 0 kolizijų | 0 kolizijų |
| Lavina, simbolio pakeitimas: bitai vid. (min–max) | 49,99 % (36,33–64,06) | 50,01 % (37,50–63,28) |
| Lavina: hex vid. (min–max) | 93,74 % (76,56–100) | 93,75 % (76,56–100) |
| Lavina, vieno bito apvertimas: bitai vid. | 50,00 % | 49,99 % |
| Spėjimas be druskos: `0000`–`9999` perrinkimas | 1,07 ms, 1 sutapimas | 1,12 ms, 1 sutapimas |
| Pasukimai bloke / reikalingas minimumas (SAC) | 2 / 2 (be atsargos) | 3 / 2 (atsarga 1,5×) |
| Pabaiga apgręžiama | taip | ne |
| Patikra su nepriklausoma realizacija | nebuvo | 36/36 failų, 608/608 ilgių |

### Išvados

- **Pagerėjo:** bloko žingsnis turi atsargą (3 pasukimai, kai reikia 2);
  pabaigos nebegalima atsukti; failai maišomi nepriklausomai nuo jų dydžio
  (1 GiB – 6 MB atminties vietoj 1,9 GB); atsirado nepriklausoma patikra.
- **Pablogėjo:** sparta sumažėjo ≈ 26–29 % (laikas × 1,36–1,41), nes kiekvienam
  blokui daromas vienas pasukimas daugiau. Dideliems failams per programą v0.2
  vis tiek greitesnė, nes nebereikia visko sudėti į atmintį.
- **Nepasikeitė:** visi užduoties statistiniai rodikliai (lavina, kolizijos,
  pradiniai nuliai) abiem versijoms atitinka atsitiktinės funkcijos lūkesčius.
  Kaip ir v0.11 bei v0.12, šie pakeitimai šalina struktūrinius trūkumus, kurių
  tokie testai neaptinka – todėl jiems įvertinti prireikė atskiro SAC matavimo.
- **Ko negalima teigti:** kad v0.2 saugi. SAC matuoja tik pavienių bitų
  pasikeitimus – jis neaptiktų didelės tikimybės diferencialų per kelis
  pasukimus; nebandyta atkurti būsenos iš maišos; Python realizaciją parašė tas
  pats DI, todėl ji patvirtina, kad kodas atitinka aprašą, bet ne tai, kad
  aprašas geras.

### Testai

`python -X utf8 experiments\run_all.py` sukompiliuoja programą, eksperimentus ir
testus, paleidžia `tests/check.py` (rezultatas `results/raw/check.txt`; nepavykus
scenarijus sustoja), SAC matavimą (`results/raw/sac.csv`) ir visus 1–7 eksperimentus.
`experiments/impl_ratas.cpp` ir `tests/*.cpp` įtraukia `ratas.cpp` tiesiogiai (jo `main`
laikinai pervadinamas), todėl visur tikrinama ta pati algoritmo kopija kaip programoje.
