# 4 eksperimentas: sparta

`konstitucija.txt` ištraukos po 1, 2, 4, … eilučių su eilučių skirtukais ir visas failas. Ištrauka paruošiama iš anksto, matuojamas tik maišos skaičiavimas (be failų I/O ir išvedimo). `std::chrono::steady_clock`, 3 apšilimo ir 10 matavimų kiekvienam dydžiui; viename matavime maiša kartojama, kol praeina ≥ 20 ms, ir laikas dalijamas iš kvietimų skaičiaus. Rezultatas naudojamas (`volatile`), o maišos funkcija yra atskirame vertimo vienete, todėl kompiliatorius skaičiavimo neišmeta.

| Eilutės | Baitai | Vidurkis, µs | Min–max, µs | MB/s |
|---|---|---|---|---|
| 1 | 70 | 0,152 | 0,152–0,153 | 459 |
| 2 | 123 | 0,200 | 0,199–0,201 | 614 |
| 4 | 205 | 0,283 | 0,276–0,287 | 725 |
| 8 | 362 | 0,425 | 0,424–0,426 | 851 |
| 16 | 996 | 1,024 | 1,021–1,028 | 972 |
| 32 | 1 841 | 1,815 | 1,810–1,820 | 1 014 |
| 64 | 3 712 | 3,790 | 3,760–3,813 | 979 |
| 128 | 9 155 | 9,328 | 9,143–9,670 | 981 |
| 256 | 20 409 | 20,237 | 20,000–20,457 | 1 008 |
| 512 | 47 434 | 47,176 | 46,681–48,163 | 1 005 |
| 789 | 75 595 | 74,720 | 73,937–76,126 | 1 011 |

Neapdoroti matavimai: `raw/speed.csv`.
