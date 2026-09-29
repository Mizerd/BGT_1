# Vykdymo aplinka

| | |
|---|---|
| Data | 2026-09-23 14:11 |
| Procesorius | Intel(R) Core(TM) i9-10900K CPU @ 3.70GHz, 20 loginiai branduoliai |
| Atmintis | 62 GB |
| OS | NixOS 26.05 (Yarara), Linux 6.18.41 |
| Kompiliatorius | g++ (GCC) 15.2.0 |
| Parinktys | `-std=c++20 -O3 -DNDEBUG` (CMake Release), viena gija |
| Spartos matavimai | prisegti prie branduolio 2 (`taskset -c 2`), dažnio valdiklis `powersave` |
| Generatorius | `std::mt19937_64`, simbolis = `'!' + (x mod 94)`, bazinis seed 20260920 |
| Duomenys | bendras poros rinkinys `Joringis-no AI/data/` (`exp1/`, `konstitucija.txt`) |
| Realizacija | commit d0c1201 |

**V0.11 (2026-09-29):** po CRLF testinio failo pataisymo iš naujo sugeneruoti tik `inputs.csv` ir `cli.csv` bei ataskaitos.
Kiti neapdoroti duomenys atkuriami baitas į baitą, todėl nekeisti. Sparta ir spėjimo laikai – 2026-09-23 matavimai
(maišos ir matavimo kodas nuo tada nepakito); V0.11 metu kompiuteris buvo apkrautas, todėl sparta nematuota iš naujo.
Duomenų SHA-256 (V0.11, kaip `run_all.sh`): `c7c2c392b53ee7ca…`
