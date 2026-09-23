#ifndef IMPL_HPP
#define IMPL_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

// Common interface so every experiment runs unchanged against each implementation.
namespace impl {

using Digest = std::array<std::uint8_t, 32>;

extern const char* const kName;
Digest hash(const std::uint8_t* data, std::size_t size);
std::string to_hex(const Digest& digest);

}  // namespace impl

#endif  // IMPL_HPP
