#define main ratas_cli_main_unused
#include "../ratas.cpp"
#undef main

#include <algorithm>

#include "impl.hpp"

namespace impl {

const char* const kName = "ratas";

Digest hash(const std::uint8_t* data, std::size_t size) {
    const std::vector<std::uint8_t> out = ratas256(data, size);
    Digest d{};
    std::copy(out.begin(), out.end(), d.begin());
    return d;
}

std::string to_hex(const Digest& digest) {
    return ::to_hex(std::vector<std::uint8_t>(digest.begin(), digest.end()));
}

}
