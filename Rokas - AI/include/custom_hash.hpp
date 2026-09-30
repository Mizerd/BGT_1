// 256-bit hash function - public interface.
//
// The core routine works on raw bytes only. It does not know or care whether
// those bytes came from a command line argument, a text file or a binary file.

#ifndef CUSTOM_HASH_HPP
#define CUSTOM_HASH_HPP

#include <array>
#include <cstddef>
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

/// The same hash for input that arrives in pieces, e.g. a large file read in
/// chunks.  Any split of the same bytes gives the same digest as custom_hash.
class Hasher {
 public:
  Hasher();

  /// Adds the next piece of input; pieces may have any size, including 0.
  void update(std::span<const std::uint8_t> bytes);

  /// Digest of everything added so far.  The hasher itself is not changed.
  Digest256 finish() const;

 private:
  std::array<std::uint64_t, 8> state_;
  std::array<std::uint8_t, 32> pending_{};  // start of an unfinished block
  std::size_t pending_size_ = 0;
  std::uint64_t blocks_ = 0;                // whole blocks absorbed so far
};

/// Renders a digest as exactly 64 lowercase hexadecimal characters,
/// leading zeroes included.
std::string to_hex(const Digest256& digest);

}  // namespace eduhash

#endif  // CUSTOM_HASH_HPP
