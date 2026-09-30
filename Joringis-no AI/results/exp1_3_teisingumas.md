# 1–3 eksperimentai: įvestys, formatas, determinizmas

Įvestys – bendras poros rinkinys `Joringis-no AI/data/exp1/` (34 failai; turinys tikrinamas `Rokas - AI/tests/check_fixtures.py`).

| Įvestis | Baitai | Simboliai (UTF-8) | Aprašymas | Maiša (pradžia) |
|---|---|---|---|---|
| `a.bin` | 1 | 1 | vienas baitas `a`, be naujos eilutės | `3ffc9fcebc47baf7…` |
| `b.bin` | 1 | 1 | vienas baitas `b`, be naujos eilutės | `2e78ecfdcc1a9059…` |
| `empty.bin` | 0 | 0 | tuščias failas | `dc978b5d0c615428…` |
| `random_1.txt` | 1500 | 1500 | atsitiktinis ASCII `!`..`~` | `c255d00946f5710f…` |
| `random_1_end.txt` | 1500 | 1500 | `random_1`, pakeistas 1 baitas gale | `39e11b865d4d9449…` |
| `random_1_middle.txt` | 1500 | 1500 | `random_1`, pakeistas 1 baitas viduryje | `6cd70f475cbb1640…` |
| `random_1_start.txt` | 1500 | 1500 | `random_1`, pakeistas 1 baitas pradžioje | `5eb3a3ae49b9c689…` |
| `random_2.txt` | 2048 | 2048 | atsitiktinis ASCII `!`..`~` | `f9eb3673e64947ff…` |
| `random_2_end.txt` | 2048 | 2048 | `random_2`, pakeistas 1 baitas gale | `8c02f15552f0a0ce…` |
| `random_2_middle.txt` | 2048 | 2048 | `random_2`, pakeistas 1 baitas viduryje | `b822a4e9439464e8…` |
| `random_2_start.txt` | 2048 | 2048 | `random_2`, pakeistas 1 baitas pradžioje | `07099a41167e821d…` |
| `random_3.txt` | 4096 | 4096 | atsitiktinis ASCII `!`..`~` | `e38bd3120f341ddb…` |
| `random_3_end.txt` | 4096 | 4096 | `random_3`, pakeistas 1 baitas gale | `d6105407c777096e…` |
| `random_3_middle.txt` | 4096 | 4096 | `random_3`, pakeistas 1 baitas viduryje | `0fdba81ae4df2d68…` |
| `random_3_start.txt` | 4096 | 4096 | `random_3`, pakeistas 1 baitas pradžioje | `b6619e0a708321c1…` |
| `struct_len15.txt` | 15 | 15 | `x` × 15 | `b4910e6527fb4818…` |
| `struct_len16.txt` | 16 | 16 | `x` × 16 | `f6ce4174637b9853…` |
| `struct_len17.txt` | 17 | 17 | `x` × 17 | `78a2b011ac1ed9b6…` |
| `struct_newline_crlf.txt` | 9 | 9 | `tekstas` + CRLF | `7acfced5af0109b6…` |
| `struct_newline_lf.txt` | 8 | 8 | `tekstas` + LF | `9b3b39a5292fa8a4…` |
| `struct_order_abc.txt` | 3 | 3 | `abc` | `7197b11599ea68b0…` |
| `struct_order_cba.txt` | 3 | 3 | `cba` | `c79b99df4e7bd8b6…` |
| `struct_order_words1.txt` | 11 | 11 | `labas rytas` | `dcdf5d0e569dcc64…` |
| `struct_order_words2.txt` | 11 | 11 | `rytas labas` | `185e6d9fd197b7f7…` |
| `struct_pad_ab.txt` | 2 | 2 | `ab` | `abf6c289948f8e05…` |
| `struct_pad_ab0.txt` | 3 | 3 | `ab` + nulinis baitas | `90de78fe19d72167…` |
| `struct_repeat_a.txt` | 32 | 32 | `a` × 32 | `79fe83b45ce96c8f…` |
| `struct_repeat_ab.txt` | 32 | 32 | `ab` × 16 | `ba8fa52f3f4460fb…` |
| `struct_space_lead.txt` | 8 | 8 | tarpas pradžioje | `b8805ca3f544afef…` |
| `struct_space_none.txt` | 7 | 7 | `tekstas` | `836d7a3e192bf1e3…` |
| `struct_space_none_copy.txt` | 7 | 7 | `tekstas`, kitas failo vardas | `836d7a3e192bf1e3…` |
| `struct_space_trail.txt` | 8 | 8 | tarpas gale | `ce43eae68ba6bc40…` |
| `utf8_lt.txt` | 26 | 15 | lietuviškos raidės (UTF-8) | `b8998088bf06b21c…` |
| `utf8_mixed.txt` | 23 | 19 | ASCII, brūkšnys ir € (UTF-8) | `c257ea7b63b55f10…` |

## Palyginimai poromis

| Pora | Tikimasi | Rezultatas |
|---|---|---|
| `random_1_start.txt` ↔ `random_1.txt` | skiriasi | skiriasi ✓ |
| `random_1_middle.txt` ↔ `random_1.txt` | skiriasi | skiriasi ✓ |
| `random_1_end.txt` ↔ `random_1.txt` | skiriasi | skiriasi ✓ |
| `random_2_start.txt` ↔ `random_2.txt` | skiriasi | skiriasi ✓ |
| `random_2_middle.txt` ↔ `random_2.txt` | skiriasi | skiriasi ✓ |
| `random_2_end.txt` ↔ `random_2.txt` | skiriasi | skiriasi ✓ |
| `random_3_start.txt` ↔ `random_3.txt` | skiriasi | skiriasi ✓ |
| `random_3_middle.txt` ↔ `random_3.txt` | skiriasi | skiriasi ✓ |
| `random_3_end.txt` ↔ `random_3.txt` | skiriasi | skiriasi ✓ |
| `b.bin` ↔ `a.bin` | skiriasi | skiriasi ✓ |
| `struct_order_cba.txt` ↔ `struct_order_abc.txt` | skiriasi | skiriasi ✓ |
| `struct_order_words2.txt` ↔ `struct_order_words1.txt` | skiriasi | skiriasi ✓ |
| `struct_pad_ab0.txt` ↔ `struct_pad_ab.txt` | skiriasi | skiriasi ✓ |
| `struct_space_lead.txt` ↔ `struct_space_none.txt` | skiriasi | skiriasi ✓ |
| `struct_space_trail.txt` ↔ `struct_space_none.txt` | skiriasi | skiriasi ✓ |
| `struct_newline_lf.txt` ↔ `struct_space_none.txt` | skiriasi | skiriasi ✓ |
| `struct_newline_crlf.txt` ↔ `struct_newline_lf.txt` | skiriasi | skiriasi ✓ |
| `struct_len16.txt` ↔ `struct_len15.txt` | skiriasi | skiriasi ✓ |
| `struct_len17.txt` ↔ `struct_len16.txt` | skiriasi | skiriasi ✓ |
| `struct_repeat_ab.txt` ↔ `struct_repeat_a.txt` | skiriasi | skiriasi ✓ |
| `struct_space_none_copy.txt` ↔ `struct_space_none.txt` | sutampa | sutampa ✓ |

## Formatas ir determinizmas

| Patikra | Rezultatas |
|---|---|
| 64 hex simboliai, mažosios raidės, dekoduojasi į tą pačią maišą | 34/34 |
| 3 kartotiniai kvietimai duoda tą pačią maišą | 34/34 |
| Seka A, B, A (A sutampa, B skiriasi) | taip |
| 1 000 kvietimų su `random_3.txt` | taip |
| Maišos, prasidedančios `0`, iš `0000`–`9999` (tikėtina ≈ 625) | 637 |
| Maišos, prasidedančios `00` (tikėtina ≈ 39) | 42 |

Pradiniai nuliai išsaugomi:

* `0462` → `008a5068efb428965701ad670d9cb18c8652b0f6131430c5c2a7dcdc353b2b97` (64 simboliai)
* `0869` → `002a7934b3f05456c4b3b7c434af52af66125077764e5f47263e319fe74e8617` (64 simboliai)
* `0982` → `00a20ec7813e2bbba49c54f368b75f49a3c4ef104a9d207b8c23b89e0d92740d` (64 simboliai)

## Komandinė eilutė

| Patikra | Rezultatas |
|---|---|
| Du atskiri programos paleidimai su failo argumentu duoda tą pačią maišą | 34/34 |
| Failo argumento maiša sutampa su maiša, apskaičiuota programos viduje | 34/34 |
| Ranka įvestas tekstas (+ Enter) sutampa su failo maiša | 31/31 |
| Meniu 2 (kelias įvestas ranka) sutampa su failo maiša | 31/31 |

Ranka tikrinami failai be naujos eilutės ir nulinių baitų – tik tokius galima įvesti viena eilute.
