# 4 eksperimentas: sparta

`konstitucija.txt` ištraukos po 1, 2, 4, … eilučių su eilučių skirtukais ir visas failas. Ištrauka paruošiama iš anksto, matuojamas tik maišos skaičiavimas (be failų I/O ir išvedimo). `std::chrono::steady_clock`, 3 apšilimo ir 10 matavimų kiekvienam dydžiui; viename matavime maiša kartojama, kol praeina ≥ 20 ms, ir laikas dalijamas iš kvietimų skaičiaus. Rezultatas naudojamas (`volatile`), o maišos funkcija yra atskirame vertimo vienete, todėl kompiliatorius skaičiavimo neišmeta.

| Eilutės | Baitai | Vidurkis, µs | Min–max, µs | MB/s |
|---|---|---|---|---|
| 1 | 70 | 0,101 | 0,099–0,103 | 693 |
| 2 | 123 | 0,116 | 0,114–0,117 | 1 063 |
| 4 | 205 | 0,160 | 0,159–0,164 | 1 281 |
| 8 | 362 | 0,230 | 0,227–0,234 | 1 573 |
| 16 | 996 | 0,522 | 0,512–0,528 | 1 907 |
| 32 | 1 841 | 0,893 | 0,865–0,930 | 2 061 |
| 64 | 3 712 | 1,748 | 1,720–1,798 | 2 123 |
| 128 | 9 155 | 4,204 | 4,137–4,264 | 2 177 |
| 256 | 20 409 | 9,289 | 9,129–9,516 | 2 197 |
| 512 | 47 434 | 21,241 | 20,733–22,271 | 2 233 |
| 789 | 75 595 | 34,186 | 33,675–34,941 | 2 211 |

Neapdoroti matavimai: `raw/speed.csv`.
