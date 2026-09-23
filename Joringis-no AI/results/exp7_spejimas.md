# 7 eksperimentas: spėjimas, vieša druska ir slaptas atsitiktinumas

Kandidatai – visos keturių skaitmenų eilutės `0000`–`9999` (10 000, po 4 baitus, be naujos eilutės). Tikslinė įvestis parinkta `std::mt19937_64` (seed 20260927); atakai pateikiama tik maiša ir kandidatų rinkinys.

## Be druskos: H(input)

|  | Rezultatas |
|---|---|
| Tikslinė įvestis | `3983` |
| Bandymų iki radimo | 3 984 |
| Patikrinta iš viso (visi sutapimai) | 10 000 |
| Sutampantys kandidatai | `3983` |
| Viso perrinkimo laikas | 1,00 ms |
| Viena iš anksto apskaičiuota lentelė 5 taikiniams | 10 000 maišų, 2,47 ms, atspėta 5/5 |

Kandidatų rinkinyje rastas tiksliai vienas sutapimas, todėl čia jis identifikuoja įvestį. Bendru atveju sutapimas to neįrodo: kolizijos neišvengiamai egzistuoja, o tikroji įvestis gali būti ir už rinkinio ribų.

## Vieša druska: H(input || salt)

Kiekvienam taikiniui – atskira atsitiktinė 16 baitų druska, pridedama po įvesties kaip tikslūs baitai (ne hex tekstas); lentelėje parodyta hex. Užpuolikas druską žino.

| # | Taikinys | Druska | Bandymų | Sutapimų | Laikas, ms |
|---|---|---|---|---|---|
| 1 | `9481` | `b0e0c4a2c1eea1651ebe9ce87bd61c92` | 9 482 | 1 | 1,71 |
| 2 | `8119` | `0d4f0979aa126e6c43a91b65cd891d62` | 8 120 | 1 | 1,69 |
| 3 | `0203` | `bd680432b6f02372d7621b6231c0888f` | 204 | 1 | 1,68 |
| 4 | `0580` | `c9e1e94e879b373a4e6dc27b2fd0914c` | 581 | 1 | 1,64 |
| 5 | `3980` | `ca200add4692169a0b1e2a14703bcffb` | 3 981 | 1 | 1,65 |

Su druska kiekvienam taikiniui reikia atskiro perrinkimo: 5 taikiniams – 50 000 maišų vietoj 10 000. Vienam taikiniui druska darbo nepadidina, bet iš anksto apskaičiuotos lentelės nebegalima panaudoti kitiems taikiniams.

## Slaptas atsitiktinumas: H(input || r)

Įsipareigojimas `c = H("3983" || r)`, kur r – 16 slaptų baitų. Kol r nežinomas, paieškos erdvė – 10 000 · 2^128 variantų, todėl perrinkimas neatliekamas. Atskleidus r, patikrai užtenka vienos maišos: teisinga įvestis patvirtinta – taip, kita įvestis atmesta – taip.

Tai iliustruoja įsipareigojimo (commitment) idėją, bet neįrodo, kad konstrukcija saugiai paslepia pranešimą ar neleidžia jo pakeisti. Tai ir ne darbo įrodymo (proof-of-work) galvosūkis, kuriame ieškoma sąlygą tenkinančios nonce.
