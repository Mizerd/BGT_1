# 4 eksperimentas: sparta

`konstitucija.txt` ištraukos po 1, 2, 4, … eilučių su eilučių skirtukais ir visas failas. Ištrauka paruošiama iš anksto, matuojamas tik maišos skaičiavimas (be failų I/O ir išvedimo). `std::chrono::steady_clock`, 3 apšilimo ir 10 matavimų kiekvienam dydžiui; viename matavime maiša kartojama, kol praeina ≥ 20 ms, ir laikas dalijamas iš kvietimų skaičiaus. Rezultatas naudojamas (`volatile`), o maišos funkcija yra atskirame vertimo vienete, todėl kompiliatorius skaičiavimo neišmeta.

| Eilutės | Baitai | Vidurkis, µs | Min–max, µs | MB/s |
|---|---|---|---|---|
| 1 | 70 | 0,235 | 0,235–0,236 | 297 |
| 2 | 123 | 0,266 | 0,265–0,268 | 461 |
| 4 | 205 | 0,354 | 0,353–0,355 | 578 |
| 8 | 362 | 0,513 | 0,512–0,513 | 706 |
| 16 | 996 | 1,116 | 1,115–1,118 | 892 |
| 32 | 1 841 | 1,909 | 1,908–1,910 | 964 |
| 64 | 3 712 | 3,694 | 3,691–3,699 | 1 004 |
| 128 | 9 155 | 8,894 | 8,880–8,904 | 1 029 |
| 256 | 20 409 | 19,669 | 19,628–19,697 | 1 037 |
| 512 | 47 434 | 45,275 | 45,176–45,362 | 1 047 |
| 789 | 75 595 | 72,098 | 71,847–72,254 | 1 048 |

Neapdoroti matavimai: `raw/speed.csv`.

## Visa programa su I/O (matuojama atskirai)

`hash-generator --file`, 1 GiB per kanalą (`/dev/stdin`): 1,13 s (951 MB/s), didžiausia atmintis 3,7 MB. Failas skaitomas 64 KiB dalimis, todėl atmintis nuo failo dydžio nepriklauso. Neapdoroti duomenys: `raw/file.csv`.
