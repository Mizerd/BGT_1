# Ratas-256 — v0.1

Mokomoji 256 bitų maišos funkcija (BGT 1 užduotis, porinio darbo pusė be DI pagalbos).
Visas kodas yra viename faile `ratas.cpp`.

Funkcija nėra kriptografiškai analizuota ir netinka slaptažodžiams, pinigams
ar realioms sistemoms — tik mokymuisi.

## Kompiliavimas

**Visual Studio 2022.** Atidaryk `ratas.sln`, pasirink konfigūraciją `Release`
ir platformą `x64`, spausk Ctrl+F5. Rezultatas — `x64\Release\ratas.exe`.

Arba iš komandinės eilutės:

```
:: „x64 Native Tools Command Prompt for VS 2022“
cl /nologo /std:c++17 /O2 /EHsc /W4 /utf-8 ratas.cpp /Fe:ratas.exe
```

g++ (Linux / macOS / MSYS2):

```
g++ -std=c++17 -O2 -Wall -Wextra -o ratas ratas.cpp
```

## Paleidimas

Programą galima paleisti dvigubu pelės spustelėjimu, iš Visual Studio (F5 arba
Ctrl+F5) arba iš terminalo. 

Failo vardą Visual Studio perduosi per
*Project → Properties → Debugging → Command Arguments*.

Du režimai. Naudojamas režimas visada atspausdinamas.

| Režimas | Ką maišo |
|---|---|
| rankinis įvedimas | ranka įvestą tekstą (viena eilutė iki Enter) |
| failas | failo **turinį**, o ne pavadinimą |

Failą galima nurodyti komandinės eilutės argumentu:

```
ratas pavyzdys.txt
```

Tą patį padaro failo užtempimas ant `ratas.exe` — Explorer perduoda jo kelią
kaip argumentą.

Paleidus be argumento (pvz., dvigubu spustelėjimu) programa pirmiausia paklausia
režimo:

```
Pasirinkite rezima:
  1 - ivesti teksta ranka
  2 - maisyti faila
Pasirinkimas: 2
Failo kelias: pavyzdys.txt
```

Pasirinkus `2` įvedamas failo kelias; jei failas yra šalia `ratas.exe`, užtenka
vardo. Aplink kelią esančios kabutės (Explorer „Copy as path“) nuimamos.

Bandymams kataloge yra `pavyzdys.txt`:

```
$ ratas pavyzdys.txt
Rezimas: failas: pavyzdys.txt
Ivesties baitu: 38
Ratas-256: 380246be72d35ab5d5c3be1f5c5cc03f49032d6515f4988951e971ee37259adf
```

Jame yra `Labas, Lietuva! Ąžuolas prie ežero.` — 35 simbolių, bet 38 baitai, nes
`Ą` ir abu `ž` UTF-8 koduotėje užima po du baitus. Failas baigiasi be naujos
eilutės simbolio, todėl tą pačią eilutę įvedus ranka gaunama ta pati maiša.

Išėjimo kodai: `0` — pavyko, `1` — blogi argumentai arba neteisingas režimo
pasirinkimas, `2` — failo nepavyko perskaityti. Neperskaitytas failas yra
klaida, o ne tuščia įvestis.

## Įvestis ir išvestis

- **Kodavimas.** Ranka įvestas tekstas koduojamas UTF-8 (Windows konsolė
  perskaitoma UTF-16 ir konvertuojama). Failas skaitomas dvejetainiu režimu —
  maišomi tikslūs jo baitai, eilučių pabaigos nekeičiamos.
- **Enter.** Ranka įvedant naujos eilutės simbolis **neįtraukiamas**. Todėl
  `ratas` ir `ratas failas.txt` duoda tą pačią maišą tik tada, kai faile nėra
  eilutės pabaigos simbolio.
- **Jokio normalizavimo.** Tarpai nešalinami, raidžių registras ir Unicode
  forma nekeičiami.
- **Išvestis.** 256 bitai = 32 baitai = 64 hex skaitmenys, mažosiomis raidėmis,
  su visais pradiniais nuliais.
- **Dydžio riba.** Visa įvestis sudedama į atmintį, todėl praktinė riba —
  laisva operatyvioji atmintis. Ilgis maišomas kaip 64 bitų skaičius.

Tuščia įvestis leidžiama: 0 baitų → `25bf155273a700862f8583fec8712c06367d2fd6eb4234279b59c593300d98d2`.

## Algoritmo idėja

Būsena — „ratas“ iš 8 stipinų (8 × 32 bitų žodžiai = 256 bitai). Įvestis
absorbuojama po 16 baitų, po kiekvieno bloko ratas pasukamas. Rezultatas
nuskaitomas dviem pusėmis su pasukimu tarp jų, todėl santrauka niekada nėra
visa vidinė būsena.

```
būsena[0..7] = frazė "Vilniaus universitetas, BGT 2026" kaip 8 LE žodžiai

kiekvienam pilnam 16 baitų blokui:
    būsena[0..3] ^= bloko žodžiai
    būsena[7]    += bloko numeris
    sukti 2 kartus

paskutinis blokas užpildomas PKCS#7 būdu ir absorbuojamas taip pat

būsena[4] ^= ilgis (žemieji 32 bitai)
būsena[5] ^= ilgis (aukštieji 32 bitai)
būsena[6] ^= 0xFFFFFFFF
sukti 4 kartus

santrauka = būsena[0..3];  sukti 2 kartus;  santrauka += būsena[0..3]
```

Vienas pasukimas (`turn`), kiekvienam stipinui `i`:

```
b = būsena[i+1];  c = būsena[i+3];  d = būsena[i+6]
a = būsena[i] + (b ^ c)
a = rotl(a, d)            // posūkio dydis priklauso nuo duomenų
a = a ^ (d + konstanta(r))
būsena[i]   = a
būsena[i+1] += rotl(a, 9)  // pokytis iškart perduodamas toliau
```

Sprendimų pagrindimas:

- **Nuo duomenų priklausantis posūkis** (`rotl(a, d)`) — pagrindinis
  netiesiškumo šaltinis šalia sudėties moduliu 2³².
- **Naujas stipinas iškart perduodamas kitam**, todėl vieno bito pokytis per
  vieną pasukimą apeina visą ratą.
- **Užpildymas ir ilgio įmaišymas** — kad skirtingo ilgio įvestys nesutaptų.
- **Savos konstantos** iš frazės, o ne paimtos iš žinomos maišos funkcijos.
