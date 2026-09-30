# 1–3 eksperimentai: įvestys, formatas, determinizmas

Įvestys – bendras poros rinkinys `Joringis-no AI/data/exp1/` (34 failai; turinys tikrinamas `tests/check_fixtures.py`).

| Įvestis | Baitai | Simboliai (UTF-8) | Aprašymas | Maiša (pradžia) |
|---|---|---|---|---|
| `a.bin` | 1 | 1 | vienas baitas `a`, be naujos eilutės | `c65cfefd8cd3f6fc…` |
| `b.bin` | 1 | 1 | vienas baitas `b`, be naujos eilutės | `02ab611e7bd95570…` |
| `empty.bin` | 0 | 0 | tuščias failas | `5c39cf532414769f…` |
| `random_1.txt` | 1500 | 1500 | atsitiktinis ASCII `!`..`~` | `90338c2606d8f89a…` |
| `random_1_end.txt` | 1500 | 1500 | `random_1`, pakeistas 1 baitas gale | `d2ab5fbc5666dde3…` |
| `random_1_middle.txt` | 1500 | 1500 | `random_1`, pakeistas 1 baitas viduryje | `2dc9fd07c4a161f6…` |
| `random_1_start.txt` | 1500 | 1500 | `random_1`, pakeistas 1 baitas pradžioje | `08165985bf4125c3…` |
| `random_2.txt` | 2048 | 2048 | atsitiktinis ASCII `!`..`~` | `9ca2ab8349fd26ab…` |
| `random_2_end.txt` | 2048 | 2048 | `random_2`, pakeistas 1 baitas gale | `b97a0d467ef6e5a3…` |
| `random_2_middle.txt` | 2048 | 2048 | `random_2`, pakeistas 1 baitas viduryje | `b1d52a12c6128640…` |
| `random_2_start.txt` | 2048 | 2048 | `random_2`, pakeistas 1 baitas pradžioje | `68bc6e7291a0ebfc…` |
| `random_3.txt` | 4096 | 4096 | atsitiktinis ASCII `!`..`~` | `b68de56738eea5b2…` |
| `random_3_end.txt` | 4096 | 4096 | `random_3`, pakeistas 1 baitas gale | `2339d3736bdf6896…` |
| `random_3_middle.txt` | 4096 | 4096 | `random_3`, pakeistas 1 baitas viduryje | `9b7ea28790bedd84…` |
| `random_3_start.txt` | 4096 | 4096 | `random_3`, pakeistas 1 baitas pradžioje | `ef827d0424fa443e…` |
| `struct_len15.txt` | 15 | 15 | `x` × 15 | `c5324ed813a0636a…` |
| `struct_len16.txt` | 16 | 16 | `x` × 16 | `fed586efe1b6aca7…` |
| `struct_len17.txt` | 17 | 17 | `x` × 17 | `5ab7d770fb5758a4…` |
| `struct_newline_crlf.txt` | 9 | 9 | `tekstas` + CRLF | `3b80238e3654f920…` |
| `struct_newline_lf.txt` | 8 | 8 | `tekstas` + LF | `b35b915336812374…` |
| `struct_order_abc.txt` | 3 | 3 | `abc` | `e43d118f0fad59b9…` |
| `struct_order_cba.txt` | 3 | 3 | `cba` | `7fea1005d766298d…` |
| `struct_order_words1.txt` | 11 | 11 | `labas rytas` | `7cea7ad822670e8b…` |
| `struct_order_words2.txt` | 11 | 11 | `rytas labas` | `9a1a810cec2dd929…` |
| `struct_pad_ab.txt` | 2 | 2 | `ab` | `9ffecc08129c46d3…` |
| `struct_pad_ab0.txt` | 3 | 3 | `ab` + nulinis baitas | `28263cfa47a63dc9…` |
| `struct_repeat_a.txt` | 32 | 32 | `a` × 32 | `56aa0c81f5b806a5…` |
| `struct_repeat_ab.txt` | 32 | 32 | `ab` × 16 | `75d054e1c83c6664…` |
| `struct_space_lead.txt` | 8 | 8 | tarpas pradžioje | `35c623b3275e0288…` |
| `struct_space_none.txt` | 7 | 7 | `tekstas` | `ec17c9204be10c95…` |
| `struct_space_none_copy.txt` | 7 | 7 | `tekstas`, kitas failo vardas | `ec17c9204be10c95…` |
| `struct_space_trail.txt` | 8 | 8 | tarpas gale | `256bf2e8f43a3055…` |
| `utf8_lt.txt` | 26 | 15 | lietuviškos raidės (UTF-8) | `0cd6d55c9b78c4c3…` |
| `utf8_mixed.txt` | 23 | 19 | ASCII, brūkšnys ir € (UTF-8) | `f7467f49cd32c8b9…` |

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
| Maišos, prasidedančios `0`, iš `0000`–`9999` (tikėtina ≈ 625) | 616 |
| Maišos, prasidedančios `00` (tikėtina ≈ 39) | 48 |

Pradiniai nuliai išsaugomi:

* `0179` → `009150c31b42e48a000c9a8567dea75d5e56480e2fd12611d0e2ed180adccabf` (64 simboliai)
* `0284` → `00a6e6d6cdd1ff52fc2233ad476a55708f41b0e3f6682898eeb28d28f8459030` (64 simboliai)
* `0488` → `002bc6271ad89aeebcc33f4f38cd8817ecadaeec196a9a4ee27292393ff7c94e` (64 simboliai)

## Komandinė eilutė

| Patikra | Rezultatas |
|---|---|
| Du atskiri programos paleidimai su failo argumentu duoda tą pačią maišą | 34/34 |
| Failo argumento maiša sutampa su maiša, apskaičiuota programos viduje | 34/34 |
| Ranka įvestas tekstas (+ Enter) sutampa su failo maiša | 31/31 |
| Meniu 2 (kelias įvestas ranka) sutampa su failo maiša | 31/31 |

Ranka tikrinami failai be naujos eilutės ir nulinių baitų – tik tokius galima įvesti viena eilute.
