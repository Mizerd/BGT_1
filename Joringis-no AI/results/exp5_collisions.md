# 5 eksperimentas. Kolizijų paieška

Kiekvienam ilgiui sugeneruota 100000 porų (200000 eilučių) iš abėcėlės `!`..`~`
(94 ASCII simboliai, 1 simbolis = 1 baitas), seed = 20260920 (generatorius xorshift64*, kiekvienam ilgiui
seed + ilgis). Užtikrinta, kad poros narės skiriasi. Tos pačios įvestys naudojamos visoms funkcijoms.

| Versija | Ilgis | Porų | Kolizijų porose | Skirtingų įvesčių | Grupių su bendra santrauka |
|---|---:|---:|---:|---:|---:|
| Ratas v0.1 | 10 | 100000 | 0 | 200000 | 0 |
| Ratas v0.1 | 100 | 100000 | 0 | 200000 | 0 |
| Ratas v0.1 | 500 | 100000 | 0 | 200000 | 0 |
| Ratas v0.1 | 1000 | 100000 | 0 | 200000 | 0 |

## Struktūruotos įvestys

Rinkinys: visi `abcdefgh` perstatymai (40320), `a`×k (k=0..300), `ab`×k (k=1..150), `abc`×k (k=1..100),
visos 2 simbolių eilutės iš abėcėlės (8836), visi 256 vieno baito atvejai, `ab` + užpildo reikšmę atitinkantys baitai,
`\0`×k (k=0..64). Iš viso 50044 eilučių, skirtingų: 50039.

| Versija | Skirtingų įvesčių | Grupių su bendra santrauka |
|---|---:|---:|
| Ratas v0.1 | 50039 | 0 |

Kolizijų pavyzdžiai (jei rasta): `exp5_collision_examples.txt`.
