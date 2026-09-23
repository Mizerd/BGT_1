# Vykdymo aplinka

| | |
|---|---|
| Data | 2026-09-23 |
| Procesorius | AMD Ryzen 9 7900X 12-Core, 24 loginiai branduoliai |
| Atmintis | 31 GB |
| OS | Windows 11 Pro 10.0.26200 |
| Kompiliatorius | MSVC 19.44.35221 (Visual Studio 2022), x64 |
| Parinktys | `/std:c++20 /O2 /EHsc /DNDEBUG`, viena gija |
| Spartos matavimai | be branduolio prisegimo (`taskset` Windows'e nėra) |
| Generatorius | `std::mt19937_64`, simbolis = `'!' + (x mod 94)`, bazinis seed 20260920 |
| Duomenys | bendras poros rinkinys `Joringis-no AI/data/` (`exp1/`, `konstitucija.txt`) |
| Realizacija | commit c6622a9 (`ratas.cpp`, v0.1) |
| Eksperimentų programa | `Rokas - AI/experiments/experiments.cpp`, nepakeista; adapteris `experiments/impl_ratas.cpp` |

## Atkūrimas

```bat
:: „x64 Native Tools Command Prompt for VS 2022“, iš „Joringis-no AI“ katalogo
cl /nologo /std:c++20 /O2 /EHsc /utf-8 /DNDEBUG /I"experiments" /I"..\Rokas - AI\experiments" ^
   /Fo:build\ experiments\impl_ratas.cpp "..\Rokas - AI\experiments\experiments.cpp" /Fe:build\experiments.exe

build\experiments.exe inputs data\exp1        > results\raw\inputs.csv
build\experiments.exe speed data\konstitucija.txt > results\raw\speed.csv
build\experiments.exe collisions              > results\raw\collisions.csv
build\experiments.exe structured              > results\raw\structured.csv
build\experiments.exe avalanche               > results\raw\avalanche.csv
build\experiments.exe guess                   > results\raw\guess.csv

python -X utf8 "..\Rokas - AI\experiments\report.py" results\raw results
```

`results/raw/cli.csv` sugeneruojamas atskirai – jame tikrinama, ar abu komandinės
eilutės režimai (argumentas ir meniu) duoda tą pačią maišą kaip ir vidinis
skaičiavimas.

## Svarbu dėl duomenų

`.gitattributes` eilutė `data/** -text` yra būtina: be jos Windows'e su
`core.autocrlf=true` `konstitucija.txt` virsta 76 384 baitų failu (vietoj 75 595),
o `struct_newline_lf.txt` – 9 baitų (vietoj 8), ir rezultatai nebesutampa su
poros partnerio matavimais.

## Palyginimo su partnerio rezultatais apribojimas

Spartos skaičiai **nėra** tiesiogiai palyginami: partnerio matavimai atlikti
kitame kompiuteryje (Intel i9-10900K), kita OS (NixOS) ir kitu kompiliatoriumi
(g++ 15.2 `-O3`). Teisingumo, kolizijų, lavinos ir spėjimo eksperimentai
nepriklauso nuo mašinos – juose naudojami tie patys seed'ai ir tie patys
duomenys, todėl tie rezultatai palyginami.
