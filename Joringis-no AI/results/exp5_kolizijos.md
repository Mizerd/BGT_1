# 5 eksperimentas: kolizijos

Kiekvienam ilgiui 100 000 atsitiktinių porų iš abėcėlės `!`..`~` (94 ASCII simboliai, 1 simbolis = 1 baitas), `std::mt19937_64`, seed = 20260920 + ilgis, simbolis = `'!' + (x mod 94)`. Jei poros narės sutampa, antroji generuojama iš naujo. Kolizija skaičiuojama tik tarp skirtingų įvesčių.

| Ilgis | Porų | Iš naujo generuota | Kolizijos porose | Skirtingų įvesčių | Kolizijų grupės rinkinyje |
|---|---|---|---|---|---|
| 10 | 100 000 | 0 | 0 | 200 000 | 0 |
| 100 | 100 000 | 0 | 0 | 200 000 | 0 |
| 500 | 100 000 | 0 | 0 | 200 000 | 0 |
| 1000 | 100 000 | 0 | 0 | 200 000 | 0 |

## Kodėl kolizijų nerandama ir ar testas jas pastebėtų

Idealiai n bitų maišai vienos poros kolizijos tikimybė ≈ 2^(−n), o m įvesčių rinkinyje yra m(m−1)/2 porų. Kai m = 200 000 ir n = 256, tikėtina ≈ 2·10^10 · 2^(−256) ≈ 10^(−67) kolizijų, todėl nulis yra įprastas ir apie saugumą nieko neįrodo. Kad matytųsi, jog testas kolizijas randa, tie patys rinkiniai patikrinti su sutrumpintomis maišomis (visi keturi ilgiai kartu):

| Sutrumpinta iki | Tikėtina | Rasta |
|---|---|---|
| 24 bitai | 4 768,35 | 4 743 |
| 32 bitai | 18,63 | 13 |
| 40 bitai | 0,07 | 0 |

## Struktūruotos įvestys

| Rinkinys | Įvesčių | Skirtingų | Kolizijų grupės |
|---|---|---|---|
| visi 1 baito | 256 | 256 | 0 |
| visi 2 baitų | 65 536 | 65 536 | 0 |
| nuliniai baitai (0–4096) | 4 097 | 4 097 | 0 |
| 'a' kartojimas (0–4096) | 4 097 | 4 097 | 0 |
| 32 B blokas ×1–512 | 512 | 512 | 0 |
| 'abcdefgh' perstatymai | 40 320 | 40 320 | 0 |
| 32 B blokų tvarka XY / YX | 2 000 | 2 000 | 0 |
| 'ab' su nuliais priekyje / gale | 2 050 | 2 049 | 0 |
| vieno bito pakeitimai 64 B įvestyje | 513 | 513 | 0 |
| visi kartu | 119 381 | 119 374 | 0 |
