# Difuzija per pasukimus (papildomas matavimas)

Atsitiktinė 256 bitų būsena (seed 20260926), vienas bito skirtumas bloko įvedimo vietoje
(stipinai 0..3, visos 128 pozicijos po lygiai), abu variantai pasukami t kartų. Skaičiuojama, kiek iš 256
būsenos bitų skiriasi (2000 imtys kiekvienam t). Tai vidinės būsenos, ne santraukos, matavimas.

| Pasukimų t | Skirt. bitų min | vid. | max | vid. % |
|---:|---:|---:|---:|---:|
| 1 | 33 | 96.9 | 142 | 37.83 |
| 2 | 98 | 128.0 | 154 | 50.00 |
| 3 | 100 | 128.0 | 157 | 49.99 |
| 4 | 99 | 127.8 | 160 | 49.94 |
