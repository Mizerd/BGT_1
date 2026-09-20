# 1 eksperimentas. Testinės įvestys

Sugeneruota programa `experiments inputs`, seed = 20260920, abėcėlė atsitiktiniams failams: ASCII `!`..`~` (94 simboliai, 1 simbolis = 1 baitas).
Failai rašomi dvejetainiu režimu, be jokių eilučių pabaigų keitimų.

| Failas | Baitų | Simbolių (UTF-8) | Aprašymas |
|---|---:|---:|---|
| `empty.bin` | 0 | 0 | tuščias failas |
| `a.bin` | 1 | 1 | vienas baitas 'a', be naujos eilutės |
| `b.bin` | 1 | 1 | vienas baitas 'b', be naujos eilutės |
| `random_1.txt` | 1500 | 1500 | atsitiktinis ASCII (abėcėlė '!'..'~'), seed 20260920 |
| `random_1_start.txt` | 1500 | 1500 | random_1 su pakeistu 1 baitu pozicijoje 0 |
| `random_1_middle.txt` | 1500 | 1500 | random_1 su pakeistu 1 baitu pozicijoje 750 |
| `random_1_end.txt` | 1500 | 1500 | random_1 su pakeistu 1 baitu pozicijoje 1499 |
| `random_2.txt` | 2048 | 2048 | atsitiktinis ASCII (abėcėlė '!'..'~'), seed 20260920 |
| `random_2_start.txt` | 2048 | 2048 | random_2 su pakeistu 1 baitu pozicijoje 0 |
| `random_2_middle.txt` | 2048 | 2048 | random_2 su pakeistu 1 baitu pozicijoje 1024 |
| `random_2_end.txt` | 2048 | 2048 | random_2 su pakeistu 1 baitu pozicijoje 2047 |
| `random_3.txt` | 4096 | 4096 | atsitiktinis ASCII (abėcėlė '!'..'~'), seed 20260920 |
| `random_3_start.txt` | 4096 | 4096 | random_3 su pakeistu 1 baitu pozicijoje 0 |
| `random_3_middle.txt` | 4096 | 4096 | random_3 su pakeistu 1 baitu pozicijoje 2048 |
| `random_3_end.txt` | 4096 | 4096 | random_3 su pakeistu 1 baitu pozicijoje 4095 |
| `struct_repeat_a.txt` | 32 | 32 | 32 kartus 'a' |
| `struct_repeat_ab.txt` | 32 | 32 | 16 kartų 'ab' |
| `struct_order_abc.txt` | 3 | 3 | simbolių tvarka |
| `struct_order_cba.txt` | 3 | 3 | ta pati aibė, kita tvarka |
| `struct_order_words1.txt` | 11 | 11 | žodžių tvarka |
| `struct_order_words2.txt` | 11 | 11 | sukeisti žodžiai |
| `struct_space_none.txt` | 7 | 7 | be tarpų |
| `struct_space_none_copy.txt` | 7 | 7 | tie patys baitai kitu failo vardu (santrauka turi sutapti) |
| `struct_space_lead.txt` | 8 | 8 | tarpas pradžioje |
| `struct_space_trail.txt` | 8 | 8 | tarpas pabaigoje |
| `struct_newline_lf.txt` | 8 | 8 | su LF pabaigoje |
| `struct_newline_crlf.txt` | 9 | 9 | su CRLF pabaigoje |
| `struct_len15.txt` | 15 | 15 | bloko riba: 15 baitų |
| `struct_len16.txt` | 16 | 16 | bloko riba: 16 baitų |
| `struct_len17.txt` | 17 | 17 | bloko riba: 17 baitų |
| `struct_pad_ab.txt` | 2 | 2 | užpildo patikra: 'ab' |
| `struct_pad_ab0.txt` | 3 | 3 | užpildo patikra: 'ab\0' |
| `utf8_lt.txt` | 26 | 15 | UTF-8 su ne ASCII simboliais |
| `utf8_mixed.txt` | 23 | 19 | UTF-8: brūkšnys ir euro ženklas |
