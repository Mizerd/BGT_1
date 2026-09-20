#!/bin/sh
# Komandinės eilutės patikros: režimų sutapimas, atskiri paleidimai, klaidos.
# Naudojimas: tests/cli_checks.sh [build]   (numatytas katalogas: build)
# Rezultatai rašomi į results/exp3_cli.md. Reikia data/exp1 (experiments inputs).
BUILD=${1:-build}
R="$BUILD/ratas"
[ -x "$R" ] || R="$BUILD/ratas.exe"
OUT=results/exp3_cli.md
pass=0; fail=0
line() { printf '%s\n' "$1" >> "$OUT"; }
ok() { pass=$((pass+1)); line "| $1 | ok |"; }
bad() { fail=$((fail+1)); line "| $1 | **NE** |"; }

: > "$OUT"
line "# 2-3 eksperimentai. Komandinės eilutės patikros"
line ""
line "Sugeneruota \`tests/cli_checks.sh\`. Programa: \`$R\`."
line ""
line "| Patikra | Rezultatas |"
line "|---|:---:|"

# 1. --text ir --file duoda tą patį, kai baitai sutampa (be naujos eilutės).
for f in a b; do
  t=$("$R" --quiet --text "$f"); g=$("$R" --quiet --file "data/exp1/$f.bin")
  if [ "$t" = "$g" ]; then ok "--text \"$f\" == --file $f.bin"; else bad "--text \"$f\" == --file $f.bin"; fi
done
t=$("$R" --quiet --text "tekstas"); g=$("$R" --quiet --file data/exp1/struct_space_none.txt)
if [ "$t" = "$g" ]; then ok "--text \"tekstas\" == --file struct_space_none.txt"; else bad "--text tekstas == file"; fi
g2=$("$R" --quiet --file data/exp1/struct_newline_lf.txt)
if [ "$t" != "$g2" ]; then ok "--text \"tekstas\" != --file struct_newline_lf.txt (LF keičia baitus)"; else bad "LF"; fi

# 2. Rankinis įvedimas: Enter neįtraukiamas -> sutampa su --text.
m=$(printf 'tekstas\n' | "$R" --quiet)
if [ "$m" = "$t" ]; then ok "rankinis įvedimas (Enter neįtraukiamas) == --text \"tekstas\""; else bad "rankinis == --text"; fi
m=$(printf 'tekstas\r\n' | "$R" --quiet)
if [ "$m" = "$t" ]; then ok "rankinis įvedimas su CRLF == --text \"tekstas\""; else bad "rankinis CRLF"; fi

# 3. --stdin maišo tikslius baitus (su nauja eilute jie skiriasi).
s=$(printf 'tekstas' | "$R" --quiet --stdin)
if [ "$s" = "$t" ]; then ok "--stdin be LF == --text"; else bad "--stdin be LF"; fi
s=$(printf 'tekstas\n' | "$R" --quiet --stdin)
if [ "$s" = "$g2" ]; then ok "--stdin su LF == --file struct_newline_lf.txt"; else bad "--stdin su LF"; fi

# 4. Tuščia įvestis visais režimais.
e1=$("$R" --quiet --text ""); e2=$("$R" --quiet --file data/exp1/empty.bin)
e3=$(printf '' | "$R" --quiet --stdin); e4=$(printf '\n' | "$R" --quiet)
if [ "$e1" = "$e2" ] && [ "$e2" = "$e3" ] && [ "$e3" = "$e4" ]; then ok "tuščia įvestis: --text/--file/--stdin/rankinis sutampa"; else bad "tuščia įvestis"; fi
if [ ${#e1} -eq 64 ]; then ok "tuščios įvesties santrauka - 64 simboliai"; else bad "tuščios įvesties ilgis"; fi

# 5. UTF-8: argumento baitai == failo baitai.
u=$("$R" --quiet --text "ąčęėįšųūž Žąsis"); uf=$("$R" --quiet --file data/exp1/utf8_lt.txt)
if [ "$u" = "$uf" ]; then ok "--text su ne ASCII (UTF-8) == --file utf8_lt.txt"; else bad "UTF-8 --text == --file"; fi

# 6. Failo pavadinimas nesvarbus - tik turinys.
c1=$("$R" --quiet --file data/exp1/struct_space_none.txt); c2=$("$R" --quiet --file data/exp1/struct_space_none_copy.txt)
if [ "$c1" = "$c2" ]; then ok "vienodas turinys skirtingu vardu -> vienoda santrauka"; else bad "failo vardas"; fi

# 7. Atskiri paleidimai: kiekvienas failas 2 kartus, A, B, A tvarka.
all=1; n=0
for f in data/exp1/*; do
  n=$((n+1))
  x=$("$R" --quiet --file "$f"); "$R" --quiet --file data/exp1/b.bin > /dev/null; y=$("$R" --quiet --file "$f")
  [ "$x" = "$y" ] || all=0
done
if [ $all -eq 1 ]; then ok "atskiri paleidimai (A, B, A) - visi $n failai sutampa"; else bad "atskiri paleidimai"; fi

# 8. Klaidos: neegzistuojantis failas -> klaida, kodas 2, ne tuščios įvesties santrauka.
out=$("$R" --quiet --file data/exp1/nera_tokio.txt 2>/dev/null); code=$?
if [ $code -eq 2 ] && [ -z "$out" ]; then ok "neskaitomas failas -> klaida, išėjimo kodas 2, be santraukos"; else bad "neskaitomas failas (kodas $code)"; fi
"$R" --blogas >/dev/null 2>&1; code=$?
if [ $code -eq 1 ]; then ok "blogi argumentai -> išėjimo kodas 1"; else bad "blogi argumentai (kodas $code)"; fi

line ""
line "Praėjo: $pass, nepavyko: $fail."
echo "CLI patikros: praejo $pass, nepavyko $fail (zr. $OUT)"
[ $fail -eq 0 ]
