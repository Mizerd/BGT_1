# 1–3 eksperimentai: įvestys, formatas, determinizmas

Įvestys – bendras poros rinkinys `Joringis-no AI/data/exp1/` (34 failai; turinys tikrinamas `tests/check_fixtures.py`).

| Įvestis | Baitai | Simboliai (UTF-8) | Aprašymas | Maiša (pradžia) |
|---|---|---|---|---|
| `a.bin` | 1 | 1 | vienas baitas `a`, be naujos eilutės | `3eb973212ce7ce3d…` |
| `b.bin` | 1 | 1 | vienas baitas `b`, be naujos eilutės | `a76abbfd6bcca713…` |
| `empty.bin` | 0 | 0 | tuščias failas | `5b0c66f97035a8f5…` |
| `random_1.txt` | 1500 | 1500 | atsitiktinis ASCII `!`..`~` | `28ff67d55c1ce8ed…` |
| `random_1_end.txt` | 1500 | 1500 | `random_1`, pakeistas 1 baitas gale | `e9118f928a1ebcb1…` |
| `random_1_middle.txt` | 1500 | 1500 | `random_1`, pakeistas 1 baitas viduryje | `e9cd58f2f43e8031…` |
| `random_1_start.txt` | 1500 | 1500 | `random_1`, pakeistas 1 baitas pradžioje | `dd0adb0cfd509f2e…` |
| `random_2.txt` | 2048 | 2048 | atsitiktinis ASCII `!`..`~` | `d1bbbc5e3023dcb9…` |
| `random_2_end.txt` | 2048 | 2048 | `random_2`, pakeistas 1 baitas gale | `d020ac42b04e7ba2…` |
| `random_2_middle.txt` | 2048 | 2048 | `random_2`, pakeistas 1 baitas viduryje | `7ec916d1565052e1…` |
| `random_2_start.txt` | 2048 | 2048 | `random_2`, pakeistas 1 baitas pradžioje | `de431f0d65dd624a…` |
| `random_3.txt` | 4096 | 4096 | atsitiktinis ASCII `!`..`~` | `5d6968c7e10a988a…` |
| `random_3_end.txt` | 4096 | 4096 | `random_3`, pakeistas 1 baitas gale | `753748f53c527c0a…` |
| `random_3_middle.txt` | 4096 | 4096 | `random_3`, pakeistas 1 baitas viduryje | `9f61871fdef07af5…` |
| `random_3_start.txt` | 4096 | 4096 | `random_3`, pakeistas 1 baitas pradžioje | `41537ffe5033c456…` |
| `struct_len15.txt` | 15 | 15 | `x` × 15 | `ad9b2cf0d2719bf7…` |
| `struct_len16.txt` | 16 | 16 | `x` × 16 | `469dea3d548d70d4…` |
| `struct_len17.txt` | 17 | 17 | `x` × 17 | `c8e67a3dea217da5…` |
| `struct_newline_crlf.txt` | 9 | 9 | `tekstas` + CRLF | `4573234ceebf4245…` |
| `struct_newline_lf.txt` | 8 | 8 | `tekstas` + LF | `b34e3663716f3c8b…` |
| `struct_order_abc.txt` | 3 | 3 | `abc` | `acbd9460c71c3bbb…` |
| `struct_order_cba.txt` | 3 | 3 | `cba` | `3c63b188f3d67075…` |
| `struct_order_words1.txt` | 11 | 11 | `labas rytas` | `948e37a450b7738e…` |
| `struct_order_words2.txt` | 11 | 11 | `rytas labas` | `5751c7aee912384b…` |
| `struct_pad_ab.txt` | 2 | 2 | `ab` | `812b2ffdd1ae5ee1…` |
| `struct_pad_ab0.txt` | 3 | 3 | `ab` + nulinis baitas | `27c5e07f891f11df…` |
| `struct_repeat_a.txt` | 32 | 32 | `a` × 32 | `0f357a67fcddb376…` |
| `struct_repeat_ab.txt` | 32 | 32 | `ab` × 16 | `e319e147d5435b44…` |
| `struct_space_lead.txt` | 8 | 8 | tarpas pradžioje | `f6d831328ef34e8d…` |
| `struct_space_none.txt` | 7 | 7 | `tekstas` | `7dec86d666c9fdcc…` |
| `struct_space_none_copy.txt` | 7 | 7 | `tekstas`, kitas failo vardas | `7dec86d666c9fdcc…` |
| `struct_space_trail.txt` | 8 | 8 | tarpas gale | `14efdde82bf23c5f…` |
| `utf8_lt.txt` | 26 | 15 | lietuviškos raidės (UTF-8) | `01bd01fba24e5a81…` |
| `utf8_mixed.txt` | 23 | 19 | ASCII, brūkšnys ir € (UTF-8) | `2ebf21db99ef7d03…` |

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
| Maišos, prasidedančios `0`, iš `0000`–`9999` (tikėtina ≈ 625) | 626 |
| Maišos, prasidedančios `00` (tikėtina ≈ 39) | 50 |

Pradiniai nuliai išsaugomi:

* `0200` → `00a752f7c5bd1f52961d7f26b359bfa389fef8bbe61e9ea9bbe92505dd7909ab` (64 simboliai)
* `0920` → `0094782310af0cd6fad922a5adaee558f83aa7d5a9fa51d445c17724c66627f5` (64 simboliai)
* `0979` → `0059dd5b735a05e72ab3391d032b88e12cd4c899bd43e48cf619ac42f00fca1f` (64 simboliai)

## Komandinė eilutė

| Patikra | Rezultatas |
|---|---|
| Du atskiri programos paleidimai su failo argumentu duoda tą pačią maišą | 34/34 |
| Failo argumento maiša sutampa su maiša, apskaičiuota programos viduje | 34/34 |
| Ranka įvestas tekstas (+ Enter) sutampa su failo maiša | 31/31 |
| Meniu 2 (kelias įvestas ranka) sutampa su failo maiša | 31/31 |

Ranka tikrinami failai be naujos eilutės ir nulinių baitų – tik tokius galima įvesti viena eilute.
