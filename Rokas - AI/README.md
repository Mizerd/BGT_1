# 256 bitų maišos funkcija

Savos konstrukcijos maišos (hash) funkcija, parašyta C++20 – porinės užduoties dalis. Iš bet kokios
baitų sekos pagamina **256 bitų** santrauką (64 hex simbolius).

> Funkcija nebuvo kriptografiškai analizuota, todėl netinka slaptažodžiams, parašams ar kitiems
> saugumui jautriems duomenims apsaugoti.

## 1. Kaip paleisti

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build

./build/hash-generator                  # tekstas įvedamas ranka, Enter baigia įvestį
./build/hash-generator --text "hello"
# 2607ba4e2a9bd8521178536b84dffc11bf933871f0e95eeca4357c2e43936c54
./build/hash-generator --file failas.txt

./build/sanity-checks                   # 54 patikros per API
./tests/run_sanity.sh build             # 27 patikros per komandinę eilutę
./experiments/run_all.sh                # visi eksperimentai → results/
```

Naudojamas režimas parašomas klaidų sraute (`Režimas: …`), standartiniame išvedime lieka tik maiša.
Failo režimu maišomas **failo turinys**, o ne jo pavadinimas. Išėjimo kodai: `0` – pavyko,
`1` – blogi argumentai, `2` – nepavyko perskaityti failo ar įvesties.

## 2. Pagrindinė algoritmo idėja

```text
Įvestis → baitai → 32 baitų blokai → 4 × 64 bitų žodžiai
   ↓
512 bitų būsena (8 × uint64_t)
   ↓  maišymas pirmyn → maišymas atgal
   ↓  ilgio įmaišymas + 3 tušti maišymo žingsniai
512 → 256 bitų sulenkimas → 64 hex simboliai
```

## 3. 32 baitų blokas ir 4 žodžiai

Vienas įprastas ASCII simbolis užima vieną baitą. **32 įvesties baitai sudaro vieną pilną
bloką**, kuris skaidomas į keturias 8 baitų dalis, o kiekviena dalis paverčiama vienu
64 bitų skaičiumi (didžiojo galo tvarka):

```text
baitai  0–7  -> word0
baitai  8–15 -> word1
baitai 16–23 -> word2
baitai 24–31 -> word3
```

Jei įvestis ilgesnė: apdorojami pirmi 32 baitai, **būsena nenunulinama**, kiti 32 baitai
apdorojami jau pakeistoje būsenoje, ir taip iki galo. Kiekvienas blokas gauna savo **pozicijos
žymę** (`(i + 1) * kTagStep`), todėl tas pats blokas skirtingose vietose maišosi skirtingai.

## 4. Vidinė būsena

```text
s0  s1  s2  s3  s4  s5  s6  s7      8 × 64 bitai = 512 bitų
```

Būsena pradedama nuo **fiksuotų konstantų**, įrašytų kode (`kLaneInit`) – jos negeneruojamos
iš naujo kiekvieno paleidimo metu, nes **ta pati įvestis visada turi duoti tą pačią maišą**.
Konstantos gautos viena dokumentuota procedūra iš `n^n` dešimtainių skaitmenų, nekopijuojant
jų iš jokios žinomos maišos funkcijos.

## 5. Keturių žodžių įterpimas

```cpp
s[0] += word0 ^ tag;
s[2] ^= word1;
s[4] += word2;
s[6] ^= word3;
```

* `word0` patenka į `s0` (kartu su pozicijos žyme);
* `word1` – į `s2`, `word2` – į `s4`, `word3` – į `s6`;
* naudojama pakaitomis sudėtis ir XOR, kad įėjimo taškai nebūtų vienodi.

> XOR palygina du skaičius bitas po bito. Jei bitai skirtingi, rezultato bitas yra 1;
> jei vienodi – 0.

## 6. Maišymas pirmyn

```cpp
for (i = 1; i < 8; ++i)
    s[i] = (s[i] ^ std::rotl(s[i - 1], 41)) * kMulForward;
```

1. ankstesnė dalis pasukama per **41** bitą, XOR įmaišo ją į dabartinę;
2. daugyba iš nelyginės konstantos paskleidžia pokyčius po visą skaičių;
3. ką tik pakeista dalis **iš karto** naudojama kitai daliai keisti.

```text
s0 -> s1 -> s2 -> s3 -> s4 -> s5 -> s6 -> s7
```

## 7. Maišymas atgal

```cpp
for (i = 6; i >= 0; --i)   // nuo s6 žemyn iki s0
    s[i] = (s[i] + std::rotl(s[i + 1], 53)) * kMulBackward;
```

```text
s0 <- s1 <- s2 <- s3 <- s4 <- s5 <- s6 <- s7
```

Pirmas perėjimas neša įtaką iš kairės į dešinę, antras – atgal iš dešinės į kairę.
Todėl **po vieno bloko kiekviena iš aštuonių dalių priklauso nuo visų keturių žodžių**.
Tai nėra saugumo įrodymas – tik paaiškinimas, kodėl pokytis nelieka vienoje vietoje.

## 8. Likutis ir įvesties ilgis

Jei įvesties ilgis nesidalija iš 32, likusieji baitai (0–31) surašomi į **nuliais užpildytą
32 baitų buferį** ir apdorojami tokiu pat žingsniu. Pabaigos baitas (pvz. `0x80`) nededamas –
vietoj to **likučio ilgis įmaišomas į to žingsnio žymę** (`(likutis + 1) * kTagTail`).
Būtent tai skiria `"ab"` nuo `"ab\0"`.

* 31 baitas – nulis pilnų blokų, 31 baitas patenka į likučio žingsnį;
* 33 baitai – vienas pilnas 32 baitų blokas, po to 1 baitas likučio žingsnyje.

Likučio žingsnis vykdomas **visada**, net kai įvestis dalijasi iš 32 arba yra tuščia.
Vėliau atskiru žingsniu įmaišomas ir **tikslus bendras įvesties ilgis** (`uint64_t`).

## 9. Galutinis maišymas ir 256 bitų rezultatas

Apdorojus visą įvestį: įmaišomas tikslus ilgis, tada atliekami **3 maišymo žingsniai be
įvesties** (kad paskutiniai baitai būtų sumaišyti taip pat gerai kaip pirmieji). Būsena vis
dar 512 bitų, bet išvedama tik pusė tiek:

```text
out0 = s0 XOR rotl(s4, 40)
out1 = s1 XOR rotl(s5, 40)
out2 = s2 XOR rotl(s6, 40)
out3 = s3 XOR rotl(s7, 40)
```

```text
4 × 64 bitai = 256 bitai = 32 baitai = 64 hex simboliai
```

## 10. Pseudokodas ir sprendimų pagrindimas

```text
s[0..7] = LANE_INIT                                     # 512 bitų, aritmetika mod 2^64

STEP(w0, w1, w2, w3, tag):
    s0 += w0 XOR tag;  s2 ^= w1;  s4 += w2;  s6 ^= w3
    for i = 1..7:          s[i] = (s[i] XOR rotl(s[i-1], 41)) * MUL_F
    for i = 6 down to 0:   s[i] = (s[i]  +  rotl(s[i+1], 53)) * MUL_B

HASH(input):
    L = baitų skaičius;  n = L / 32;  r = L mod 32
    for i = 0..n-1:  STEP(4 žodžiai iš bloko i, (i+1) * TAG_STEP)
    likutis = paskutiniai r baitų + (32 - r) nulinių baitų
    STEP(4 žodžiai iš likučio, (n+1) * TAG_STEP + (r+1) * TAG_TAIL)
    STEP(L, rotl(L, 32), 0, 0, TAG_END)                 # ilgis
    for j = 1..3:  STEP(0, 0, 0, 0, TAG_END + j * TAG_STEP)
    for i = 0..3:  out[i] = s[i] XOR rotl(s[i+4], 40)
    return out[0..3] didžiojo galo tvarka               # 32 baitai = 64 hex
```

* **Du perėjimai** – kad per vieną žingsnį visos būsenos dalys priklausytų nuo viso bloko.
* **Daugyba iš nelyginės konstantos** – netiesinė dalis; nelyginė daugyba apverčiama, todėl būsena
  „nesusispaudžia“ ir informacija neprarandama.
* **Žymės** – bloko pozicija ir likučio ilgis įmaišomi skaičiais, todėl nereikia papildomo `0x80` baito.
* **512 → 256 sulenkimas** – kad maiša neatskleistų visos būsenos (žr. 11 skyrių).
* **Didysis galas ir aiškūs poslinkiai** vietoj `reinterpret_cast` – rezultatas nepriklauso nuo procesoriaus baitų tvarkos.

## 11. Kodėl 512 bitų būsena?

Ankstesnė versija turėjo **256 bitų būseną ir grąžindavo ją visą**. Kadangi beveik visos
vidinės operacijos yra apverčiamos, o pradinė būsena ir žymės yra viešos, iš santraukos buvo
galima tiesiogiai eiti skaičiavimu atgal ir trumpą žinutę atstatyti algebriškai. Eksperimentas
tai patvirtino: **10 000 iš 10 000** atsitiktinių iki 15 baitų žinučių atkurta iš 1 versijos
maišos, vidutiniškai per 0,09 µs (14.6 skyrius).

Dabartinė versija turi **512 bitų būseną, o grąžina tik 256 bitų derinį**, todėl santrauka
nebeatskleidžia visos galutinės būsenos.

> Tai pašalina akivaizdų ankstesnės versijos tiesioginio atstatymo kelią, tačiau neįrodo
> kriptografinio saugumo ar atsparumo pirmavaizdžio paieškai.

## 12. Įvesties apdorojimas ir determinizmas

* **Ranka** (be argumentų): įvedama viena eilutė; **Enter simbolis į įvestį neįtraukiamas**, kiti
  simboliai (tarpai, `\r`) paliekami. Tuščia eilutė – tuščia įvestis; jei neįvesta nė eilutė – klaida.
* **`--text`**: argumento baitai tokie, kokius perdavė apvalkalas (Linux – UTF-8); tarpai nenukerpami,
  raidžių dydis nekeičiamas, tekstas nenormalizuojamas, naujos eilutės simbolis nepridedamas.
* **`--file`**: failas atidaromas dvejetainiu režimu, maišomi tikslūs baitai, eilučių pabaigos
  nekeičiamos; neperskaitytas failas duoda klaidą, o ne tuščios įvesties maišą.
* Vienodi baitai → vienoda maiša; nenaudojamas laikas, atsitiktinumas ar globali būsena.
* Rezultatas visada – 64 mažosiomis raidėmis rašomi šešioliktainiai simboliai su pradiniais nuliais.

Vienodai atrodantis tekstas gali turėti skirtingus baitus (pvz. LF ir CRLF), todėl ir maišos skiriasi.

## 13. Praktiniai įvesties apribojimai

Pats algoritmas apdoroja įvestį blokais, todėl teoriškai tinka bet kokio ilgio baitų sekai, o ilgis
skaičiuojamas 64 bitų `uint64_t` reikšme. Tačiau **komandinės eilutės programa pirmiausia įkelia visą
failą į atmintį** (`std::vector<std::uint8_t>`), todėl praktinę failo ribą lemia turima RAM ir adresų
erdvė. Ranka įvedama tik viena eilutė; kelių eilučių tekstui naudojamas `--file`.

## 14. Eksperimentai

Visi eksperimentai paleidžiami viena komanda `./experiments/run_all.sh`: ji sukompiliuoja lyginamas
realizacijas, sugeneruoja įvestis ir perrašo `results/`. Neapdoroti matavimai – `results/raw/*.csv`,
išsamūs rezultatai – `results/exp*.md`.

| | |
|---|---|
| Aplinka | Intel Core i9-10900K, 62 GB RAM, NixOS 26.05 (Linux 6.18), g++ 15.2, `-std=c++20 -O3 -DNDEBUG`, viena gija ([aplinka.md](results/aplinka.md)) |
| Bendri poros duomenys | `Joringis-no AI/data/exp1/` (34 įvestys) ir `konstitucija.txt` (789 eilutės, 75 595 B) |
| Atsitiktinės įvestys | `std::mt19937_64`, bazinis seed 20260920, abėcėlė `!`..`~` (94 ASCII simboliai, 1 simbolis = 1 baitas) |
| Lyginamos realizacijos | **2 versija** (dabartinė), **1 versija** (256 bitų būsena, commit `b3d54be`), poros **Ratas v0.1** |

Visos trys realizacijos kviečiamos per tą pačią sąsają ir maišo tuos pačius baitus toje pačioje aplinkoje.

### 14.1 Įvestys, formatas, determinizmas (1–3)

| Patikra | 2 versija | 1 versija | Ratas v0.1 |
|---|---|---|---|
| Porų palyginimai: 1 baito pakeitimas pradžioje / viduryje / gale, tvarka, tarpai, nauja eilutė, CRLF, nulinis baitas, 15/16/17 B | 22/22 | 22/22 | 22/22 |
| 64 hex simboliai, vienodas raidžių dydis, pradiniai nuliai | 35/35 | 35/35 | 35/35 |
| Kartotiniai kvietimai, seka A, B, A, 1 000 kvietimų | taip | taip | taip |

Komandinė eilutė (2 versija): du atskiri paleidimai sutampa **34/34**, ranka įvestas tekstas ir
`--text` sutampa su failo maiša **31/31**. UTF-8: `utf8_lt.txt` – 15 simbolių, bet 26 baitai;
`utf8_mixed.txt` – 19 simbolių, 23 baitai. Pradiniai nuliai: `0321` → `00de9b72…` (64 simboliai).
Bendrame rinkinyje `struct_newline_crlf.txt` iš tikrųjų saugomas su LF, todėl tikras CRLF atvejis
papildomai sudarytas atmintyje. Išsamiai: [exp1_3_teisingumas.md](results/exp1_3_teisingumas.md).

### 14.2 Sparta (4)

Ištraukos po 1, 2, 4, … `konstitucija.txt` eilučių ir visas failas; matuojamas tik maišos skaičiavimas
(įvestis paruošta iš anksto, be I/O). 3 apšilimo ir 10 matavimų kiekvienam dydžiui, viename matavime
maiša kartojama, kol praeina ≥ 20 ms. Laikas vienai maišai, µs – vidurkis (min–max):

| Eilutės | Baitai | 2 versija | 1 versija | Ratas v0.1 |
|---|---|---|---|---|
| 1 | 70 | 0,097 (0,096–0,100) | 0,024 (0,024–0,025) | 0,125 (0,125–0,127) |
| 2 | 123 | 0,113 (0,110–0,117) | 0,032 (0,031–0,032) | 0,168 (0,168–0,172) |
| 4 | 205 | 0,154 (0,153–0,157) | 0,045 (0,044–0,045) | 0,242 (0,241–0,245) |
| 8 | 362 | 0,226 (0,225–0,229) | 0,072 (0,071–0,072) | 0,386 (0,384–0,396) |
| 16 | 996 | 0,514 (0,512–0,517) | 0,179 (0,179–0,180) | 0,963 (0,959–0,973) |
| 32 | 1 841 | 0,887 (0,884–0,893) | 0,323 (0,322–0,329) | 1,731 (1,728–1,736) |
| 64 | 3 712 | 1,737 (1,730–1,757) | 0,638 (0,635–0,650) | 3,428 (3,416–3,447) |
| 128 | 9 155 | 4,194 (4,166–4,293) | 1,550 (1,548–1,553) | 8,370 (8,330–8,548) |
| 256 | 20 409 | 9,221 (9,196–9,340) | 3,443 (3,435–3,470) | 18,559 (18,491–18,760) |
| 512 | 47 434 | 21,366 (21,308–21,473) | 7,996 (7,969–8,108) | 43,015 (42,890–43,431) |
| 789 | 75 595 | 34,008 (33,921–34,065) | 12,740 (12,692–12,861) | 68,884 (68,337–69,801) |

![Vienos maišos skaičiavimo laikas pagal įvesties dydį](results/exp4_sparta.svg)

* Nuo ~1 KB laikas auga **tiesiškai** su įvesties dydžiu (log-log grafike – tiesė): 2 versijai
  ≈ 0,45 ns baitui, t. y. ≈ 2,2 GB/s.
* **Mažoms įvestims** kreivė išsilygina: net 70 B įvestis pereina 5 fiksuotus žingsnius (likutis, ilgis,
  3 tušti), todėl 0,097 µs – daugiausia pastovios išlaidos.
* 2 versija ≈ **2,7 karto lėtesnė** už 1 versiją (14 daugybų 32 baitams vietoj 4) – tai didesnės
  būsenos kaina. Matavimų sklaida maža: min–max skiriasi ne daugiau kaip 5,5 %, dažniausiai < 3 %.

### 14.3 Kolizijos (5)

Kiekvienam ilgiui 100 000 atsitiktinių porų (poros narės visada skiriasi) ir paieška tarp visų
200 000 įvesčių. Reikšmės: 2 versija / 1 versija / Ratas v0.1.

| Ilgis | Porų | Kolizijos porose | Kolizijų grupės tarp visų įvesčių |
|---|---|---|---|
| 10 | 100 000 | 0 / 0 / 0 | 0 / 0 / 0 |
| 100 | 100 000 | 0 / 0 / 0 | 0 / 0 / 0 |
| 500 | 100 000 | 0 / 0 / 0 | 0 / 0 / 0 |
| 1 000 | 100 000 | 0 / 0 / 0 | 0 / 0 / 0 |

Idealiai 256 bitų maišai 200 000 įvesčių rinkinyje tikėtina ≈ m(m−1)/2 · 2^(−256) ≈ 10^(−67) kolizijų,
todėl nulis – įprastas rezultatas, kuris saugumo **neįrodo**. Kad testas tikrai randa kolizijas, tie
patys rinkiniai patikrinti su sutrumpintomis maišomis (visi keturi ilgiai kartu):

| Sutrumpinta iki | Tikėtina | 2 versija | 1 versija | Ratas v0.1 |
|---|---|---|---|---|
| 24 bitai | 4 768 | 4 654 | 4 807 | 4 743 |
| 32 bitai | 18,6 | 19 | 17 | 13 |
| 40 bitai | 0,07 | 0 | 0 | 0 |

Struktūruoti rinkiniai – visi 1 ir 2 baitų variantai, nuliniai baitai ir `a` kartojimas ilgiu 0–4096,
32 B bloko kartojimas, `abcdefgh` perstatymai, blokų tvarka XY / YX, `ab` su nuliais priekyje ir gale,
vieno bito pakeitimai: **119 374 skirtingos įvestys, 0 kolizijų** visose trijose realizacijose.
Išsamiai: [exp5_kolizijos.md](results/exp5_kolizijos.md).

### 14.4 Lavinos efektas (6)

100 000 porų (po 25 000 kiekvienam ilgiui), kiekvienoje pakeistas vienas atsitiktinis simbolis kitu
tos pačios abėcėlės simboliu. Bitai lyginami **dekodavus hex**. Bitų skirtumas, % – vidurkis (min–max):

| Ilgis | 2 versija | 1 versija | Ratas v0.1 |
|---|---|---|---|
| 10 | 49,99 (36,72–62,11) | 50,02 (37,89–62,50) | 50,02 (39,06–62,50) |
| 100 | 50,00 (38,67–61,33) | 49,99 (37,11–62,50) | 50,00 (36,33–61,72) |
| 500 | 50,03 (37,89–62,50) | 50,00 (36,72–62,11) | 50,02 (37,50–62,50) |
| 1 000 | 49,97 (38,67–64,45) | 50,03 (36,72–62,89) | 49,99 (38,28–63,28) |
| **visi** | **50,00** (36,72–64,45) | **50,01** (36,72–62,89) | **50,01** (36,33–63,28) |

Hex skirtumas visoms poroms: **93,75 % / 93,76 % / 93,75 %** (min 76,56–78,12 %, max 100 %; orientyras 93,75 %).
Bitų skirtumo standartinis nuokrypis 3,12 % – beveik lygus idealiam 3,13 %. Apvertus vieną įvesties bitą
(papildomas testas) – 50,01 % / 49,99 % / 49,99 %.

![Bitų skirtumo histograma](results/exp6_histograma.svg)

**Ar geras lavinos efektas gali egzistuoti kartu su lengvomis kolizijomis?** Taip. Pavyzdžiui, funkcija,
kuri ignoruoja galinius nulius ar dalį įvesties, pakeistiems baitams rodytų idealią lavina, bet `ab` ir
`ab\0` sutaptų. Tokias silpnybes atskleidžia struktūruoti testai (nuliai, ilgiai, blokų tvarka), o ne
lavinos matavimas. Mūsų 1 versija – tikras to pavyzdys: lavinos efektas idealus, bet trumpos žinutės
atkuriamos iš maišos (14.6). Išsamiai: [exp6_lavina.md](results/exp6_lavina.md).

### 14.5 Spėjimas, druska ir slaptas atsitiktinumas (7)

Kandidatai `0000`–`9999`; atakai pateikiama tik tikslinė maiša. 2 versijos rezultatai:

| Atvejis | Maišos skaičiavimų | Laikas | Rezultatas |
|---|---|---|---|
| Be druskos, 1 taikinys | 3 984 iki radimo, 10 000 visas perrinkimas | 0,76 ms | vienintelis sutapimas `3983` |
| Be druskos, 5 taikiniai, viena iš anksto apskaičiuota lentelė | 10 000 | 2,52 ms | 5/5 |
| Vieša 16 B druska, kiekvienam taikiniui atskira | 5 × 10 000 = 50 000 | ≈ 1 ms taikiniui | 5/5, po 1 sutapimą |
| Slapta 16 B `r`, `H(input ‖ r)` | 10 000 · 2^128 – neatliekama | – | atskleidus `r`, patikra – 1 maiša |

* Mažą kandidatų aibę perrinkti trunka < 1 ms, nors išvestys atrodo atsitiktinės: sunkumą lemia **paieškos
  erdvė**, o ne maišos „atsitiktinumas“.
* Sutapimas čia identifikuoja įvestį, nes rinkinyje jis vienintelis; bendru atveju ne – kolizijos egzistuoja.
* Druska vienam taikiniui darbo nepadidina, bet **neleidžia vienos lentelės panaudoti visiems** taikiniams.
* Slaptas `r` iliustruoja įsipareigojimą (commitment), bet neįrodo, kad ši konstrukcija saugiai paslepia
  pranešimą. Tai ir ne darbo įrodymo (proof-of-work) galvosūkis. Išsamiai: [exp7_spejimas.md](results/exp7_spejimas.md).

### 14.6 1 versijos silpnybė: žinutės atkūrimas iš maišos

1 versijoje (256 bitų būsena, visa išvedama) iki 15 baitų žinutė patenka į vieną likučio žingsnį. Iš
maišos atšaukiami 3 tušti žingsniai ir ilgio žingsnis, o likučio žingsnyje lieka nežinomi tik du žodžiai:
dvi būsenos dalys, kurių jie nepaliečia, turi sutapti su žinomomis pradinėmis reikšmėmis (taip patikrinamas
spėtas ilgis), o likusios dvi tiesiog atiduoda žodžius. Rezultatas: **10 000/10 000** atsitiktinių 0–15 baitų
žinučių atkurta, vidutiniškai **0,09 µs** – be jokio perrinkimo (pvz. `hello` → `dc39127a…` → `hello`).
2 versijoje šis kelias neprasideda, nes trūksta 256 būsenos bitų. Išsamiai: [exp8_atstatymas.md](results/exp8_atstatymas.md).

## 15. Versijų palyginimas ir išvados

| | 1 versija | 2 versija |
|---|---|---|
| Būsena → išvestis | 256 b, visa išvedama | 512 b → 256 b sulenkimas |
| Blokas; daugybos 32 baitams | 16 B; 4 | 32 B; 14 |
| Sparta visam failui | 5 933 MB/s | 2 223 MB/s |
| Kolizijos (800 000 atsitiktinių + 119 374 struktūruotos) | 0 | 0 |
| Lavinos efektas: bitai / hex | 50,01 % / 93,76 % | 50,00 % / 93,75 % |
| Žinutės (≤ 15 B) atkūrimas iš maišos | 10 000/10 000, 0,09 µs | tuo pačiu keliu neįmanomas |

* **Pagerėjo:** pašalintas tiesioginis atkūrimo kelias – vienintelė išmatuota esminė silpnybė.
* **Pablogėjo:** ~2,7 karto lėčiau.
* **Nepasikeitė:** kolizijų ir lavinos testai abiem versijoms vienodai geri – todėl 1 versijos silpnybės
  jie **neatskleidė**; ją parodė tik struktūros analizė ir atkūrimo ataka.

**Ko eksperimentai neįrodo:** 0 kolizijų tarp ~10^6 įvesčių nieko nesako apie 2^128 darbo ribą; lavinos
efektas matuoja vidutinį pokytį, ne atsparumą tikslinėms atakoms; 2 versijos atsparumas pirmavaizdžiui
ir kolizijoms neįrodytas; sparta išmatuota viename kompiuteryje.

**Sąsaja su paskaitos sąvokomis:**

* **Pirmavaizdis** – mažą rinkinį perrinkome per < 1 ms (14.5), o 1 versijoje jis gaunamas net be perrinkimo (14.6).
* **Kolizijos** – egzistuoja neišvengiamai (begalė įvesčių → 2^256 reikšmių); sutrumpintos maišos rodo
  gimtadienio paradoksą (14.3).
* **Lavinos efektas** – ≈ 50 % bitų, pasiskirstymas sutampa su B(256; 0,5) (14.4).
* **Patikimos maišos reikšmės** – determinizmas, fiksuotas ilgis ir tikslūs baitai (14.1) yra būtina sąlyga,
  kad maiša tiktų blokams susieti ar Merkle įrodymams, bet realiai sistemai reikia kriptografinės maišos (pvz. SHA-256).

## 16. Palyginimas su poros realizacija

Tie patys duomenys, ta pati aplinka ir sąsaja (Ratas v0.1 – commit `fc53d3b`):

| | 2 versija | Ratas v0.1 |
|---|---|---|
| 1–3 eksperimentų patikros | visos | visos |
| Sparta: 70 B / visas failas | 0,097 µs / 2 223 MB/s | 0,125 µs / 1 097 MB/s |
| Kolizijos (atsitiktinės + struktūruotos) | 0 | 0 |
| Lavinos efektas: bitai / hex | 50,00 % / 93,75 % | 50,01 % / 93,75 % |
| Kandidatų `0000`–`9999` perrinkimas | 0,76 ms | 0,66 ms |

Pagal šiuos matus abi realizacijos statistiškai nesiskiria; 2 versija ≈ 2 kartus greitesnė ilgiems failams,
Ratas – šiek tiek greitesnis 4 baitų įvestims.

## 17. Žinomi apribojimai

* sava konstrukcija, nerecenzuota kriptografų, be jokių saugumo garantijų;
* vienas maišymo žingsnis 32 baitų blokui – nedidelė atsargos riba;
* galutinis 512 → 256 sulenkimas paprastas (tiesinis); nėra nei rakto, nei druskos (*salt*);
* komandinė eilutė įkelia visą failą į atmintį, ranka įvedama tik viena eilutė;
* išbandyta tik Linux sistemoje.

## 18. DI naudojimas ir originalumas

Ši realizacija – porinės užduoties dalis, kurią leidžiama kurti su DI nuo pradžių.

* **Įrankis:** Claude Code (Anthropic), Claude Opus modeliai.
* **Svarbiausios užklausos:** sukurti savą 256 bitų maišos funkciją C++20, prieš tai peržiūrėjus žinomas
  maišų šeimas, kad nebūtų atkartota jų struktūra; pašalinti tiesioginio atkūrimo silpnybę išplečiant
  būseną; parengti README gynimui; atlikti 1–8 eksperimentus su tais pačiais duomenimis kaip poros realizacija.
* **Priimta:** konstantos iš `n^n` skaitmenų procedūros; visada vykdomas likučio žingsnis su ilgiu žymėje
  vietoj `0x80` baito; 512 bitų būsena su sulenkimu.
* **Atmesta ar pakeista:** 1 versijos 256 bitų būsena (dėl atkūrimo silpnybės); 19 skaitmenų gabalai
  konstantoms (< 10^19, todėl vyresnieji bitai dažnai būdavo nuliai) pakeisti 20 skaitmenų; xor-shift-multiply finalizatorius
  (per daug panašus į MurmurHash / xxHash); papildoma paslėpta būsenos dalis 1 versijoje buvo atmesta dėl
  paprastumo – vėliau paaiškėjo, kad būtent jos trūko.
* **Patikra:** nepriklausoma Python realizacija sutapo baitas į baitą; 54 + 27 patikrų; ASan / UBSan;
  kompiliavimas be įspėjimų; konstantos palygintos su žinomų maišų konstantomis; 1 versijos silpnybė
  patvirtinta atkūrimo programa (14.6).

Prieš projektuojant peržiūrėtos SHA-2, SHA-3, BLAKE2/3, SipHash, MurmurHash3, xxHash, CityHash, FarmHash ir
FNV – sąmoningai nenaudojamos jų konstantos, raundų funkcijos ar galutinio maišymo procedūros.

## 19. Šaltiniai

* VU „Blokų grandinių technologijos“, 1 užduotis ir kontrolinis sąrašas (2026); `konstitucija.txt` – kurso medžiaga.
* NIST, [Hash Functions](https://csrc.nist.gov/projects/hash-functions) (SHA-2, SHA-3).
* [BLAKE2](https://www.blake2.net/), [BLAKE3](https://github.com/BLAKE3-team/BLAKE3-specs),
  [SipHash](https://cr.yp.to/siphash/siphash-20120918.pdf), [xxHash](https://github.com/Cyan4973/xxHash/blob/dev/doc/xxhash_spec.md),
  [MurmurHash3](https://github.com/aappleby/smhasher), [CityHash](https://github.com/google/cityhash),
  [FNV](https://datatracker.ietf.org/doc/draft-eastlake-fnv/) – originalumo peržiūrai.

## 20. Būsena

* Realizacija, patikros ir visi 8 eksperimentai **atlikti** (`results/`).
* Poros realizacija Ratas v0.1 palyginta tais pačiais duomenimis (16 skyrius).
* Dar liko: bendras poros README su indėliais ir versijų žymos (tags).
