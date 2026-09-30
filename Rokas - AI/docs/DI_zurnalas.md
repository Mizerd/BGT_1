# DI sąveikos žurnalas

Įrankis: Claude Code (Anthropic), Claude Opus modeliai. Sprendimus priėmė ir rezultatus tikrino studentas.

| Etapas | Užklausa | Svarbiausi pasiūlymai | Priimta / atmesta ir kodėl | Patikra |
|---|---|---|---|---|
| Pirmasis dizainas (09-20) | Sukurti savą 256 bitų maišą C++20, peržiūrėjus žinomas maišų šeimas, kad nebūtų atkartota jų struktūra | 4 × 64 b būsena su žiedine grandine; konstantos iš `n^n` skaitmenų; likučio ilgis žymėje vietoj `0x80` | Priimta. Atmesta: xor-shift-multiply finalizatorius (per daug panašus į MurmurHash / xxHash); 19 skaitmenų gabalai konstantoms (vyresnieji bitai dažnai nuliai) – pakeisti 20 skaitmenų | Nepriklausoma Python realizacija sutapo baitas į baitą; ASan / UBSan; konstantos palygintos su žinomų maišų konstantomis |
| 512 bitų būsena (09-20) | Pašalinti silpnybę: maiša atskleisdavo visą būseną, o žingsniai apverčiami | 8 × 64 b būsena, perėjimas pirmyn ir atgal, 512 → 256 sulenkimas | Priimta. Atmesta: dvi atskiros 4 dalių pusės, kurias jungia tik keli ryšiai – pokytis per lėtai pereitų tarp pusių | Pirmojoje versijoje 10 000 / 10 000 iki 15 B žinučių atkurta iš maišos; naujoje tas kelias nebeegzistuoja; lavinos efektas ≈ 50 % |
| Eksperimentai (09-23) | Atlikti 1–8 eksperimentus su tais pačiais duomenimis kaip poros realizacija; pridėti rankinį įvedimą | Bendra eksperimentų programa su adapteriais; `std::mt19937_64`, seed 20260920; sutrumpintų maišų kolizijų patikra | Priimta. Atmesta: pirmosios versijos palyginimas README – palikta tik dabartinė realizacija | Ratas-256 rezultatai Windows (MSVC) ir Linux (g++) sutapo baitas į baitą; sutrumpintų maišų kolizijos atitinka gimtadienio įvertį |
| V0.11 patikra (09-29) | Išsamiai patikrinti realizaciją, pataisyti tikras problemas, nekeičiant algoritmo | Pataisyti CRLF testinį failą; pridėti žinomų atsakymų testus; griežtesnės lavinos patikros; duomenų kontrolinės sumos | Priimta. Atmesta: algoritmo pakeitimai – atidėta V0.12 | Maišos reikšmės nepakito (10 žinomų atsakymų, nepriklausoma Python realizacija); 57 + 32 patikros; rezultatai sugeneruoti iš naujo |
| V0.12 optimizavimas ir testai (09-30) | Išmatuoti spartą neapkrautame kompiuteryje, optimizuoti ir daugiau testuoti, nekeičiant maišos reikšmių | Išmatuoti ciklus žingsniui ir palyginti su teorine riba (14 nuoseklių daugybų ≈ 71 ciklas); `Hasher` failams skaityti dalimis; GCC `-fno-tree-reassoc`; CMake numatytai `Release` | Priimta. Atmesta: greitesnis algoritmas (didesni blokai) – keistų visas maišas ir silpnintų maišymą; `always_inline` – nestandartinis, standartinio `inline` pakako | 62 + 33 patikros, 312 įvesčių su Python realizacija, mutacijų testai; ASan / UBSan, clang++, `-O0` / `-O2` / `-march=native` – 1, 5, 6 eksperimentų rezultatai sutapo baitas į baitą; 5 GiB srautu = visa atmintyje |
| Palyginimas su standartais (09-30) | Papildoma užduotis: palyginti abi poros maišas su MD5, SHA-1, SHA-256 pagal spartą ir lavinos efektą | OpenSSL `EVP` adapteris bendrai eksperimentų programai; maišos ilgis – kompiliavimo parametras; procentai ir idealus nuokrypis pagal maišos ilgį; sparta kartojama, kai kiti procesai iškraipo matavimą (apkrova > 12 % ar sklaida > 10 %) | Priimta. Atmesta: Python `hashlib` spartai (matuotų interpretatorių, ne maišą); vienkartinis `EVP_Digest` (OpenSSL 3 kaskart ieško algoritmo) | Adapteriai sutapo su Python `hashlib` (102/102); DI ir Ratas-256 rezultatai su pakeista programa – baitas į baitą kaip anksčiau; rasta ir pataisyta: spartos cikle skaitytas 32 B maišos baitas (MD5 – tik 16 B) |
| V0.13 silpnybės (09-30) | Kokią silpnybę taisyti? Pataisyti silpnybes kode, atnaujinti schemą | Struktūrinė analizė: vienas raundas atsukamas dalis po dalies, žodžiai laisvai nustato 4 dalis – vieno bloko žodžius galima išspręsti; pataisymas – 2 raundai ir grįžtamasis ryšys | Priimta. Atmesta: tik grįžtamasis ryšys (pusės būsenos valdymas liktų, mūsų vertinimu ≈ 2^64 kelias); žodžius įterpti į visas 8 dalis (visą būseną valdyti būtų dar lengviau); mažiau tuščių žingsnių (daugiau pakeitimų) | Atakos patikrintos tikra programa: kolizija, antrasis pirmavaizdis, pirmavaizdis (64 nuliai); `tests/attack_v012.py` – V0.13 jos nebeveikia; naujos žinomos reikšmės sutampa su nepriklausoma Python realizacija (312/312); ASan / UBSan, clang++; schema patikrinta naršyklėje (21 rodyklė prijungta) |
| V0.2 peržiūra ir tvarkymas (09-30) | Pilnas auditas prieš galutinę versiją; sutvarkyti kodo komentarus; bendri leidimai V0.11–V0.13 su poros versijomis | Nepriklausomas agentas (Claude Sonnet) patikrino teiginius ir planą: rado pasenusius Ratas-256 duomenis palyginimuose, prieštaravimus bendrame README, versijų lentelės skaičius be duomenų repozitorijoje | Priimta: palyginimai pakartoti su Ratas-256 v0.2, į duomenis įrašomi abiejų realizacijų commit'ai, versijų lentelė – iš įrašytų duomenų, programa su failu matuojama atskirai. Atmesta: ChatGPT audito teiginys, kad `main` neturi V0.13 (patikrinta – klaidingas) | Komentarų pakeitimai patikrinti: kodas be komentarų nepasikeitė (6 C++ failai); 62 + 33 + 312 + 6 patikros; atkartojamumas 8/8 su Ratas-256 v0.2 |

## V0.11 rastos ir pataisytos problemos

* `struct_newline_crlf.txt` visą laiką turėjo LF, ne CRLF – testas nieko netikrino. Pataisyta, pridėta `tests/check_fixtures.py`.
* Nebuvo žinomų atsakymų testų – atsitiktinis algoritmo pakeitimas nebūtų pastebėtas. Pridėta 10 vektorių.
* Lavinos eksperimentas ignoravo hex dekodavimo klaidas ir netikrino, ar pakeistas tiksliai vienas simbolis ar bitas. Pridėtos patikros (visos 200 000 porų buvo teisingos).
* Grafikų ašys buvo įrašytos ranka – dabar skaičiuojamos iš duomenų; bendrame README spartos grafikas nebuvo įdėtas.
* `palyginimas/palyginti.sh` kiekvieną kartą perrašydavo įrašytus Windows spartos matavimus – atskirti režimai.

## V0.12 rastos ir pataisytos problemos

* Be `CMAKE_BUILD_TYPE` CMake kompiliuodavo be optimizacijų – ≈ 7 kartus lėčiau. Dabar numatytai `Release`.
* GCC pergrupuodavo XOR taip, kad dvi operacijos atsidurdavo daugybų grandinėje: 73,5 ciklo žingsniui vietoj ≈ 71.
* `--file` įkeldavo visą failą į atmintį: 2 GiB – 2 GB RAM ir 2,4 s. Dabar 64 KiB dalimis: 4 MB, 1,1 s.
* Pirmasis `Hasher` variantas trumpoms įvestims buvo iki 15 % lėtesnis (būsena ėjo per atmintį) – pataisyta prieš įrašant.

## V0.13 rastos ir pataisytos problemos

* V0.12 kolizijos, antrieji pirmavaizdžiai ir pirmavaizdžiai buvo skaičiuojami akimirksniu, be paieškos, nors visi statistiniai
  testai (0 kolizijų, 50 % lavinos efektas) buvo geri. Pataisyta: 2 raundai žingsnyje ir grįžtamasis ryšys; sparta ≈ 2 kartus mažesnė.
* Algoritmo schema neatitiko naujo žingsnio – nubraižyta iš naujo (`docs/algoritmo-schema.*`).
* Spartos matavimus kartais iškraipydavo kiti kompiuterio procesai – dabar tokie paleidimai kartojami automatiškai.

## V0.2 rastos ir pataisytos problemos

* Palyginimuose Ratas-256 buvo matuota v0.11, nors repozitorijoje jau buvo v0.12, vėliau v0.2. Pakartota; kad žymė nebeatsiliktų,
  `palyginti.sh` ir `standartai.sh` į duomenis įrašo abiejų realizacijų commit'us.
* Bendrame README – pasenęs Ratas-256 aprašymas ir teiginiai apie DI; perrašyta pagal jo README ir rezultatus.
* Versijų lentelėje buvo skaičiai iš atskiro matavimo, kurio duomenų repozitorijoje nėra; dabar – iš `results/raw/`.
* Komentarai kode buvo ilgi ir kartojo kodą – sutrumpinti iki paaiškinimų, kodėl taip daroma.
