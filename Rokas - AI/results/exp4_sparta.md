# 4 eksperimentas: sparta

`konstitucija.txt` ištraukos po 1, 2, 4, … eilučių su eilučių skirtukais ir visas failas. Ištrauka paruošiama iš anksto, matuojamas tik maišos skaičiavimas (be failų I/O ir išvedimo). `std::chrono::steady_clock`, 3 apšilimo ir 10 matavimų kiekvienam dydžiui; viename matavime maiša kartojama, kol praeina ≥ 20 ms, ir laikas dalijamas iš kvietimų skaičiaus. Rezultatas naudojamas (`volatile`), o maišos funkcija yra atskirame vertimo vienete, todėl kompiliatorius skaičiavimo neišmeta.

| Eilutės | Baitai | Vidurkis, µs | Min–max, µs | MB/s |
|---|---|---|---|---|
| 1 | 70 | 0,235 | 0,235–0,236 | 297 |
| 2 | 123 | 0,267 | 0,266–0,275 | 459 |
| 4 | 205 | 0,353 | 0,352–0,354 | 580 |
| 8 | 362 | 0,515 | 0,514–0,517 | 702 |
| 16 | 996 | 1,121 | 1,120–1,123 | 888 |
| 32 | 1 841 | 1,914 | 1,911–1,918 | 962 |
| 64 | 3 712 | 3,701 | 3,693–3,707 | 1 002 |
| 128 | 9 155 | 8,893 | 8,881–8,913 | 1 029 |
| 256 | 20 409 | 19,695 | 19,651–19,744 | 1 036 |
| 512 | 47 434 | 45,293 | 45,153–45,439 | 1 047 |
| 789 | 75 595 | 71,995 | 71,899–72,078 | 1 050 |

Neapdoroti matavimai: `raw/speed.csv`.
