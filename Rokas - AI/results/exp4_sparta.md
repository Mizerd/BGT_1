# 4 eksperimentas: sparta

Failas `konstitucija.txt` (bendras poros failas), ištraukos po 1, 2, 4, … eilučių su eilučių skirtukais ir visas failas. Kiekviena ištrauka paruošiama iš anksto; matuojamas tik maišos skaičiavimas (be failų I/O ir išvedimo). `std::chrono::steady_clock`, 3 apšilimo matavimai, tada 10 matavimų kiekvienam dydžiui. Viename matavime maiša kviečiama tiek kartų, kad jis truktų ≥ 20 ms, ir laikas dalijamas iš kvietimų skaičiaus. Rezultatas naudojamas (`volatile`), o maišos funkcija yra atskirame vertimo vienete, todėl kompiliatorius skaičiavimo neišmeta.

Laikas vienai maišai, µs: vidurkis (min–max).

| Eilutės | Baitai | 2 versija | 1 versija | Ratas v0.1 |
|---|---|---|---|---|
| 1 | 70 | 0,097 (0,096–0,100) | 0,024 (0,024–0,025) | 0,125 (0,125–0,127) |
| 2 | 123 | 0,113 (0,110–0,117) | 0,032 (0,031–0,032) | 0,168 (0,168–0,172) |
| 4 | 205 | 0,154 (0,153–0,157) | 0,045 (0,044–0,045) | 0,242 (0,241–0,245) |
| 8 | 362 | 0,226 (0,225–0,229) | 0,072 (0,071–0,072) | 0,386 (0,384–0,396) |
| 16 | 996 | 0,514 (0,512–0,517) | 0,179 (0,179–0,180) | 0,963 (0,959–0,973) |
| 32 | 1 841 | 0,887 (0,884–0,893) | 0,323 (0,322–0,329) | 1,731 (1,728–1,736) |
| 64 | 3 712 | 1,737 (1,730–1,757) | 0,638 (0,635–0,650) | 3,428 (3,416–3,447) |
| 128 | 9 155 | 4,194 (4,166–4,293) | 1,550 (1,548–1,553) | 8,370 (8,330–8,548) |
| 256 | 20 409 | 9,221 (9,196–9,340) | 3,443 (3,435–3,470) | 18,559 (18,491–18,760) |
| 512 | 47 434 | 21,366 (21,308–21,473) | 7,996 (7,969–8,108) | 43,015 (42,890–43,431) |
| 789 | 75 595 | 34,008 (33,921–34,065) | 12,740 (12,692–12,861) | 68,884 (68,337–69,801) |

Pralaidumas visam failui: 2 versija – 2 223 MB/s, 1 versija – 5 933 MB/s, Ratas v0.1 – 1 097 MB/s.

Neapdoroti matavimai: `raw/speed.csv` (kiekvienas matavimas, kvietimų skaičius, bendras laikas).
