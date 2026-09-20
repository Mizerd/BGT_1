// Educational 256-bit hash function - public interface.
//
// The core routine works on raw bytes only. It does not know or care whether
// those bytes came from a command line argument, a text file or a binary file.

#ifndef CUSTOM_HASH_HPP
#define CUSTOM_HASH_HPP

#include <array>
#include <cstdint>
#include <span>
#include <string>

namespace eduhash {

/// A fixed 256-bit (32 byte) digest.
using Digest256 = std::array<std::uint8_t, 32>;

/// Hashes an arbitrary byte sequence, including the empty sequence.
/// The result is deterministic, depends on every input byte, on their order
/// and on the exact input length, and is independent of host endianness.
Digest256 custom_hash(std::span<const std::uint8_t> input);

/// Renders a digest as exactly 64 lowercase hexadecimal characters,
/// leading zeroes included.
std::string to_hex(const Digest256& digest);

}  // namespace eduhash

#endif  // CUSTOM_HASH_HPP
