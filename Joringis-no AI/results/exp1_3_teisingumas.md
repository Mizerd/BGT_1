# 1–3 eksperimentai: įvestys, formatas, determinizmas

Įvestys – bendras poros rinkinys `Joringis-no AI/data/exp1/` ir papildomas tikras CRLF atvejis, sudarytas atmintyje.

| Įvestis | Baitai | Simboliai (UTF-8) | Aprašymas | Maiša (pradžia) |
|---|---|---|---|---|
| `a.bin` | 1 | 1 | vienas baitas `a`, be naujos eilutės | `d657166d0fbb82fe…` |
| `b.bin` | 1 | 1 | vienas baitas `b`, be naujos eilutės | `ed07e77ea02ad64e…` |
| `empty.bin` | 0 | 0 | tuščias failas | `25bf155273a70086…` |
| `random_1.txt` | 1500 | 1500 | atsitiktinis ASCII `!`..`~` | `7563094a47a7f3b6…` |
| `random_1_end.txt` | 1500 | 1500 | `random_1`, pakeistas 1 baitas gale | `27acb26c13900f4f…` |
| `random_1_middle.txt` | 1500 | 1500 | `random_1`, pakeistas 1 baitas viduryje | `a87728b335a3f684…` |
| `random_1_start.txt` | 1500 | 1500 | `random_1`, pakeistas 1 baitas pradžioje | `7831617a2d23368e…` |
| `random_2.txt` | 2048 | 2048 | atsitiktinis ASCII `!`..`~` | `ca361ee66446f484…` |
| `random_2_end.txt` | 2048 | 2048 | `random_2`, pakeistas 1 baitas gale | `7fbe6302aef47d16…` |
| `random_2_middle.txt` | 2048 | 2048 | `random_2`, pakeistas 1 baitas viduryje | `fed01468155d6a2b…` |
| `random_2_start.txt` | 2048 | 2048 | `random_2`, pakeistas 1 baitas pradžioje | `e86ed4cf458b2c1a…` |
| `random_3.txt` | 4096 | 4096 | atsitiktinis ASCII `!`..`~` | `bf0574e66c59816c…` |
| `random_3_end.txt` | 4096 | 4096 | `random_3`, pakeistas 1 baitas gale | `5d8793eb0f8b032b…` |
| `random_3_middle.txt` | 4096 | 4096 | `random_3`, pakeistas 1 baitas viduryje | `cb1ce6677e337b7a…` |
| `random_3_start.txt` | 4096 | 4096 | `random_3`, pakeistas 1 baitas pradžioje | `576863a17837552f…` |
| `struct_len15.txt` | 15 | 15 | `x` × 15 | `0bde54252aa2136e…` |
| `struct_len16.txt` | 16 | 16 | `x` × 16 | `542d5d8ef9481fe5…` |
| `struct_len17.txt` | 17 | 17 | `x` × 17 | `32e362a484bb1834…` |
| `struct_newline_crlf.txt` | 8 | 8 | pagal pavadinimą CRLF, bet faile LF (baitai kaip `_lf`) | `24ef7e95a97721b6…` |
| `struct_newline_lf.txt` | 8 | 8 | `tekstas` + LF | `24ef7e95a97721b6…` |
| `struct_order_abc.txt` | 3 | 3 | `abc` | `0592e0a70d07a3c4…` |
| `struct_order_cba.txt` | 3 | 3 | `cba` | `8bfde162bff34f72…` |
| `struct_order_words1.txt` | 11 | 11 | `labas rytas` | `92f1a47fb9545b50…` |
| `struct_order_words2.txt` | 11 | 11 | `rytas labas` | `dd432e72fb3c3fab…` |
| `struct_pad_ab.txt` | 2 | 2 | `ab` | `47740998562307ee…` |
| `struct_pad_ab0.txt` | 3 | 3 | `ab` + nulinis baitas | `bfa18bb37eb3caaf…` |
| `struct_repeat_a.txt` | 32 | 32 | `a` × 32 | `3494449e6eca6681…` |
| `struct_repeat_ab.txt` | 32 | 32 | `ab` × 16 | `f26c8a25f26c5cab…` |
| `struct_space_lead.txt` | 8 | 8 | tarpas pradžioje | `adbb15f653be3021…` |
| `struct_space_none.txt` | 7 | 7 | `tekstas` | `b7e64dce66e8c28f…` |
| `struct_space_none_copy.txt` | 7 | 7 | `tekstas`, kitas failo vardas | `b7e64dce66e8c28f…` |
| `struct_space_trail.txt` | 8 | 8 | tarpas gale | `14efdde82bf23c5f…` |
| `utf8_lt.txt` | 26 | 15 | lietuviškos raidės (UTF-8) | `f0435b0b19975fda…` |
| `utf8_mixed.txt` | 23 | 19 | ASCII, brūkšnys ir € (UTF-8) | `f3322c1e9e6482a6…` |
| `crlf_atmintyje` | 9 | 9 | `tekstas` + CRLF (sudaryta atmintyje) | `4958acbadeeb0fbd…` |

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
| `crlf_atmintyje` ↔ `struct_newline_lf.txt` | skiriasi | skiriasi ✓ |
| `struct_len16.txt` ↔ `struct_len15.txt` | skiriasi | skiriasi ✓ |
| `struct_len17.txt` ↔ `struct_len16.txt` | skiriasi | skiriasi ✓ |
| `struct_repeat_ab.txt` ↔ `struct_repeat_a.txt` | skiriasi | skiriasi ✓ |
| `struct_space_none_copy.txt` ↔ `struct_space_none.txt` | sutampa | sutampa ✓ |
| `struct_newline_crlf.txt` ↔ `struct_newline_lf.txt` | sutampa | sutampa ✓ |

`struct_newline_crlf.txt` bendrame rinkinyje saugomas su LF, todėl jo baitai sutampa su `_lf`. Tikras CRLF atvejis patikrintas eilute `crlf_atmintyje`.

## Formatas ir determinizmas

| Patikra | Rezultatas |
|---|---|
| 64 hex simboliai, mažosios raidės, dekoduojasi į tą pačią maišą | 35/35 |
| 3 kartotiniai kvietimai duoda tą pačią maišą | 35/35 |
| Seka A, B, A (A sutampa, B skiriasi) | taip |
| 1 000 kvietimų su `random_3.txt` | taip |
| Maišos, prasidedančios `0`, iš `0000`–`9999` (tikėtina ≈ 625) | 608 |
| Maišos, prasidedančios `00` (tikėtina ≈ 39) | 30 |

Pradiniai nuliai išsaugomi:

* `0066` → `008ba3d17a7e3574882d121dfc3fbc50d7d3739bc26766fa6f8b5e9abfcb013b` (64 simboliai)
* `0806` → `00508b3d659a62b134060b3bfc20192a3acd6d4347646d34dd2de8e9f9f42263` (64 simboliai)
* `0920` → `0094782310af0cd6fad922a5adaee558f83aa7d5a9fa51d445c17724c66627f5` (64 simboliai)

## Komandinė eilutė

| Patikra | Rezultatas |
|---|---|
| Du atskiri programos paleidimai su `--file` duoda tą pačią maišą | 34/34 |
| `--file` maiša sutampa su maiša, apskaičiuota programos viduje | 34/34 |
| Ranka įvestas tekstas (+ Enter) sutampa su failo maiša | 31/31 |
| `--text` sutampa su failo maiša | 31/31 |

Ranka ir `--text` tikrinami failai be naujos eilutės ir nulinių baitų – tik tokius galima įvesti viena eilute.
