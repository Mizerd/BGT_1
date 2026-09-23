# 7 eksperimentas: spėjimas, vieša druska ir slaptas atsitiktinumas

Kandidatai – visos keturių skaitmenų eilutės `0000`–`9999` (10 000, po 4 baitus, be naujos eilutės). Tikslinė įvestis parinkta `std::mt19937_64` (seed 20260927); atakai pateikiama tik maiša ir kandidatų rinkinys.

## Be druskos: H(input)

|  | 2 versija | 1 versija | Ratas v0.1 |
|---|---|---|---|
| Tikslinė įvestis | `3983` | `3983` | `3983` |
| Bandymų iki radimo | 3 984 | 3 984 | 3 984 |
| Patikrinta iš viso (visi sutapimai) | 10 000 | 10 000 | 10 000 |
| Sutampantys kandidatai | `3983` | `3983` | `3983` |
| Viso perrinkimo laikas, ms | 0,76 | 0,18 | 0,66 |

Kandidatų rinkinyje rastas tiksliai vienas sutapimas, todėl čia jis identifikuoja įvestį. Bendru atveju sutapimas to neįrodo: kolizijos neišvengiamai egzistuoja, o tikroji įvestis gali būti ir už rinkinio ribų.

Be druskos vieną kartą apskaičiuota 10 000 maišų lentelė tinka visiems taikiniams:

|  | 2 versija | 1 versija | Ratas v0.1 |
|---|---|---|---|
| Taikinių | 5 | 5 | 5 |
| Maišos skaičiavimų | 10 000 | 10 000 | 10 000 |
| Laikas (lentelė + paieška), ms | 2,52 | 2,03 | 2,51 |
| Atspėta | 5/5 | 5/5 | 5/5 |

## Vieša druska: H(input || salt)

Kiekvienam taikiniui – atskira atsitiktinė 16 baitų druska (`std::mt19937_64`), pridedama po įvesties kaip tikslūs baitai (ne hex tekstas); lentelėje druska parodyta hex. Užpuolikas druską žino.

| # | Taikinys | Druska | 2 versija: bandymai / sutapimai / ms | 1 versija: bandymai / sutapimai / ms | Ratas v0.1: bandymai / sutapimai / ms |
|---|---|---|---|---|---|
| 1 | `9481` | `b0e0c4a2c1eea1651ebe9ce87bd61c92` | 9 482 / 1 / 0,96 | 9 482 / 1 / 0,41 | 9 482 / 1 / 1,09 |
| 2 | `8119` | `0d4f0979aa126e6c43a91b65cd891d62` | 8 120 / 1 / 1,00 | 8 120 / 1 / 0,38 | 8 120 / 1 / 1,08 |
| 3 | `0203` | `bd680432b6f02372d7621b6231c0888f` | 204 / 1 / 0,96 | 204 / 1 / 0,44 | 204 / 1 / 1,10 |
| 4 | `0580` | `c9e1e94e879b373a4e6dc27b2fd0914c` | 581 / 1 / 1,01 | 581 / 1 / 0,40 | 581 / 1 / 1,03 |
| 5 | `3980` | `ca200add4692169a0b1e2a14703bcffb` | 3 981 / 1 / 0,97 | 3 981 / 1 / 0,38 | 3 981 / 1 / 1,07 |

Su druska kiekvienam taikiniui tenka atskiras perrinkimas: 5 taikiniams – 50 000 maišos skaičiavimų vietoj 10 000. Vienam taikiniui druska darbo nepadidina (vis tiek ≤ 10 000 bandymų), bet iš anksto apskaičiuotos lentelės nebegalima panaudoti kitiems taikiniams su kita druska.

## Slaptas atsitiktinumas: H(input || r)

Įsipareigojimas `c = H("3983" || r)`, kur r – 16 slaptų baitų. Kol r nežinomas, kiekvienam kandidatui reikėtų perrinkti 2^128 r reikšmių, t. y. 10 000 · 2^128 variantų – tai neatliekama. Atskleidus r, patikrinti užtenka vieno maišos skaičiavimo:

|  | 2 versija | 1 versija | Ratas v0.1 |
|---|---|---|---|
| `H(input || r) == c` su teisinga įvestimi | taip | taip | taip |
| kita įvestis su tuo pačiu r atmetama | taip | taip | taip |

Tai iliustruoja įsipareigojimo (commitment) idėją, bet neįrodo, kad ši konstrukcija saugiai paslepia pranešimą ar neleidžia jo vėliau pakeisti. Tai ir ne darbo įrodymo (proof-of-work) galvosūkis: ten ieškoma nonce, su kuria maiša tenkina sąlygą, o čia – konkreti įvestis iš mažo rinkinio.
