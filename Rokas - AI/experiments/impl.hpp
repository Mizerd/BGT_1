#ifndef IMPL_HPP
#define IMPL_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

// What an implementation must provide to be run by experiments.cpp.
namespace impl {

// 32 bytes unless built for a shorter digest (MD5: 16, SHA-1: 20).
#ifndef DIGEST_BYTES
#define DIGEST_BYTES 32
#endif
using Digest = std::array<std::uint8_t, DIGEST_BYTES>;

extern const char* const kName;
Digest hash(const std::uint8_t* data, std::size_t size);
std::string to_hex(const Digest& digest);

}  // namespace impl

#endif  // IMPL_HPP
