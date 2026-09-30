# Vykdymo aplinka

| | |
|---|---|
| Data | 2026-09-30 13:00 |
| Procesorius | Intel(R) Core(TM) i9-10900K CPU @ 3.70GHz, 20 loginiai branduoliai |
| Atmintis | 62 GB |
| OS | NixOS 26.05 (Yarara), Linux 6.18.41 |
| Kompiliatorius | g++ (GCC) 15.2.0 |
| Parinktys | `-std=c++20 -O3 -DNDEBUG` (CMake Release), viena gija |
| Spartos matavimai | prisegti prie branduolio 2 (`taskset -c 2`), dažnio valdiklis `powersave` |
| Generatorius | `std::mt19937_64`, simbolis = `'!' + (x mod 94)`, bazinis seed 20260920 |
| CPU apkrova prieš spartos matavimą | 5 % |
| Duomenys | bendras poros rinkinys `Joringis-no AI/data/` (`exp1/`, `konstitucija.txt`) |
| Duomenų SHA-256 | `c7c2c392b53ee7ca…` |
| Realizacija | commit 137975d |
| Eksperimentų programa | commit 3b11b5e |
| Darbo medis | švarus |
