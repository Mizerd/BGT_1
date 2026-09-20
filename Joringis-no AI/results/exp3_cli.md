# 2-3 eksperimentai. Komandinės eilutės patikros

Sugeneruota `tests/cli_checks.sh`. Programa: `build/ratas`.

| Patikra | Rezultatas |
|---|:---:|
| --text "a" == --file a.bin | ok |
| --text "b" == --file b.bin | ok |
| --text "tekstas" == --file struct_space_none.txt | ok |
| --text "tekstas" != --file struct_newline_lf.txt (LF keičia baitus) | ok |
| rankinis įvedimas (Enter neįtraukiamas) == --text "tekstas" | ok |
| rankinis įvedimas su CRLF == --text "tekstas" | ok |
| --stdin be LF == --text | ok |
| --stdin su LF == --file struct_newline_lf.txt | ok |
| tuščia įvestis: --text/--file/--stdin/rankinis sutampa | ok |
| tuščios įvesties santrauka - 64 simboliai | ok |
| --text su ne ASCII (UTF-8) == --file utf8_lt.txt | ok |
| vienodas turinys skirtingu vardu -> vienoda santrauka | ok |
| atskiri paleidimai (A, B, A) - visi 34 failai sutampa | ok |
| neskaitomas failas -> klaida, išėjimo kodas 2, be santraukos | ok |
| blogi argumentai -> išėjimo kodas 1 | ok |

Praėjo: 15, nepavyko: 0.
