# 256 bitų maišos funkcija

Savos konstrukcijos maišos (hash) funkcija, parašyta C++20 – **DI pagalba kurta** porinės
užduoties pusė. Iš bet kokios baitų sekos pagamina **256 bitų** santrauką (64 hex simbolius).

> Funkcija nebuvo kriptografiškai analizuota, todėl netinka slaptažodžiams, parašams ar kitiems
> saugumui jautriems duomenims apsaugoti.

## 1. Kaip paleisti

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build

./build/hash-generator --text "hello"
# 2607ba4e2a9bd8521178536b84dffc11bf933871f0e95eeca4357c2e43936c54
./build/hash-generator --file failas.txt

./build/sanity-checks                   # 54 patikros per API
./tests/run_sanity.sh build             # 22 patikros per komandinę eilutę
```

Failo režimu maišomas **failo turinys**, o ne jo pavadinimas. Išėjimo kodai: `0` – pavyko,
`1` – blogi argumentai, `2` – failo nepavyko perskaityti.

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

## 10. Kodėl 512 bitų būsena?

Ankstesnė versija turėjo **256 bitų būseną ir grąžindavo ją visą**. Kadangi beveik visos
vidinės operacijos yra apverčiamos, o pradinė būsena ir žymės yra viešos, iš santraukos buvo
galima tiesiogiai eiti skaičiavimu atgal ir trumpą žinutę atstatyti algebriškai.

Dabartinė versija turi **512 bitų būseną, o grąžina tik 256 bitų derinį**, todėl santrauka
nebeatskleidžia visos galutinės būsenos.

> Tai pašalina akivaizdų ankstesnės versijos tiesioginio atstatymo kelią, tačiau neįrodo
> kriptografinio saugumo ar atsparumo pirmavaizdžio paieškai.

## 11. Determinizmas ir įvesties apdorojimas

* vienodi baitai → vienoda santrauka; nenaudojamas laikas, atsitiktinumas ar globali būsena;
* tekstas maišomas toks, koks perduotas – raidžių dydis nekeičiamas, tarpai nenukerpami,
  naujos eilutės simbolis nepridedamas;
* failai atidaromi dvejetainiu režimu, maišomi tikslūs jų baitai; neperskaitytas failas duoda
  klaidą, o ne tuščios įvesties santrauką;
* rezultatas visada – 64 mažosiomis raidėmis rašomi šešioliktainiai simboliai.

## 12. Praktiniai įvesties apribojimai

Pats algoritmas apdoroja įvestį blokais, todėl teoriškai tinka bet kokio ilgio baitų sekai,
o ilgis skaičiuojamas 64 bitų `uint64_t` reikšme. Tačiau **dabartinė komandinės eilutės
programa pirmiausia įkelia visą failą į atmintį** (`std::vector<std::uint8_t>`). Todėl
praktinę failo ribą lemia turima RAM ir adresų erdvė. Srautinis (angl. *streaming*)
skaitymas šiame etape neįgyvendintas.

## 13. Patikrinimai

`sanity-checks` (54 patikros) ir `run_sanity.sh` (22 patikros) tikrina:

* tuščią įvestį, trumpas eilutes (`a`, `b`, `hello`, `Hello`, `abc`, `cba`), dvejetainius duomenis;
* determinizmą A, B, A ir pakartotinius programos paleidimus;
* 15/16/17 ir 31/32/33 baitų ribas bei pakeitimus pradžioje, viduryje ir gale;
* `hello` ir `hello\n` skirtumą, CRLF/LF failus, failo ir tokio pat teksto sutapimą;
* klaidas dėl neskaitomo failo ir rezultato formą (32 baitai, 64 mažosios hex raidės).

Taip pat: kompiliavimas be įspėjimų (`-Wall -Wextra -Wpedantic`) ir švarus ASan/UBSan
paleidimas. Kūrimo metu atliktas **preliminarus** lavinos efekto (angl. *avalanche*)
matavimas – jis rodo tik tai, kad akivaizdžios struktūros nematyti, ir **nieko neįrodo apie
saugumą**. Oficialūs užduoties eksperimentai dar neatlikti.

## 14. Žinomi apribojimai

* sava konstrukcija, nerecenzuota kriptografų, be jokių saugumo garantijų;
* vienas maišymo žingsnis 32 baitų blokui – nedidelė atsargos riba;
* galutinis 512 → 256 sulenkimas paprastas (tiesinis); nėra nei rakto, nei druskos (*salt*);
* komandinė eilutė įkelia visą failą į atmintį;
* kolizijų, pirmavaizdžio ir lavinos efekto eksperimentai dar nepadaryti.

## 15. DI naudojimas ir originalumas

Prieš projektuojant buvo peržiūrėtos įprastos maišos funkcijų šeimos – SHA-2, SHA-3,
BLAKE2/3, SipHash, MurmurHash3, xxHash, CityHash, FarmHash ir FNV – kad nebūtų atkartota
esama konstrukcija. Sąmoningai nenaudojamos jų konstantos, raundų funkcijos ar galutinio
maišymo procedūros.

## 16. Ką dar reikės padaryti

* DI pusės realizacija **padaryta ir patikrinta**;
* dar reikia poros nario savarankiškos, be DI parašytos realizacijos;
* poros `v0.1` žyma ar leidimas dar **nekuriami**;
* bendri atkartojami eksperimentai bus daromi, kai bus abi realizacijos.
