# 4 eksperimentas: sparta

`konstitucija.txt` ištraukos po 1, 2, 4, … eilučių su eilučių skirtukais ir visas failas. Ištrauka paruošiama iš anksto, matuojamas tik maišos skaičiavimas (be failų I/O ir išvedimo). `std::chrono::steady_clock`, 3 apšilimo ir 10 matavimų kiekvienam dydžiui; viename matavime maiša kartojama, kol praeina ≥ 20 ms, ir laikas dalijamas iš kvietimų skaičiaus. Rezultatas naudojamas (`volatile`), o maišos funkcija yra atskirame vertimo vienete, todėl kompiliatorius skaičiavimo neišmeta.

| Eilutės | Baitai | Vidurkis, µs | Min–max, µs | MB/s |
|---|---|---|---|---|
| 1 | 70 | 0,189 | 0,189–0,191 | 369 |
| 2 | 123 | 0,251 | 0,248–0,253 | 490 |
| 4 | 205 | 0,356 | 0,349–0,361 | 575 |
| 8 | 362 | 0,561 | 0,557–0,566 | 645 |
| 16 | 996 | 1,378 | 1,356–1,435 | 722 |
| 32 | 1 841 | 2,454 | 2,430–2,478 | 750 |
| 64 | 3 712 | 4,992 | 4,818–5,913 | 743 |
| 128 | 9 155 | 11,982 | 11,843–12,266 | 764 |
| 256 | 20 409 | 26,468 | 26,257–26,665 | 771 |
| 512 | 47 434 | 62,354 | 61,789–63,261 | 760 |
| 789 | 75 595 | 98,699 | 96,294–100,300 | 765 |

Neapdoroti matavimai: `raw/speed.csv`.
