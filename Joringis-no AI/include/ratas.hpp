// Ratas-256 v0.1 – mokomoji 256 bitų maišos funkcija (be DI kurta porinės užduoties pusė).
//
// Idėja: būsena yra "ratas" iš 8 stipinų (8 x 32 bitų žodžiai = 256 bitų).
// Įvestis absorbuojama 16 baitų blokais, po kiekvieno bloko ratas "pasukamas"
// (round). Pabaigoje įmaišomas ilgis, ratas pasukamas dar kelis kartus, ir
// rezultatas nuskaitomas dviem pusėmis (po 128 bitų) su papildomais pasukimais
// tarp jų, todėl santrauka niekada nėra visa vidinė būsena.
//
// Funkcija nėra kriptografiškai analizuota – tinka tik mokymuisi.

#ifndef RATAS_HPP
#define RATAS_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace ratas {

constexpr const char* kVersion = "v0.1";

// 256 bitų santrauka = 32 baitai.
constexpr std::size_t kDigestBytes = 32;
constexpr std::size_t kDigestBits = kDigestBytes * 8;
constexpr std::size_t kDigestHexLen = kDigestBytes * 2;

using Digest = std::vector<std::uint8_t>;  // visada kDigestBytes ilgio

// Suskaičiuoja santrauką iš tikslių baitų. Tuščia įvestis leidžiama.
Digest hash(const std::uint8_t* data, std::size_t size);
Digest hash(const std::vector<std::uint8_t>& data);
Digest hash(const std::string& bytes);  // string = baitų talpykla, ne "tekstas"

// 64 mažųjų hex simbolių užrašas su visais pradiniais nuliais.
std::string to_hex(const Digest& d);

// Vidinės dalys, atvertos tik eksperimentams (difuzijos matavimui).
namespace detail {
void init_state(std::uint32_t s[8]);
void turn(std::uint32_t s[8], unsigned round);
}  // namespace detail

}  // namespace ratas

#endif
