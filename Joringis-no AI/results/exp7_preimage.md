# 7 eksperimentas. Spėjimas, vieša druska ir slaptas atsitiktinumas

Kandidatų rinkinys: visos keturių skaitmenų eilutės `0000`..`9999` (10 000). Tikslinė įvestis pasirinkta iš
anksto; atakai duodama tik jos santrauka (ir druska, kai ji vieša). Maiša: Ratas v0.1.

## 1. Be druskos: H(input)

- Tikslinė santrauka: `1b4d25b6b31e57f06829abe6beaa925de27609b1f0f06c982004a592543ec2ba`
- Bandymų: 10000 (visas rinkinys perrinktas), pirmas sutapimas ties bandymu nr. 7392
- Laikas: 1002 µs (0.10 µs vienam kandidatui)
- Sutampantys kandidatai: `7391` 

Sutapimas nebūtinai identifikuoja pradinę įvestį: rasta tik įvestis iš kandidatų rinkinio, kurios
santrauka sutampa. Tai gali būti ir kita įvestis su ta pačia santrauka (kolizija), tačiau 256 bitų
maišai su 10 000 kandidatų atsitiktinė kolizija itin mažai tikėtina, todėl praktiškai tai ta pati įvestis.

## 2. Vieša druska: H(input || salt)

- Druska: 8 atsitiktiniai baitai (seed 20260927), pridedami PO įvesties kaip tikslūs baitai (ne hex tekstas).
  Užrašoma hex: `9e2a81f058112dc5`. Užpuolikas druską žino.
- Tikslinė santrauka: `c272314e0eb3f514196c06873c5e472a361d1af239e5a152364b88900df76b7b`
- Bandymų: 10000, laikas: 1133 µs, sutapimai: `7391` 
- Pastangos vienam taikiniui: tokios pat kaip be druskos (10 000 maišų) – vieša druska nuo perrinkimo neapsaugo.
- Iš anksto apskaičiuota 10 000 įrašų lentelė su druska `9e2a81f058112dc5` pritaikyta taikiniui su kita druska `54f05f26478c9d31`: nerasta. Kiekvienai druskai lentelę reikia skaičiuoti iš naujo, todėl druska naikina pakartotinį naudojimą (rainbow tables), bet ne perrinkimą.

## 3. Slaptas atsitiktinumas: H(input || r), r nežinomas

- r – 2 slapti baitai (65 536 galimybių). Paieškos erdvė: 10 000 × 65 536 = 655 360 000 porų (input, r).
- Perrinkta 1/64 r erdvės: 10240000 bandymų per 1130.4 ms, tikslas nerastas; visa erdvė užtruktų apie 72.3 s (65 536 kartų daugiau nei be r).
- Su 16 slaptų baitų r (2^128) perrinkimas būtų praktiškai neįmanomas – r paslepia lengvai spėjamą įvestį.
- Atskleidus r = `da19`, patikrinimas yra viena maiša: H(`7391` || r) == tikslas -> taip.

Tai iliustruoja įsipareigojimo (commitment) idėją: paskelbus H(input || r) įvestis lieka paslėpta, o vėliau atskleidus
(input, r) kiekvienas gali ją patikrinti. Tai NEĮRODO, kad Ratas-256 saugiai slepia pranešimą ar neleidžia rasti kitos
poros (input', r') su ta pačia santrauka – tam reiktų kriptografinės analizės.
