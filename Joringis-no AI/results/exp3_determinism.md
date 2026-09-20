# 3 eksperimentas. Determinizmas (API)

Kiekviena įvestis maišoma 3 kartus iš eilės, tada seka A, B, A (B – kita įvestis),
tada A po 4096 baitų failo. Visi rezultatai turi sutapti. Atskirų programos paleidimų
palyginimas – `tests/cli_checks.sh` (žr. `exp3_cli.md`).

| Failas | 3 kartai | A,B,A | po didelio failo |
|---|:---:|:---:|:---:|
| `empty.bin` | ok | ok | ok |
| `a.bin` | ok | ok | ok |
| `b.bin` | ok | ok | ok |
| `random_1.txt` | ok | ok | ok |
| `random_1_start.txt` | ok | ok | ok |
| `random_1_middle.txt` | ok | ok | ok |
| `random_1_end.txt` | ok | ok | ok |
| `random_2.txt` | ok | ok | ok |
| `random_2_start.txt` | ok | ok | ok |
| `random_2_middle.txt` | ok | ok | ok |
| `random_2_end.txt` | ok | ok | ok |
| `random_3.txt` | ok | ok | ok |
| `random_3_start.txt` | ok | ok | ok |
| `random_3_middle.txt` | ok | ok | ok |
| `random_3_end.txt` | ok | ok | ok |
| `struct_repeat_a.txt` | ok | ok | ok |
| `struct_repeat_ab.txt` | ok | ok | ok |
| `struct_order_abc.txt` | ok | ok | ok |
| `struct_order_cba.txt` | ok | ok | ok |
| `struct_order_words1.txt` | ok | ok | ok |
| `struct_order_words2.txt` | ok | ok | ok |
| `struct_space_none.txt` | ok | ok | ok |
| `struct_space_none_copy.txt` | ok | ok | ok |
| `struct_space_lead.txt` | ok | ok | ok |
| `struct_space_trail.txt` | ok | ok | ok |
| `struct_newline_lf.txt` | ok | ok | ok |
| `struct_newline_crlf.txt` | ok | ok | ok |
| `struct_len15.txt` | ok | ok | ok |
| `struct_len16.txt` | ok | ok | ok |
| `struct_len17.txt` | ok | ok | ok |
| `struct_pad_ab.txt` | ok | ok | ok |
| `struct_pad_ab0.txt` | ok | ok | ok |
| `utf8_lt.txt` | ok | ok | ok |
| `utf8_mixed.txt` | ok | ok | ok |

Neatitikimų: 0.
