# 5 eksperimentas: kolizijos

Kiekvienam ilgiui 100 000 atsitiktinių porų iš abėcėlės `!`..`~` (94 ASCII simboliai, 1 simbolis = 1 baitas), `std::mt19937_64`, seed = 20260920 + ilgis, simbolis = `'!' + (x mod 94)`. Jei poros narės sutampa, antroji generuojama iš naujo. Kolizija skaičiuojama tik tarp skirtingų įvesčių. Visos trys realizacijos maišo tas pačias įvestis.

Reikšmės langeliuose: 2 v. / 1 v. / Ratas.

| Ilgis | Porų | Skirtingų įvesčių rinkinyje | Kolizijos porose | Kolizijų grupės visame rinkinyje |
|---|---|---|---|---|
| 10 | 100 000 | 200 000 | 0 / 0 / 0 | 0 / 0 / 0 |
| 100 | 100 000 | 200 000 | 0 / 0 / 0 | 0 / 0 / 0 |
| 500 | 100 000 | 200 000 | 0 / 0 / 0 | 0 / 0 / 0 |
| 1000 | 100 000 | 200 000 | 0 / 0 / 0 | 0 / 0 / 0 |

## Kodėl kolizijų nerandama ir ar testas jas pastebėtų

Idealiai n bitų maišai vienos poros kolizijos tikimybė ≈ 2^(−n), o m įvesčių rinkinyje yra m(m−1)/2 porų. Kai m = 200 000 ir n = 256, tikėtinas kolizijų skaičius ≈ 2·10^10 · 2^(−256) ≈ 10^(−67), todėl nulis yra įprastas rezultatas ir apie saugumą nieko neįrodo. Kad matytųsi, jog pats testas kolizijas randa, tie patys rinkiniai patikrinti su iki 24, 32 ir 40 bitų sutrumpintomis maišomis (sumuota per visus keturis ilgius):

| Sutrumpinta iki | Tikėtina | 2 versija | 1 versija | Ratas v0.1 |
|---|---|---|---|---|
| 24 bitai | 4 768,35 | 4 654 | 4 807 | 4 743 |
| 32 bitai | 18,63 | 19 | 17 | 13 |
| 40 bitai | 0,07 | 0 | 0 | 0 |

Sutrumpintų maišų kolizijų skaičius atitinka gimtadienio paradokso įvertį, t. y. pagal šį matą maišos elgiasi kaip atsitiktinės.

## Struktūruotos įvestys

Kolizijų grupės langeliuose: 2 v. / 1 v. / Ratas.

| Rinkinys | Įvesčių | Skirtingų | Kolizijų grupės |
|---|---|---|---|
| visi 1 baito | 256 | 256 | 0 / 0 / 0 |
| visi 2 baitų | 65 536 | 65 536 | 0 / 0 / 0 |
| nuliniai baitai (0–4096) | 4 097 | 4 097 | 0 / 0 / 0 |
| 'a' kartojimas (0–4096) | 4 097 | 4 097 | 0 / 0 / 0 |
| 32 B blokas ×1–512 | 512 | 512 | 0 / 0 / 0 |
| 'abcdefgh' perstatymai | 40 320 | 40 320 | 0 / 0 / 0 |
| 32 B blokų tvarka XY / YX | 2 000 | 2 000 | 0 / 0 / 0 |
| 'ab' su nuliais priekyje / gale | 2 050 | 2 049 | 0 / 0 / 0 |
| vieno bito pakeitimai 64 B įvestyje | 513 | 513 | 0 / 0 / 0 |
| visi kartu | 119 381 | 119 374 | 0 / 0 / 0 |
