# 6 eksperimentas: lavinos efektas

100 000 porų: po 25 000 ilgiams 10, 100, 500 ir 1 000. Kiekvienoje poroje vienas atsitiktinai parinktas simbolis pakeistas kitu tos pačios abėcėlės `!`..`~` simboliu, ilgis nekinta (`std::mt19937_64`, seed = 20260920 + 100 + ilgis). Prieš lyginant bitus abi hex maišos dekoduojamos į baitus. Orientyrai: ≈ 50 % bitų ir ≈ 93,75 % hex skaitmenų.

## Bitų skirtumas, %: vidurkis (min–max)

| Ilgis | 2 versija | 1 versija | Ratas v0.1 |
|---|---|---|---|
| 10 | 49,99 (36,72–62,11) | 50,02 (37,89–62,50) | 50,02 (39,06–62,50) |
| 100 | 50,00 (38,67–61,33) | 49,99 (37,11–62,50) | 50,00 (36,33–61,72) |
| 500 | 50,03 (37,89–62,50) | 50,00 (36,72–62,11) | 50,02 (37,50–62,50) |
| 1000 | 49,97 (38,67–64,45) | 50,03 (36,72–62,89) | 49,99 (38,28–63,28) |
| visi | 50,00 (36,72–64,45) | 50,01 (36,72–62,89) | 50,01 (36,33–63,28) |

## Hex skirtumas, %: vidurkis (min–max)

| Ilgis | 2 versija | 1 versija | Ratas v0.1 |
|---|---|---|---|
| 10 | 93,75 (76,56–100,00) | 93,77 (78,12–100,00) | 93,74 (79,69–100,00) |
| 100 | 93,74 (79,69–100,00) | 93,74 (78,12–100,00) | 93,72 (78,12–100,00) |
| 500 | 93,75 (81,25–100,00) | 93,75 (76,56–100,00) | 93,78 (79,69–100,00) |
| 1000 | 93,74 (79,69–100,00) | 93,78 (79,69–100,00) | 93,76 (78,12–100,00) |
| visi | 93,75 (76,56–100,00) | 93,76 (76,56–100,00) | 93,75 (78,12–100,00) |

Bitų skirtumo standartinis nuokrypis (visos poros): 2 versija – 3,12 %, 1 versija – 3,13 %, Ratas v0.1 – 3,12 %. Idealiam atsitiktiniam atvejiui – √(256·0,25)/256 = 3,13 %.

## Papildomai: apverstas tiksliai vienas įvesties bitas

Tie patys ilgiai ir porų skaičius, bet pakeičiamas vienas bitas (baitų režimu, nebūtinai ASCII; seed = 20260920 + 200 + ilgis).

| Visos 100 000 porų | 2 versija | 1 versija | Ratas v0.1 |
|---|---|---|---|
| bitų skirtumas, % | 50,01 (35,94–64,06) | 49,99 (36,33–64,06) | 49,99 (37,50–63,28) |
| hex skirtumas, % | 93,74 (78,12–100,00) | 93,75 (78,12–100,00) | 93,73 (78,12–100,00) |

![Bitų skirtumo histograma](exp6_histograma.svg)

Pilka linija – binominis pasiskirstymas B(256; 0,5), kurio tikėtųsi iš idealiai atsitiktinės maišos. Histogramos duomenys: `raw/avalanche.csv` (eilutės `hist`).
