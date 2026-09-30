# 4 eksperimentas: sparta

`konstitucija.txt` ištraukos po 1, 2, 4, … eilučių su eilučių skirtukais ir visas failas. Ištrauka paruošiama iš anksto, matuojamas tik maišos skaičiavimas (be failų I/O ir išvedimo). `std::chrono::steady_clock`, 3 apšilimo ir 10 matavimų kiekvienam dydžiui; viename matavime maiša kartojama, kol praeina ≥ 20 ms, ir laikas dalijamas iš kvietimų skaičiaus. Rezultatas naudojamas (`volatile`), o maišos funkcija yra atskirame vertimo vienete, todėl kompiliatorius skaičiavimo neišmeta.

| Eilutės | Baitai | Vidurkis, µs | Min–max, µs | MB/s |
|---|---|---|---|---|
| 1 | 70 | 0,234 | 0,234–0,235 | 298 |
| 2 | 123 | 0,266 | 0,266–0,267 | 461 |
| 4 | 205 | 0,353 | 0,353–0,354 | 580 |
| 8 | 362 | 0,513 | 0,512–0,514 | 705 |
| 16 | 996 | 1,116 | 1,115–1,117 | 892 |
| 32 | 1 841 | 1,907 | 1,904–1,908 | 965 |
| 64 | 3 712 | 3,685 | 3,680–3,690 | 1 007 |
| 128 | 9 155 | 8,860 | 8,851–8,866 | 1 033 |
| 256 | 20 409 | 19,613 | 19,526–19,655 | 1 040 |
| 512 | 47 434 | 45,081 | 44,936–45,183 | 1 052 |
| 789 | 75 595 | 71,780 | 71,704–71,905 | 1 053 |

Neapdoroti matavimai: `raw/speed.csv`.

## Visa programa su I/O (matuojama atskirai)

`hash-generator --file`, 1 GiB per kanalą (`/dev/stdin`): 1,11 s (965 MB/s), didžiausia atmintis 3,7 MB. Failas skaitomas 64 KiB dalimis, todėl atmintis nuo failo dydžio nepriklauso. Neapdoroti duomenys: `raw/file.csv`.
