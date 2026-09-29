# Palyginimas tomis pačiomis sąlygomis

Abi realizacijos kompiliuojamos su ta pačia eksperimentų programa (`Rokas - AI/experiments/experiments.cpp`);
skiriasi tik adapteris `experiments/impl_*.cpp`.

| Failas | Kas tai |
|---|---|
| `raw/speed.csv` | spartos matavimai, abi realizacijos tame pačiame kompiuteryje |
| `sparta.md`, `sparta.svg` | lentelė ir grafikas, sugeneruoti `palyginti.py` iš `raw/speed.csv` |
| `raw/atkartojamumas.csv` | ar 1–3, 5, 6 eksperimentų išvestis sutampa su įrašyta `results/raw/` (1 – taip) |

Įrašyti spartos matavimai: AMD Ryzen 9 7900X, Windows 11, MSVC 19.44, `/std:c++20 /O2 /DNDEBUG`, viena gija.

## Linux

```bash
./palyginimas/palyginti.sh atkartojamumas   # nekeičia spartos duomenų
./palyginimas/palyginti.sh sparta           # tik neapkrautame kompiuteryje
```

## Windows („x64 Native Tools Command Prompt for VS 2022“, iš repozitorijos šaknies)

```bat
mkdir palyginimas\build palyginimas\raw
cl /nologo /std:c++20 /O2 /EHsc /utf-8 /DNDEBUG /I"Rokas - AI\experiments" /Fo:palyginimas\build\ ^
   "Joringis-no AI\experiments\impl_ratas.cpp" "Rokas - AI\experiments\experiments.cpp" /Fe:palyginimas\build\bedi.exe
cl /nologo /std:c++20 /O2 /EHsc /utf-8 /DNDEBUG /I"Rokas - AI\experiments" /I"Rokas - AI\include" /Fo:palyginimas\build\ ^
   "Rokas - AI\experiments\impl_eduhash.cpp" "Rokas - AI\src\custom_hash.cpp" "Rokas - AI\experiments\experiments.cpp" ^
   /Fe:palyginimas\build\di.exe
palyginimas\build\bedi.exe speed "Joringis-no AI\data\konstitucija.txt" >  palyginimas\raw\speed.csv
palyginimas\build\di.exe   speed "Joringis-no AI\data\konstitucija.txt" >> palyginimas\raw\speed.csv
python -X utf8 palyginimas\palyginti.py palyginimas\raw palyginimas
```
