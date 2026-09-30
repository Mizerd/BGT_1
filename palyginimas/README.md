# Palyginimas tomis pačiomis sąlygomis

Abi realizacijos kompiliuojamos su ta pačia eksperimentų programa (`Rokas - AI/experiments/experiments.cpp`);
skiriasi tik adapteris `experiments/impl_*.cpp`.

| Failas | Kas tai |
|---|---|
| `raw/speed.csv` | spartos matavimai Windows, abi realizacijos tame pačiame kompiuteryje |
| `raw/speed_linux.csv`, `.txt` | tas pats Linux ir aplinkos aprašas (`palyginti.sh sparta`) |
| `sparta.md`, `sparta.svg` | lentelės (Windows ir Linux) ir grafikas (Windows), sugeneruoti `palyginti.py` |
| `raw/atkartojamumas.csv` | ar 1–3, 5, 6 eksperimentų išvestis sutampa su įrašyta `results/raw/` (1 – taip) |
| `standartai.sh`, `standartai.py`, `impl_openssl.cpp` | papildoma užduotis: abi maišos ir MD5, SHA-1, SHA-256 (OpenSSL) |
| `raw/std_*.csv`, `standartai.md`, `standartai.svg` | jos duomenys, lentelės ir spartos grafikas |

Windows: AMD Ryzen 9 7900X, Windows 11, MSVC 19.44, `/std:c++20 /O2 /DNDEBUG`, viena gija, V0.1 kodas.
Linux: Intel i9-10900K, g++ 15.2, `-O3` (DI maišai ir `-fno-tree-reassoc`, kaip CMake), viena gija; DI maiša V0.13, Ratas-256 v0.11.

## Linux

```bash
./palyginimas/palyginti.sh atkartojamumas   # nekeičia spartos duomenų
./palyginimas/palyginti.sh sparta           # perrašo raw/speed_linux.csv; tik neapkrautame kompiuteryje
nix-shell -p openssl pkg-config --run ./palyginimas/standartai.sh   # reikia OpenSSL 3; be NixOS – tiesiog ./palyginimas/standartai.sh
```

## Windows („x64 Native Tools Command Prompt for VS 2022“, iš repozitorijos šaknies)

```bat
mkdir palyginimas\build palyginimas\raw
cl /nologo /std:c++20 /O2 /EHsc /utf-8 /DNDEBUG /I"Rokas - AI\experiments" /Fo:palyginimas\build\ ^
   "Joringis-no AI\experiments\impl_ratas.cpp" "Rokas - AI\experiments\experiments.cpp" /Fe:palyginimas\build\bedi.exe
cl /nologo /std:c++20 /O2 /EHsc /utf-8 /DNDEBUG /I"Rokas - AI\experiments" /I"Rokas - AI\include" /Fo:palyginimas\build\ ^
   "Rokas - AI\experiments\impl_dihash.cpp" "Rokas - AI\src\custom_hash.cpp" "Rokas - AI\experiments\experiments.cpp" ^
   /Fe:palyginimas\build\di.exe
palyginimas\build\bedi.exe speed "Joringis-no AI\data\konstitucija.txt" >  palyginimas\raw\speed.csv
palyginimas\build\di.exe   speed "Joringis-no AI\data\konstitucija.txt" >> palyginimas\raw\speed.csv
python -X utf8 palyginimas\palyginti.py palyginimas\raw palyginimas
```
