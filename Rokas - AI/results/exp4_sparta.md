# 4 eksperimentas: sparta

`konstitucija.txt` ištraukos po 1, 2, 4, … eilučių su eilučių skirtukais ir visas failas. Ištrauka paruošiama iš anksto, matuojamas tik maišos skaičiavimas (be failų I/O ir išvedimo). `std::chrono::steady_clock`, 3 apšilimo ir 10 matavimų kiekvienam dydžiui; viename matavime maiša kartojama, kol praeina ≥ 20 ms, ir laikas dalijamas iš kvietimų skaičiaus. Rezultatas naudojamas (`volatile`), o maišos funkcija yra atskirame vertimo vienete, todėl kompiliatorius skaičiavimo neišmeta.

| Eilutės | Baitai | Vidurkis, µs | Min–max, µs | MB/s |
|---|---|---|---|---|
| 1 | 70 | 0,100 | 0,098–0,101 | 700 |
| 2 | 123 | 0,115 | 0,115–0,115 | 1 068 |
| 4 | 205 | 0,158 | 0,157–0,160 | 1 295 |
| 8 | 362 | 0,233 | 0,230–0,236 | 1 555 |
| 16 | 996 | 0,532 | 0,529–0,533 | 1 870 |
| 32 | 1 841 | 0,920 | 0,909–0,922 | 2 001 |
| 64 | 3 712 | 1,790 | 1,770–1,802 | 2 073 |
| 128 | 9 155 | 4,314 | 4,266–4,343 | 2 121 |
| 256 | 20 409 | 9,571 | 9,491–9,584 | 2 132 |
| 512 | 47 434 | 22,191 | 21,865–22,615 | 2 137 |
| 789 | 75 595 | 35,191 | 34,864–35,362 | 2 148 |

Neapdoroti matavimai: `raw/speed.csv`.
