# 4 eksperimentas: sparta

`konstitucija.txt` ištraukos po 1, 2, 4, … eilučių su eilučių skirtukais ir visas failas. Ištrauka paruošiama iš anksto, matuojamas tik maišos skaičiavimas (be failų I/O ir išvedimo). `std::chrono::steady_clock`, 3 apšilimo ir 10 matavimų kiekvienam dydžiui; viename matavime maiša kartojama, kol praeina ≥ 20 ms, ir laikas dalijamas iš kvietimų skaičiaus. Rezultatas naudojamas (`volatile`), o maišos funkcija yra atskirame vertimo vienete, todėl kompiliatorius skaičiavimo neišmeta.

| Eilutės | Baitai | Vidurkis, µs | Min–max, µs | MB/s |
|---|---|---|---|---|
| 1 | 70 | 0,163 | 0,163–0,169 | 428 |
| 2 | 123 | 0,214 | 0,213–0,214 | 575 |
| 4 | 205 | 0,296 | 0,293–0,300 | 692 |
| 8 | 362 | 0,459 | 0,458–0,461 | 788 |
| 16 | 996 | 1,115 | 1,113–1,118 | 893 |
| 32 | 1 841 | 1,985 | 1,979–1,994 | 927 |
| 64 | 3 712 | 3,914 | 3,902–3,925 | 948 |
| 128 | 9 155 | 9,499 | 9,475–9,547 | 963 |
| 256 | 20 409 | 21,038 | 20,982–21,223 | 970 |
| 512 | 47 434 | 48,883 | 48,823–49,130 | 970 |
| 789 | 75 595 | 77,953 | 77,606–79,414 | 969 |

Neapdoroti matavimai: `raw/speed.csv`.
