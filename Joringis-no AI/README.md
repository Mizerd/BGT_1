# Ratas-256 v0.12

Mokomoji 256 bitų maišos funkcija (BGT 1 užduotis, porinio darbo pusė be DI pagalbos).

> **DI naudojimas.** v0.1-0.11 sukurta be DI pagalbos.
Visas kodas yra viename faile `ratas.cpp`. Nuo v0.12 kode komentarų nėra –
visi paaiškinimai yra šiame README.

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
Ratas-256: 845e28147df7cde265fde49c6375aaed6375d8072ac93a18deea6d41028ffd0e
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
- **Dydžio riba.** Visa įvestis sudedama į atmintį, todėl praktinė riba yra
  laisva operatyvioji atmintis. Ilgis maišomas kaip 64 bitų skaičius.

Tuščia įvestis leidžiama: 0 baitų → `5c39cf532414769f66f73b799efcb7b3ac4f5dac1cb87914d96a62b4dce77449`.

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
    sukti 2 kartus
    būsena[0..7] += h[0..7]      // v0.12: feed-forward

paskutinis blokas užpildomas PKCS#7 būdu ir absorbuojamas taip pat

būsena[4] ^= ilgis (žemieji 32 bitai)
būsena[5] ^= ilgis (aukštieji 32 bitai)
būsena[6] ^= 0xFFFFFFFF
sukti 4 kartus

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
  apdorojimas nebėra apgręžiamas (žr. „v0.12“).
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
