# 4 eksperimentas: sparta

`konstitucija.txt` ištraukos po 1, 2, 4, … eilučių su eilučių skirtukais ir visas failas. Ištrauka paruošiama iš anksto, matuojamas tik maišos skaičiavimas (be failų I/O ir išvedimo). `std::chrono::steady_clock`, 3 apšilimo ir 10 matavimų kiekvienam dydžiui; viename matavime maiša kartojama, kol praeina ≥ 20 ms, ir laikas dalijamas iš kvietimų skaičiaus. Rezultatas naudojamas (`volatile`), o maišos funkcija yra atskirame vertimo vienete, todėl kompiliatorius skaičiavimo neišmeta.

| Eilutės | Baitai | Vidurkis, µs | Min–max, µs | MB/s |
|---|---|---|---|---|
| 1 | 70 | 0,224 | 0,224–0,225 | 311 |
| 2 | 123 | 0,306 | 0,306–0,307 | 401 |
| 4 | 205 | 0,443 | 0,443–0,444 | 462 |
| 8 | 362 | 0,721 | 0,718–0,732 | 502 |
| 16 | 996 | 1,904 | 1,809–2,443 | 523 |
| 32 | 1 841 | 3,271 | 3,249–3,363 | 562 |
| 64 | 3 712 | 6,431 | 6,419–6,441 | 577 |
| 128 | 9 155 | 16,065 | 15,769–16,464 | 569 |
| 256 | 20 409 | 35,483 | 35,164–36,634 | 575 |
| 512 | 47 434 | 84,312 | 81,929–88,488 | 562 |
| 789 | 75 595 | 135,141 | 131,599–144,288 | 559 |

Neapdoroti matavimai: `raw/speed.csv`.
