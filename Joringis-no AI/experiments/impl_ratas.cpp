// Adapteris: sujungia Ratas-256 su bendra eksperimentų sąsaja (impl.hpp).
//
// Eksperimentų programa (`Rokas - AI/experiments/experiments.cpp`) yra bendra
// abiem poros realizacijoms ir naudojama nepakeista, kad palyginimo sąlygos
// sutaptų: tie patys seed'ai, ta pati abėcėlė, tie patys imčių dydžiai.
//
// `ratas.cpp` įtraukiamas tiesiogiai ir NEKEIČIAMAS - toks, koks pažymėtas
// v0.1. Kad nesusidurtų dvi `main` funkcijos, komandinės eilutės `main` čia
// laikinai pervadinamas. Taip visoje užduotyje lieka viena vienintelė
// algoritmo kopija, kuri negali prasilenkti su v0.1 programa.

#define main ratas_cli_main_unused
#include "../ratas.cpp"
#undef main

#include <algorithm>

#include "impl.hpp"

namespace impl {

const char* const kName = "beDI";

Digest hash(const std::uint8_t* data, std::size_t size) {
    const std::vector<std::uint8_t> out = ratas256(data, size);
    Digest d{};
    std::copy(out.begin(), out.end(), d.begin());
    return d;
}

std::string to_hex(const Digest& digest) {
    return ::to_hex(std::vector<std::uint8_t>(digest.begin(), digest.end()));
}

}  // namespace impl
