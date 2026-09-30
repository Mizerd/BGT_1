# 1–3 eksperimentai: įvestys, formatas, determinizmas

Įvestys – bendras poros rinkinys `Joringis-no AI/data/exp1/` (34 failai; turinys tikrinamas `Rokas - AI/tests/check_fixtures.py`).

| Įvestis | Baitai | Simboliai (UTF-8) | Aprašymas | Maiša (pradžia) |
|---|---|---|---|---|
| `a.bin` | 1 | 1 | vienas baitas `a`, be naujos eilutės | `23c85b745d9b9078…` |
| `b.bin` | 1 | 1 | vienas baitas `b`, be naujos eilutės | `8b6a10c5d1d27b2a…` |
| `empty.bin` | 0 | 0 | tuščias failas | `f83958be8ca002bc…` |
| `random_1.txt` | 1500 | 1500 | atsitiktinis ASCII `!`..`~` | `2bb37b4acf07217f…` |
| `random_1_end.txt` | 1500 | 1500 | `random_1`, pakeistas 1 baitas gale | `cce4bf38dda099c8…` |
| `random_1_middle.txt` | 1500 | 1500 | `random_1`, pakeistas 1 baitas viduryje | `0e765b4084d9bd52…` |
| `random_1_start.txt` | 1500 | 1500 | `random_1`, pakeistas 1 baitas pradžioje | `e5d649dfc9acb9b7…` |
| `random_2.txt` | 2048 | 2048 | atsitiktinis ASCII `!`..`~` | `89731f3200f853c2…` |
| `random_2_end.txt` | 2048 | 2048 | `random_2`, pakeistas 1 baitas gale | `18b511acf7c2163f…` |
| `random_2_middle.txt` | 2048 | 2048 | `random_2`, pakeistas 1 baitas viduryje | `35e345e78e87f014…` |
| `random_2_start.txt` | 2048 | 2048 | `random_2`, pakeistas 1 baitas pradžioje | `68a75546b22a2e12…` |
| `random_3.txt` | 4096 | 4096 | atsitiktinis ASCII `!`..`~` | `16d13d59db0651d5…` |
| `random_3_end.txt` | 4096 | 4096 | `random_3`, pakeistas 1 baitas gale | `ed3abdfa24f08e80…` |
| `random_3_middle.txt` | 4096 | 4096 | `random_3`, pakeistas 1 baitas viduryje | `31a67f85b6fe27d7…` |
| `random_3_start.txt` | 4096 | 4096 | `random_3`, pakeistas 1 baitas pradžioje | `e90bddbbc282e433…` |
| `struct_len15.txt` | 15 | 15 | `x` × 15 | `23a4b23a47ffd070…` |
| `struct_len16.txt` | 16 | 16 | `x` × 16 | `51aae143ea611f13…` |
| `struct_len17.txt` | 17 | 17 | `x` × 17 | `1e584fbda21fe880…` |
| `struct_newline_crlf.txt` | 9 | 9 | `tekstas` + CRLF | `8736b5476001cd12…` |
| `struct_newline_lf.txt` | 8 | 8 | `tekstas` + LF | `53c0ea455f7248e8…` |
| `struct_order_abc.txt` | 3 | 3 | `abc` | `a881978cbef8990b…` |
| `struct_order_cba.txt` | 3 | 3 | `cba` | `ebef26ee20cf53c2…` |
| `struct_order_words1.txt` | 11 | 11 | `labas rytas` | `2b750aa87656da72…` |
| `struct_order_words2.txt` | 11 | 11 | `rytas labas` | `2eccd148493f4be7…` |
| `struct_pad_ab.txt` | 2 | 2 | `ab` | `60227218b93b6550…` |
| `struct_pad_ab0.txt` | 3 | 3 | `ab` + nulinis baitas | `91f8cb30491164a4…` |
| `struct_repeat_a.txt` | 32 | 32 | `a` × 32 | `928672f1c5c3aa94…` |
| `struct_repeat_ab.txt` | 32 | 32 | `ab` × 16 | `989039bd6d7a3ae8…` |
| `struct_space_lead.txt` | 8 | 8 | tarpas pradžioje | `74614ef24314ed8f…` |
| `struct_space_none.txt` | 7 | 7 | `tekstas` | `0a55e930958d244b…` |
| `struct_space_none_copy.txt` | 7 | 7 | `tekstas`, kitas failo vardas | `0a55e930958d244b…` |
| `struct_space_trail.txt` | 8 | 8 | tarpas gale | `3eab323f895c8cca…` |
| `utf8_lt.txt` | 26 | 15 | lietuviškos raidės (UTF-8) | `903e6edf7496a365…` |
| `utf8_mixed.txt` | 23 | 19 | ASCII, brūkšnys ir € (UTF-8) | `8e60cb75ebf8d3fd…` |

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
| Maišos, prasidedančios `0`, iš `0000`–`9999` (tikėtina ≈ 625) | 629 |
| Maišos, prasidedančios `00` (tikėtina ≈ 39) | 32 |

Pradiniai nuliai išsaugomi:

* `0244` → `00bb64c1ec9ad497b65d8ea598408466833a9c666461b034cabfe8452c8bdac3` (64 simboliai)
* `1088` → `0054568b9045875532b39a6ee22f45fbb607ce1af928e07c3ef0b6324be19429` (64 simboliai)
* `1439` → `00a39079f068e6df29944aa3d7d6533ca4edf02fa9ef76a0b630eaf56b13b3a5` (64 simboliai)

## Komandinė eilutė

| Patikra | Rezultatas |
|---|---|
| Du atskiri programos paleidimai su `--file` duoda tą pačią maišą | 34/34 |
| `--file` maiša sutampa su maiša, apskaičiuota programos viduje | 34/34 |
| Ranka įvestas tekstas (+ Enter) sutampa su failo maiša | 31/31 |
| `--text` sutampa su failo maiša | 31/31 |

Ranka ir `--text` tikrinami failai be naujos eilutės ir nulinių baitų – tik tokius galima įvesti viena eilute.
