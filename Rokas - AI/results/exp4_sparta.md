# 4 eksperimentas: sparta

`konstitucija.txt` ištraukos po 1, 2, 4, … eilučių su eilučių skirtukais ir visas failas. Ištrauka paruošiama iš anksto, matuojamas tik maišos skaičiavimas (be failų I/O ir išvedimo). `std::chrono::steady_clock`, 3 apšilimo ir 10 matavimų kiekvienam dydžiui; viename matavime maiša kartojama, kol praeina ≥ 20 ms, ir laikas dalijamas iš kvietimų skaičiaus. Rezultatas naudojamas (`volatile`), o maišos funkcija yra atskirame vertimo vienete, todėl kompiliatorius skaičiavimo neišmeta.

| Eilutės | Baitai | Vidurkis, µs | Min–max, µs | MB/s |
|---|---|---|---|---|
| 1 | 70 | 0,236 | 0,234–0,244 | 297 |
| 2 | 123 | 0,266 | 0,265–0,268 | 462 |
| 4 | 205 | 0,353 | 0,352–0,354 | 580 |
| 8 | 362 | 0,513 | 0,511–0,520 | 706 |
| 16 | 996 | 1,116 | 1,114–1,120 | 892 |
| 32 | 1 841 | 1,924 | 1,905–1,995 | 956 |
| 64 | 3 712 | 3,721 | 3,680–3,850 | 997 |
| 128 | 9 155 | 8,882 | 8,854–8,923 | 1 030 |
| 256 | 20 409 | 19,708 | 19,639–19,816 | 1 035 |
| 512 | 47 434 | 45,387 | 45,131–45,853 | 1 045 |
| 789 | 75 595 | 72,014 | 71,732–72,383 | 1 049 |

Neapdoroti matavimai: `raw/speed.csv`.

## Visa programa su I/O (matuojama atskirai)

`hash-generator --file`, 1 GiB per kanalą (`/dev/stdin`): 1,13 s (948 MB/s), didžiausia atmintis 3,7 MB. Failas skaitomas 64 KiB dalimis, todėl atmintis nuo failo dydžio nepriklauso. Neapdoroti duomenys: `raw/file.csv`.
