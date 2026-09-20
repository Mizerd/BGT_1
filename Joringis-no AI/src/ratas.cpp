// Ratas-256 v0.1 realizacija. Žr. include/ratas.hpp ir README.md.

#include "ratas.hpp"

namespace ratas {
namespace {

constexpr std::size_t kSpokes = 8;       // 8 x 32 bitai = 256 bitų būsena
constexpr std::size_t kBlockBytes = 16;  // 4 x 32 bitų žodžiai per bloką

// Pasukimų skaičiai. Vienas pasukimas vieno bito skirtumą paskleidžia tik
// per ~38 % būsenos bitų, du – per ~50 % (žr. README, difuzijos matavimas),
// todėl po kiekvieno bloko sukama 2 kartus, o pabaigoje – dvigubai daugiau.
constexpr int kRoundsPerBlock = 2;  // po kiekvieno 16 baitų bloko
constexpr int kRoundsFinal = 4;     // įmaišius ilgį, prieš pirmą rezultato pusę
constexpr int kRoundsSqueeze = 2;   // tarp pirmos ir antros rezultato pusės

// Pradinė būsena – 32 baitų frazė, skaitoma kaip 8 mažojo galo (little-endian)
// 32 bitų žodžiai. Frazė pasirinkta tam, kad konstantos būtų aiškiai savos ir
// lengvai patikrinamos, o ne nukopijuotos iš kokios nors žinomos funkcijos.
constexpr char kSeedPhrase[kSpokes * 4 + 1] = "Vilniaus universitetas, BGT 2026";

using State = std::uint32_t[kSpokes];

inline std::uint32_t rotl32(std::uint32_t x, unsigned n) {
    n &= 31u;
    if (n == 0) return x;
    return (x << n) | (x >> (32u - n));
}

inline std::uint32_t load_le32(const std::uint8_t* b) {
    return static_cast<std::uint32_t>(b[0]) |
           (static_cast<std::uint32_t>(b[1]) << 8) |
           (static_cast<std::uint32_t>(b[2]) << 16) |
           (static_cast<std::uint32_t>(b[3]) << 24);
}

inline void store_le32(std::uint32_t w, std::uint8_t* b) {
    b[0] = static_cast<std::uint8_t>(w);
    b[1] = static_cast<std::uint8_t>(w >> 8);
    b[2] = static_cast<std::uint8_t>(w >> 16);
    b[3] = static_cast<std::uint8_t>(w >> 24);
}

// Frazės i-tasis žodis (mažojo galo tvarka), suskaičiuojamas kompiliuojant.
constexpr std::uint32_t seed_word(std::size_t i) {
    return static_cast<std::uint32_t>(static_cast<unsigned char>(kSeedPhrase[4 * i])) |
           (static_cast<std::uint32_t>(static_cast<unsigned char>(kSeedPhrase[4 * i + 1])) << 8) |
           (static_cast<std::uint32_t>(static_cast<unsigned char>(kSeedPhrase[4 * i + 2])) << 16) |
           (static_cast<std::uint32_t>(static_cast<unsigned char>(kSeedPhrase[4 * i + 3])) << 24);
}

constexpr std::uint32_t kIV[kSpokes] = {
    seed_word(0), seed_word(1), seed_word(2), seed_word(3),
    seed_word(4), seed_word(5), seed_word(6), seed_word(7),
};

void init_state(State& s) {
    for (std::size_t i = 0; i < kSpokes; ++i) s[i] = kIV[i];
}


// Pasukimo (round) konstanta: r-tasis pradinės būsenos žodis, padaugintas iš
// nelyginio skaičiaus (2r+1) ir pastumtas per r. Skirtingi pasukimai gauna
// skirtingas konstantas, todėl vienodos būsenos nesikartoja simetriškai.
inline std::uint32_t round_const(unsigned r) {
    return kIV[r % kSpokes] * (2u * r + 1u) + r;
}

// Vienas rato pasukimas. Kiekvienas stipinas i atnaujinamas pagal tris kitus
// stipinus (i+1, i+3, i+6). Stipinas i+6 parenka ir posūkio dydį (nuo duomenų
// priklausantis rotate), tai pagrindinis netiesiškumo šaltinis šalia sudėties
// moduliu 2^32. Naujas stipinas iškart perduodamas kitam (i+1), kad pokytis
// per vieną pasukimą apeitų visą ratą.
void turn(State& s, unsigned r) {
    const std::uint32_t rc = round_const(r);
    for (std::size_t i = 0; i < kSpokes; ++i) {
        const std::uint32_t b = s[(i + 1) % kSpokes];
        const std::uint32_t c = s[(i + 3) % kSpokes];
        const std::uint32_t d = s[(i + 6) % kSpokes];
        std::uint32_t a = s[i];
        a += (b ^ c);
        a = rotl32(a, d);          // posūkio dydis = d mod 32
        a ^= (d + rc);
        s[i] = a;
        s[(i + 1) % kSpokes] += rotl32(a, 9);
    }
}

void turns(State& s, int count, unsigned& counter) {
    for (int k = 0; k < count; ++k) turn(s, counter++);
}

// 16 baitų bloko įmaišymas: keturi žodžiai XOR'inami į stipinus 0..3, bloko
// eilės numeris pridedamas prie stipino 7, po to ratas pasukamas.
void absorb_block(State& s, const std::uint8_t* block, std::uint32_t block_index,
                  unsigned& counter) {
    s[0] ^= load_le32(block);
    s[1] ^= load_le32(block + 4);
    s[2] ^= load_le32(block + 8);
    s[3] ^= load_le32(block + 12);
    s[7] += block_index;
    turns(s, kRoundsPerBlock, counter);
}

}  // namespace

// Vidiniai žingsniai atverti eksperimentams; pati maiša naudoja juos pačius.
namespace detail {
void init_state(std::uint32_t s[8]) { ratas::init_state(*reinterpret_cast<State*>(s)); }
void turn(std::uint32_t s[8], unsigned round) { ratas::turn(*reinterpret_cast<State*>(s), round); }
}  // namespace detail

Digest hash(const std::uint8_t* data, std::size_t size) {
    State s;
    init_state(s);
    unsigned counter = 0;  // pasukimų numeratorius per visą skaičiavimą

    // 1. Pilni blokai.
    const std::size_t full_blocks = size / kBlockBytes;
    std::uint32_t index = 1;
    for (std::size_t i = 0; i < full_blocks; ++i, ++index)
        absorb_block(s, data + i * kBlockBytes, index, counter);

    // 2. Paskutinis blokas su užpildu. Trūkstamų baitų skaičius n (1..16)
    //    įrašomas n kartų (kaip PKCS#7). Jei įvestis dalijasi iš 16, pridedamas
    //    visas blokas iš 16 baitų 0x10. Todėl "ab" ir "ab\0" užpildomi skirtingai.
    std::uint8_t last[kBlockBytes];
    const std::size_t rest = size - full_blocks * kBlockBytes;
    const std::uint8_t pad = static_cast<std::uint8_t>(kBlockBytes - rest);
    for (std::size_t i = 0; i < kBlockBytes; ++i)
        last[i] = (i < rest) ? data[full_blocks * kBlockBytes + i] : pad;
    absorb_block(s, last, index, counter);

    // 3. Ilgio įmaišymas (baitais, 64 bitai) ir uždarymo žymė.
    const std::uint64_t len = static_cast<std::uint64_t>(size);
    s[4] ^= static_cast<std::uint32_t>(len);
    s[5] ^= static_cast<std::uint32_t>(len >> 32);
    s[6] ^= 0xFFFFFFFFu;
    turns(s, kRoundsFinal, counter);

    // 4. Rezultatas dviem pusėmis: stipinai 0..3, pasukimai, vėl stipinai 0..3.
    Digest out(kDigestBytes);
    for (std::size_t i = 0; i < 4; ++i) store_le32(s[i], &out[4 * i]);
    turns(s, kRoundsSqueeze, counter);
    for (std::size_t i = 0; i < 4; ++i) store_le32(s[i], &out[16 + 4 * i]);
    return out;
}

Digest hash(const std::vector<std::uint8_t>& data) {
    return hash(data.data(), data.size());
}

Digest hash(const std::string& bytes) {
    return hash(reinterpret_cast<const std::uint8_t*>(bytes.data()), bytes.size());
}

std::string to_hex(const Digest& d) {
    static const char* digits = "0123456789abcdef";
    std::string s;
    s.reserve(d.size() * 2);
    for (std::uint8_t b : d) {
        s.push_back(digits[b >> 4]);
        s.push_back(digits[b & 0x0F]);
    }
    return s;
}

}  // namespace ratas
