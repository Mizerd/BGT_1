# 6 eksperimentas: lavinos efektas

100 000 porų: po 25 000 ilgiams 10, 100, 500 ir 1 000. Kiekvienoje poroje vienas atsitiktinai parinktas simbolis pakeistas kitu tos pačios abėcėlės `!`..`~` simboliu, ilgis nekinta (`std::mt19937_64`, seed = 20260920 + 100 + ilgis). Prieš lyginant bitus abi hex maišos dekoduojamos į baitus. Orientyrai: ≈ 50 % bitų ir ≈ 93,75 % hex skaitmenų.

| Ilgis | Porų | Bitai: vid. % | min % | max % | st. nuokr. % | Hex: vid. % | min % | max % |
|---|---|---|---|---|---|---|---|---|
| 10 | 25 000 | 49,99 | 36,72 | 62,11 | 3,11 | 93,75 | 76,56 | 100,00 |
| 100 | 25 000 | 50,00 | 38,67 | 61,33 | 3,12 | 93,74 | 79,69 | 100,00 |
| 500 | 25 000 | 50,03 | 37,89 | 62,50 | 3,14 | 93,75 | 81,25 | 100,00 |
| 1000 | 25 000 | 49,97 | 38,67 | 64,45 | 3,12 | 93,74 | 79,69 | 100,00 |
| visi | 100 000 | 50,00 | 36,72 | 64,45 | 3,12 | 93,75 | 76,56 | 100,00 |

Idealiam atsitiktiniam atvejiui bitų skirtumo standartinis nuokrypis √(256·0,25)/256 = 3,13 %.

## Papildomai: apverstas tiksliai vienas įvesties bitas

Tie patys ilgiai ir porų skaičius, bet apverčiamas vienas bitas (baitų režimu; seed = 20260920 + 200 + ilgis): bitų skirtumas 50,01 % (35,94–64,06 %), hex skirtumas 93,74 % (78,12–100,00 %).

![Bitų skirtumo histograma](exp6_histograma.svg)

Pilka linija – binominis pasiskirstymas B(256; 0,5), kurio tikėtųsi iš idealiai atsitiktinės maišos. Histogramos duomenys: `raw/avalanche.csv` (eilutės `hist`).
