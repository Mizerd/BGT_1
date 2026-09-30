# Vykdymo aplinka

| | |
|---|---|
| Data | 2026-09-30 14:24 |
| Procesorius | Intel(R) Core(TM) i9-10900K CPU @ 3.70GHz, 20 loginiai branduoliai |
| Atmintis | 62 GB |
| OS | NixOS 26.05 (Yarara), Linux 6.18.41 |
| Kompiliatorius | g++ (GCC) 15.2.0 |
| Parinktys | `-std=c++20 -O3 -DNDEBUG` (CMake Release), viena gija |
| Spartos matavimai | prisegti prie branduolio 2 (`taskset -c 2`), dažnio valdiklis `powersave` |
| Generatorius | `std::mt19937_64`, simbolis = `'!' + (x mod 94)`, bazinis seed 20260920 |
| CPU apkrova matuojant laiką | ≤ 11 % (dėl kitų procesų pakartotų paleidimų: 1) |
| Duomenys | bendras poros rinkinys `Joringis-no AI/data/` (`exp1/`, `konstitucija.txt`) |
| Duomenų SHA-256 | `c7c2c392b53ee7ca…` |
| Realizacija | commit 71a1249 |
| Eksperimentų programa | commit 2effd15 |
| Darbo medis | švarus |
