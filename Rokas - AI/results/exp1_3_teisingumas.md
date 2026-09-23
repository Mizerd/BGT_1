# 1–3 eksperimentai: įvestys, formatas, determinizmas

Įvestys – bendras poros rinkinys `Joringis-no AI/data/exp1/` (seed 20260920, abėcėlė `!`..`~`) ir papildomas tikras CRLF atvejis, sudarytas atmintyje.

| Įvestis | Baitai | Simboliai (UTF-8) | Aprašymas | 2 versijos maiša (pradžia) |
|---|---|---|---|---|
| `a.bin` | 1 | 1 | vienas baitas `a`, be naujos eilutės | `9572b1ea73ece20f…` |
| `b.bin` | 1 | 1 | vienas baitas `b`, be naujos eilutės | `fb3e28dc3f71ef78…` |
| `empty.bin` | 0 | 0 | tuščias failas | `547ac2e87baff518…` |
| `random_1.txt` | 1500 | 1500 | atsitiktinis ASCII `!`..`~` | `c9b9d655d6fc8132…` |
| `random_1_end.txt` | 1500 | 1500 | `random_1`, pakeistas 1 baitas gale | `b626132890d5b4f8…` |
| `random_1_middle.txt` | 1500 | 1500 | `random_1`, pakeistas 1 baitas viduryje | `f688c959f0ab934b…` |
| `random_1_start.txt` | 1500 | 1500 | `random_1`, pakeistas 1 baitas pradžioje | `cac8c3a77bfd7f67…` |
| `random_2.txt` | 2048 | 2048 | atsitiktinis ASCII `!`..`~` | `5c625363aa69616f…` |
| `random_2_end.txt` | 2048 | 2048 | `random_2`, pakeistas 1 baitas gale | `fee069a80ad98938…` |
| `random_2_middle.txt` | 2048 | 2048 | `random_2`, pakeistas 1 baitas viduryje | `88e7f589c8b34675…` |
| `random_2_start.txt` | 2048 | 2048 | `random_2`, pakeistas 1 baitas pradžioje | `3fd7ac2c89971895…` |
| `random_3.txt` | 4096 | 4096 | atsitiktinis ASCII `!`..`~` | `3260e997acd0a9f8…` |
| `random_3_end.txt` | 4096 | 4096 | `random_3`, pakeistas 1 baitas gale | `84a7c6f90e9805f3…` |
| `random_3_middle.txt` | 4096 | 4096 | `random_3`, pakeistas 1 baitas viduryje | `c18538a95873bd25…` |
| `random_3_start.txt` | 4096 | 4096 | `random_3`, pakeistas 1 baitas pradžioje | `21da002d7b10cd7d…` |
| `struct_len15.txt` | 15 | 15 | `x` × 15 | `09940d6257aba707…` |
| `struct_len16.txt` | 16 | 16 | `x` × 16 | `7b15abd49be581fc…` |
| `struct_len17.txt` | 17 | 17 | `x` × 17 | `d701d7e68a2d8be7…` |
| `struct_newline_crlf.txt` | 8 | 8 | pagal pavadinimą CRLF, bet faile LF (baitai kaip `_lf`) | `2f9364dc946f475a…` |
| `struct_newline_lf.txt` | 8 | 8 | `tekstas` + LF | `2f9364dc946f475a…` |
| `struct_order_abc.txt` | 3 | 3 | `abc` | `060f1c0f305405e0…` |
| `struct_order_cba.txt` | 3 | 3 | `cba` | `907404a8ef8088e6…` |
| `struct_order_words1.txt` | 11 | 11 | `labas rytas` | `1837ab2190821c5e…` |
| `struct_order_words2.txt` | 11 | 11 | `rytas labas` | `06d42660d06b83ed…` |
| `struct_pad_ab.txt` | 2 | 2 | `ab` | `938953512772dcca…` |
| `struct_pad_ab0.txt` | 3 | 3 | `ab` + nulinis baitas | `00eb6a31277464fe…` |
| `struct_repeat_a.txt` | 32 | 32 | `a` × 32 | `7afb03652689b938…` |
| `struct_repeat_ab.txt` | 32 | 32 | `ab` × 16 | `41482166d4950b01…` |
| `struct_space_lead.txt` | 8 | 8 | tarpas pradžioje | `5ec0d8d62cd54d96…` |
| `struct_space_none.txt` | 7 | 7 | `tekstas` | `0e02e50db24ca488…` |
| `struct_space_none_copy.txt` | 7 | 7 | `tekstas`, kitas failo vardas | `0e02e50db24ca488…` |
| `struct_space_trail.txt` | 8 | 8 | tarpas gale | `7d24c45b6d2ee3aa…` |
| `utf8_lt.txt` | 26 | 15 | lietuviškos raidės (UTF-8) | `6a4e81e1a5369044…` |
| `utf8_mixed.txt` | 23 | 19 | ASCII, brūkšnys ir € (UTF-8) | `6498dec29cae97da…` |
| `crlf_atmintyje` | 9 | 9 | `tekstas` + CRLF (sudaryta atmintyje) | `d07259291e5a0621…` |

## Palyginimai poromis

| Pora | Tikimasi | 2 versija | 1 versija | Ratas v0.1 |
|---|---|---|---|---|
| `random_1_start.txt` ↔ `random_1.txt` | skiriasi | skiriasi ✓ | skiriasi ✓ | skiriasi ✓ |
| `random_1_middle.txt` ↔ `random_1.txt` | skiriasi | skiriasi ✓ | skiriasi ✓ | skiriasi ✓ |
| `random_1_end.txt` ↔ `random_1.txt` | skiriasi | skiriasi ✓ | skiriasi ✓ | skiriasi ✓ |
| `random_2_start.txt` ↔ `random_2.txt` | skiriasi | skiriasi ✓ | skiriasi ✓ | skiriasi ✓ |
| `random_2_middle.txt` ↔ `random_2.txt` | skiriasi | skiriasi ✓ | skiriasi ✓ | skiriasi ✓ |
| `random_2_end.txt` ↔ `random_2.txt` | skiriasi | skiriasi ✓ | skiriasi ✓ | skiriasi ✓ |
| `random_3_start.txt` ↔ `random_3.txt` | skiriasi | skiriasi ✓ | skiriasi ✓ | skiriasi ✓ |
| `random_3_middle.txt` ↔ `random_3.txt` | skiriasi | skiriasi ✓ | skiriasi ✓ | skiriasi ✓ |
| `random_3_end.txt` ↔ `random_3.txt` | skiriasi | skiriasi ✓ | skiriasi ✓ | skiriasi ✓ |
| `b.bin` ↔ `a.bin` | skiriasi | skiriasi ✓ | skiriasi ✓ | skiriasi ✓ |
| `struct_order_cba.txt` ↔ `struct_order_abc.txt` | skiriasi | skiriasi ✓ | skiriasi ✓ | skiriasi ✓ |
| `struct_order_words2.txt` ↔ `struct_order_words1.txt` | skiriasi | skiriasi ✓ | skiriasi ✓ | skiriasi ✓ |
| `struct_pad_ab0.txt` ↔ `struct_pad_ab.txt` | skiriasi | skiriasi ✓ | skiriasi ✓ | skiriasi ✓ |
| `struct_space_lead.txt` ↔ `struct_space_none.txt` | skiriasi | skiriasi ✓ | skiriasi ✓ | skiriasi ✓ |
| `struct_space_trail.txt` ↔ `struct_space_none.txt` | skiriasi | skiriasi ✓ | skiriasi ✓ | skiriasi ✓ |
| `struct_newline_lf.txt` ↔ `struct_space_none.txt` | skiriasi | skiriasi ✓ | skiriasi ✓ | skiriasi ✓ |
| `crlf_atmintyje` ↔ `struct_newline_lf.txt` | skiriasi | skiriasi ✓ | skiriasi ✓ | skiriasi ✓ |
| `struct_len16.txt` ↔ `struct_len15.txt` | skiriasi | skiriasi ✓ | skiriasi ✓ | skiriasi ✓ |
| `struct_len17.txt` ↔ `struct_len16.txt` | skiriasi | skiriasi ✓ | skiriasi ✓ | skiriasi ✓ |
| `struct_repeat_ab.txt` ↔ `struct_repeat_a.txt` | skiriasi | skiriasi ✓ | skiriasi ✓ | skiriasi ✓ |
| `struct_space_none_copy.txt` ↔ `struct_space_none.txt` | sutampa | sutampa ✓ | sutampa ✓ | sutampa ✓ |
| `struct_newline_crlf.txt` ↔ `struct_newline_lf.txt` | sutampa | sutampa ✓ | sutampa ✓ | sutampa ✓ |

`struct_newline_crlf.txt` repozitorijoje saugomas su LF, todėl jo baitai sutampa su `_lf` ir maišos turi sutapti. Tikras CRLF atvejis patikrintas eilute `crlf_atmintyje`.

## Formatas ir determinizmas

| Patikra | 2 versija | 1 versija | Ratas v0.1 |
|---|---|---|---|
| 64 hex simboliai, vienodas raidžių dydis, dekoduojasi į tą pačią maišą | 35/35 | 35/35 | 35/35 |
| 3 kartotiniai kvietimai duoda tą pačią maišą | 35/35 | 35/35 | 35/35 |
| Seka A, B, A (A sutampa, B skiriasi) | taip | taip | taip |
| 1 000 kvietimų su `random_3.txt` | taip | taip | taip |
| Maišos, prasidedančios `0`, iš `0000`–`9999` (tikėtina ≈ 625) | 626 | 633 | 608 |
| Maišos, prasidedančios `00` (tikėtina ≈ 39) | 32 | 41 | 30 |

Pradiniai nuliai išsaugomi (2 versija):

* `0321` → `00de9b72f46c4aead3b5a952a9f78bfca2e4d9af9b25f0db2397ace89f1f44b3` (64 simboliai)
* `0417` → `0021bafafcbc2df30ac728f67ef57f8df97dd4d48453175076e93d444fff4086` (64 simboliai)
* `0481` → `00cefba73464e2581714e41770cfdbe451676169c9f13576a2db6b96bfb116f2` (64 simboliai)

## Komandinė eilutė (2 versija)

| Patikra | Rezultatas |
|---|---|
| Du atskiri programos paleidimai su `--file` duoda tą pačią maišą | 34/34 |
| `--file` maiša sutampa su maiša, apskaičiuota programos viduje | 34/34 |
| Ranka įvestas tekstas (+ Enter) sutampa su failo maiša | 31/31 |
| `--text` sutampa su failo maiša | 31/31 |

Ranka ir `--text` tikrinami failai be naujos eilutės ir nulinių baitų – tik tokius galima įvesti viena eilute.
