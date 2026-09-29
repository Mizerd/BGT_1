# DI sąveikos žurnalas

Įrankis: Claude Code (Anthropic), Claude Opus modeliai. Sprendimus priėmė ir rezultatus tikrino studentas.

| Etapas | Užklausa | Svarbiausi pasiūlymai | Priimta / atmesta ir kodėl | Patikra |
|---|---|---|---|---|
| Pirmasis dizainas (09-20) | Sukurti savą 256 bitų maišą C++20, peržiūrėjus žinomas maišų šeimas, kad nebūtų atkartota jų struktūra | 4 × 64 b būsena su žiedine grandine; konstantos iš `n^n` skaitmenų; likučio ilgis žymėje vietoj `0x80` | Priimta. Atmesta: xor-shift-multiply finalizatorius (per daug panašus į MurmurHash / xxHash); 19 skaitmenų gabalai konstantoms (vyresnieji bitai dažnai nuliai) – pakeisti 20 skaitmenų | Nepriklausoma Python realizacija sutapo baitas į baitą; ASan / UBSan; konstantos palygintos su žinomų maišų konstantomis |
| 512 bitų būsena (09-20) | Pašalinti silpnybę: maiša atskleisdavo visą būseną, o žingsniai apverčiami | 8 × 64 b būsena, perėjimas pirmyn ir atgal, 512 → 256 sulenkimas | Priimta. Atmesta: dvi atskiros 4 dalių pusės, kurias jungia tik keli ryšiai – pokytis per lėtai pereitų tarp pusių | Pirmojoje versijoje 10 000 / 10 000 iki 15 B žinučių atkurta iš maišos; naujoje tas kelias nebeegzistuoja; lavinos efektas ≈ 50 % |
| Eksperimentai (09-23) | Atlikti 1–8 eksperimentus su tais pačiais duomenimis kaip poros realizacija; pridėti rankinį įvedimą | Bendra eksperimentų programa su adapteriais; `std::mt19937_64`, seed 20260920; sutrumpintų maišų kolizijų patikra | Priimta. Atmesta: pirmosios versijos palyginimas README – palikta tik dabartinė realizacija | Ratas-256 rezultatai Windows (MSVC) ir Linux (g++) sutapo baitas į baitą; sutrumpintų maišų kolizijos atitinka gimtadienio įvertį |
| V0.11 patikra (09-29) | Išsamiai patikrinti realizaciją, pataisyti tikras problemas, nekeičiant algoritmo | Pataisyti CRLF testinį failą; pridėti žinomų atsakymų testus; griežtesnės lavinos patikros; duomenų kontrolinės sumos | Priimta. Atmesta: algoritmo pakeitimai – atidėta V0.12 | Maišos reikšmės nepakito (10 žinomų atsakymų, nepriklausoma Python realizacija); 57 + 32 patikros; rezultatai sugeneruoti iš naujo |

## V0.11 rastos ir pataisytos problemos

* `struct_newline_crlf.txt` visą laiką turėjo LF, ne CRLF – testas nieko netikrino. Pataisyta, pridėta `tests/check_fixtures.py`.
* Nebuvo žinomų atsakymų testų – atsitiktinis algoritmo pakeitimas nebūtų pastebėtas. Pridėta 10 vektorių.
* Lavinos eksperimentas ignoravo hex dekodavimo klaidas ir netikrino, ar pakeistas tiksliai vienas simbolis ar bitas. Pridėtos patikros (visos 200 000 porų buvo teisingos).
* Grafikų ašys buvo įrašytos ranka – dabar skaičiuojamos iš duomenų; bendrame README spartos grafikas nebuvo įdėtas.
* `palyginimas/palyginti.sh` kiekvieną kartą perrašydavo įrašytus Windows spartos matavimus – atskirti režimai.
