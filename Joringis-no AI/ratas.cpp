// Ratas-256 v0.1 - mokomoji 256 bitu maisos funkcija.
// Porinio darbo pusė, kurta be DI pagalbos.
// Idėja: būsena yra "ratas" iš 8 stipinų (8 x 32 bitų žodžiai = 256 bitų).
// Įvestis absorbuojama 16 baitų blokais, po kiekvieno bloko ratas pasukamas.
// Pabaigoje įmaišomas įvesties ilgis, ratas pasukamas dar kelis kartus, ir santrauka nuskaitoma dviem pusėmis po 128 bitus su pasukimu tarp jų, todėl santrauka niekada nėra visa vidinė būsena.
// Funkcija nėra kriptografiškai analizuota - tinka tik mokymuisi.

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

namespace {

constexpr size_t kSpokes = 8;        // 8 x 32 bitai = 256 bitų būsena
constexpr size_t kBlockBytes = 16;   // 4 x 32 bitų žodžiai per bloką
constexpr size_t kDigestBytes = 32;  // 256 bitai = 32 baitai = 64 hex

// Pasukimų skaičiai: po kiekvieno bloko sukama 2 kartus, pabaigoje daugiau, kad vieno įvesties bito pokytis spėtų pasklisti per visą būseną.
constexpr int kRoundsPerBlock = 2;
constexpr int kRoundsFinal = 4;
constexpr int kRoundsSqueeze = 2;

// Pradinė būsena - 32 baitų frazė, skaitoma kaip 8 mažojo galo 32 bitų žodžiai.
constexpr char kSeedPhrase[kSpokes * 4 + 1] = "Vilniaus universitetas, BGT 2026";

using State = uint32_t[kSpokes];

uint32_t rotl32(uint32_t x, unsigned n) {
    n &= 31u;
    if (n == 0) return x;
    return (x << n) | (x >> (32u - n));
}

uint32_t load_le32(const uint8_t* b) {
    return static_cast<uint32_t>(b[0]) |
           (static_cast<uint32_t>(b[1]) << 8) |
           (static_cast<uint32_t>(b[2]) << 16) |
           (static_cast<uint32_t>(b[3]) << 24);
}

void store_le32(uint32_t w, uint8_t* b) {
    b[0] = static_cast<uint8_t>(w);
    b[1] = static_cast<uint8_t>(w >> 8);
    b[2] = static_cast<uint8_t>(w >> 16);
    b[3] = static_cast<uint8_t>(w >> 24);
}

constexpr uint32_t seed_word(size_t i) {
    return static_cast<uint32_t>(static_cast<unsigned char>(kSeedPhrase[4 * i])) |
           (static_cast<uint32_t>(static_cast<unsigned char>(kSeedPhrase[4 * i + 1])) << 8) |
           (static_cast<uint32_t>(static_cast<unsigned char>(kSeedPhrase[4 * i + 2])) << 16) |
           (static_cast<uint32_t>(static_cast<unsigned char>(kSeedPhrase[4 * i + 3])) << 24);
}

constexpr uint32_t kIV[kSpokes] = {
    seed_word(0), seed_word(1), seed_word(2), seed_word(3),
    seed_word(4), seed_word(5), seed_word(6), seed_word(7),
};

// Pasukimo konstanta: r-tasis pradinės būsenos žodis, padaugintas iš nelyginio skaičiaus (2r+1) ir pastumtas per r, kad pasukimai nesikartotų simetriškai.
uint32_t round_const(unsigned r) {
    return kIV[r % kSpokes] * (2u * r + 1u) + r;
}

// Vienas rato pasukimas. Kiekvienas stipinas i atnaujinamas pagal stipinus i+1, i+3 ir i+6. Stipinas i+6 parenka ir posūkio dydį - tai pagrindinis netiesiškumo šaltinis šalia sudėties moduliu 2^32. 
// Naujas stipinas iškart perduodamas kitam (i+1), kad pokytis per vieną pasukimą apeitų visą ratą.
void turn(State& s, unsigned r) {
    const uint32_t rc = round_const(r);
    for (size_t i = 0; i < kSpokes; ++i) {
        const uint32_t b = s[(i + 1) % kSpokes];
        const uint32_t c = s[(i + 3) % kSpokes];
        const uint32_t d = s[(i + 6) % kSpokes];
        uint32_t a = s[i];
        a += (b ^ c);
        a = rotl32(a, d);
        a ^= (d + rc);
        s[i] = a;
        s[(i + 1) % kSpokes] += rotl32(a, 9);
    }
}

void turns(State& s, int count, unsigned& counter) {
    for (int k = 0; k < count; ++k) turn(s, counter++);
}

// 16 baitų bloko įmaišymas: keturi žodžiai XOR'inami į stipinus 0..3, bloko eilės numeris pridedamas prie stipino 7, po to ratas pasukamas.
void absorb_block(State& s, const uint8_t* block, uint32_t block_index,
                  unsigned& counter) {
    s[0] ^= load_le32(block);
    s[1] ^= load_le32(block + 4);
    s[2] ^= load_le32(block + 8);
    s[3] ^= load_le32(block + 12);
    s[7] += block_index;
    turns(s, kRoundsPerBlock, counter);
}

// Pagrindinė maišos funkcija: tikslūs baitai -> 32 baitų santrauka.
vector<uint8_t> ratas256(const uint8_t* data, size_t size) {
    State s;
    for (size_t i = 0; i < kSpokes; ++i) s[i] = kIV[i];
    unsigned counter = 0;  // pasukimų numeratorius per visą skaičiavimą

    // 1. Pilni blokai.
    const size_t full_blocks = size / kBlockBytes;
    uint32_t index = 1;
    for (size_t i = 0; i < full_blocks; ++i, ++index)
        absorb_block(s, data + i * kBlockBytes, index, counter);

    // 2. Paskutinis blokas su užpildu: trūkstamų baitų skaičius n (1..16) įrašomas n kartų (kaip PKCS#7).
    //    Jei ilgis dalijasi iš 16, pridedamas visas blokas iš 16 baitų 0x10, todėl skirtingo ilgio įvestys niekada nesutampa po užpildymo.
    uint8_t last[kBlockBytes];
    const size_t rest = size - full_blocks * kBlockBytes;
    const uint8_t pad = static_cast<uint8_t>(kBlockBytes - rest);
    for (size_t i = 0; i < kBlockBytes; ++i)
        last[i] = (i < rest) ? data[full_blocks * kBlockBytes + i] : pad;
    absorb_block(s, last, index, counter);

    // 3. Ilgio (baitais, 64 bitai) įmaišymas ir uždarymo žymė.
    const uint64_t len = static_cast<uint64_t>(size);
    s[4] ^= static_cast<uint32_t>(len);
    s[5] ^= static_cast<uint32_t>(len >> 32);
    s[6] ^= 0xFFFFFFFFu;
    turns(s, kRoundsFinal, counter);

    // 4. Rezultatas dviem pusėmis: stipinai 0..3, pasukimai, vėl stipinai 0..3.
    vector<uint8_t> out(kDigestBytes);
    for (size_t i = 0; i < 4; ++i) store_le32(s[i], &out[4 * i]);
    turns(s, kRoundsSqueeze, counter);
    for (size_t i = 0; i < 4; ++i) store_le32(s[i], &out[16 + 4 * i]);
    return out;
}

// 64 mažųjų hex simboliai su visais pradiniais nuliais.
string to_hex(const vector<uint8_t>& d) {
    static const char* digits = "0123456789abcdef";
    string s;
    s.reserve(d.size() * 2);
    for (uint8_t b : d) {
        s.push_back(digits[b >> 4]);
        s.push_back(digits[b & 0x0F]);
    }
    return s;
}

// Failas skaitomas dvejetainiu režimu, be jokio eilučių pabaigų keitimo.
// Neatsidaręs ar nepilnai perskaitytas failas -> false (ne tuščia įvestis).
bool read_file(const string& path, vector<uint8_t>& bytes) {
    ifstream f(path, ios::binary);
    if (!f) return false;
    char buf[1 << 16];
    while (f.read(buf, sizeof(buf)) || f.gcount() > 0)
        bytes.insert(bytes.end(), buf, buf + f.gcount());
    return !f.bad();
}

// Rankinis įvedimas: viena eilutė iki Enter. Enter sukurtas naujos eilutės simbolis į maišą NEĮTRAUKIAMAS.
// Windows konsolėje skaitoma UTF-16 ir verčiama į UTF-8, kad ne ASCII raidės būtų maišomos kaip UTF-8 baitai.
string read_line() {
#ifdef _WIN32
    HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode = 0;
    if (GetConsoleMode(h, &mode)) {
        wstring w;
        wchar_t buf[512];
        DWORD got = 0;
        while (ReadConsoleW(h, buf, 512, &got, nullptr) && got > 0) {
            w.append(buf, got);
            if (w.back() == L'\n') break;
        }
        while (!w.empty() && (w.back() == L'\n' || w.back() == L'\r')) w.pop_back();
        if (w.empty()) return string();
        const int n = WideCharToMultiByte(CP_UTF8, 0, w.data(), static_cast<int>(w.size()),
                                          nullptr, 0, nullptr, nullptr);
        string s(static_cast<size_t>(n), 0);
        WideCharToMultiByte(CP_UTF8, 0, w.data(), static_cast<int>(w.size()),
                            &s[0], n, nullptr, nullptr);
        return s;
    }
#endif
    string line;
    if (!getline(cin, line)) line.clear();
    if (!line.empty() && line.back() == '\r') line.pop_back();
    return line;
}
void hold_console() {
#ifdef _WIN32
    DWORD pids[2];
    if (GetConsoleProcessList(pids, 2) == 1) {
        cout << "\nSpauskite Enter, kad uzdarytumete langa...";
        cin.get();
    }
#endif
}

// Režimai: be argumentų - tekstas įvedamas ranka; su vienu argumentu - to failo TURINIO (ne pavadinimo) maiša.
// Išėjimo kodai: 0 - pavyko, 1 - blogi argumentai, 2 - failo neperskaitė.
int run(int argc, char** argv) {
    vector<uint8_t> bytes;
    string mode;

    if (argc == 2) {
        // Failas nurodytas komandinės eilutės argumentu (arba užtempus jį ant .exe).
        mode = string("failas: ") + argv[1];
        if (!read_file(argv[1], bytes)) {
            cerr << "Klaida: nepavyko perskaityti failo: " << argv[1] << '\n';
            return 2;
        }
    } else if (argc == 1) {
        // Paleista be argumentų (pvz., dvigubu spustelėjimu) - klausiame režimo.
        cout << "Pasirinkite rezima:\n"
                "  1 - ivesti teksta ranka\n"
                "  2 - maisyti faila\n"
                "Pasirinkimas: ";
        const string choice = read_line();

        if (choice == "1") {
            mode = "rankinis ivedimas";
            cout << "Iveskite teksta ir spauskite Enter (Enter neitraukiamas):\n";
            const string line = read_line();
            bytes.assign(line.begin(), line.end());
        } else if (choice == "2") {
            cout << "Failo kelias: ";
            string path = read_line();
            // Nuimame kabutes, jei kelias nukopijuotas per Explorer "Copy as path".
            if (path.size() >= 2 && path.front() == '"' && path.back() == '"')
                path = path.substr(1, path.size() - 2);
            mode = "failas: " + path;
            if (!read_file(path, bytes)) {
                cerr << "Klaida: nepavyko perskaityti failo: " << path << '\n';
                return 2;
            }
        } else {
            cerr << "Klaida: reikia pasirinkti 1 arba 2\n";
            return 1;
        }
    } else {
        cerr << "Naudojimas: ratas [failas]\n";
        return 1;
    }

    cout << "Rezimas: " << mode << '\n'
         << "Ivesties baitu: " << bytes.size() << '\n'
         << "Ratas-256: " << to_hex(ratas256(bytes.data(), bytes.size())) << '\n';
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    const int code = run(argc, argv);
    hold_console();
    return code;
}
