# 1 versijos silpnybė: pranešimo atkūrimas iš maišos

1 versijoje būsena buvo 256 bitų ir visa išvedama kaip maiša, o kiekvienas maišymo žingsnis yra apverčiamas. Iki 15 baitų įvestis patenka į vieną likučio žingsnį, todėl iš maišos galima skaičiuoti atgal: atspėjamas ilgis (16 variantų), atšaukiami 3 tušti žingsniai ir ilgio žingsnis, o likučio žingsnyje nežinomi tik du 64 bitų žodžiai. Dvi būsenos dalys, į kurias žodžiai nepatenka, turi sutapti su žinomomis pradinėmis reikšmėmis – tai patikrina spėjimą, o likusios dvi tiesiog atiduoda abu žodžius. Taikinio maišos gautos tikra 1 versijos realizacija (commit `b3d54be`).

| Pranešimas | 1 versijos maiša | Atkurta iš maišos |
|---|---|---|
| (tuščias) | `5326818c0e46b7dbf9bb0d7b…` | (tuščias) |
| `a` | `2bc84d34e106017e7c84af27…` | `a` |
| `abc` | `d55f48f8b07edab915337bba…` | `abc` |
| `hello` | `dc39127a9e7914e3de985472…` | `hello` |
| `Lietuva` | `5296e5eacd160a12c094567e…` | `Lietuva` |
| `0000` | `b2f9ac0855062dfe82d67ad9…` | `0000` |
| `4821` | `4de5a8b08e774c97fcb781a5…` | `4821` |
| `slaptažodis` | `0432bbac412e0c4886783688…` | `slaptažodis` |
| `123456789012345` | `b4fec548412b1fbc815f09b9…` | `123456789012345` |

10 000 atsitiktinių 0–15 baitų pranešimų (bet kokios baitų reikšmės): atkurta **10 000/10 000**, vidutiniškai 0,09 µs vienam. Tai ne perrinkimas, o tiesioginis skaičiavimas.

2 versijoje tas pats kelias neprasideda: maiša yra 512 bitų būsenos sulenkimas į 256 bitus, todėl skaičiavimui atgal trūksta 256 būsenos bitų (2^256 variantų). Tai pašalina šį konkretų atkūrimo kelią, bet neįrodo atsparumo pirmavaizdžio paieškai.
