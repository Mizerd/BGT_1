#include "custom_hash.hpp"

#include <bit>
#include <cstddef>

namespace eduhash {
namespace {

// ---------------------------------------------------------------------------
// Constants.
//
// Every constant below was produced by one documented procedure that has
// nothing to do with any existing hash function: the decimal digits of the
// sequence n^n (n = 2, 3, 4, ...) are concatenated into one long decimal
// string, cut into consecutive 20 digit chunks, and each chunk is reduced
// modulo 2^64.  A chunk is accepted as a 64 bit word only if, after forcing
// the lowest bit to 1, its population count lies between 26 and 38.  The
// rotation amounts come from the chunks that follow, mapped to the range
// [9, 55] and required to be distinct.  See README.md, "Constant derivation".
// ---------------------------------------------------------------------------

// Starting values of the four lanes.
constexpr std::uint64_t kLaneInit0 = 0x50EFEF418AACDDB3ull;
constexpr std::uint64_t kLaneInit1 = 0x06837D22C9EC9F1Bull;
constexpr std::uint64_t kLaneInit2 = 0x93ED2CF209AF0CA9ull;
constexpr std::uint64_t kLaneInit3 = 0x1511760347A610CBull;

// Odd multipliers used by the two non-linear chaining steps.
constexpr std::uint64_t kMulSecond = 0xE0613E1645D94F07ull;
constexpr std::uint64_t kMulFourth = 0x7AA3E75445026145ull;

// Tag constants: block position, tail length and finalization.
constexpr std::uint64_t kTagStep = 0x0976B9398CFCD1FFull;
constexpr std::uint64_t kTagTail = 0x2B84EC77CA013781ull;
constexpr std::uint64_t kTagEnd = 0x2AD26360F20D2A3Dull;

// Rotation amounts.
constexpr int kRotFirst = 55;
constexpr int kRotThird = 10;
constexpr int kRotCross1 = 17;
constexpr int kRotCross2 = 35;

// One absorbed block is two 64 bit words.
constexpr std::size_t kBlockBytes = 16;

// Mixing steps performed after the length injection, with no further input.
constexpr int kDrainSteps = 3;

/// The four 64 bit lanes that make up the whole internal state.
struct State {
  std::uint64_t a;
  std::uint64_t b;
  std::uint64_t c;
  std::uint64_t d;
};

/// Reads eight bytes as one big-endian 64 bit word.  Written with explicit
/// shifts so that the result does not depend on the byte order of the host.
std::uint64_t load_be64(const std::uint8_t* bytes) {
  std::uint64_t word = 0;
  for (std::size_t i = 0; i < 8; ++i) {
    word = (word << 8) | static_cast<std::uint64_t>(bytes[i]);
  }
  return word;
}

/// Writes one 64 bit word as eight big-endian bytes, again with explicit
/// shifts rather than a reinterpreting cast.
void store_be64(std::uint64_t word, std::uint8_t* bytes) {
  for (std::size_t i = 0; i < 8; ++i) {
    bytes[i] = static_cast<std::uint8_t>(word >> (56 - 8 * i));
  }
}

/// The single mixing step of the design: it absorbs two message words plus a
/// tag and drives them once around the lane ring A -> B -> C -> D -> A.
///
/// The two message words enter at two different places (A and C) so that a
/// block is never simply added into a private accumulator.  The two multiply
/// steps are the non-linear part; the rotations move the high bits produced by
/// a multiplication back down to low bit positions, where the next addition
/// and multiplication can spread them again.  The last two lines are the extra
/// cross links that make every lane depend on every other lane within a single
/// step.  Each individual operation is invertible, so the step never collapses
/// distinct states onto each other.
void mix(State& s, std::uint64_t word0, std::uint64_t word1, std::uint64_t tag) {
  s.a += word0 ^ tag;                      // first word enters lane A
  s.c ^= word1;                            // second word enters lane C
  s.a = std::rotl(s.a, kRotFirst);
  s.b = (s.b ^ s.a) * kMulSecond;          // A -> B
  s.c += s.b;                              // B -> C
  s.c = std::rotl(s.c, kRotThird);
  s.d = (s.d ^ s.c) * kMulFourth;          // C -> D
  s.a += s.d;                              // D -> A closes the ring
  s.b ^= std::rotl(s.c, kRotCross1);       // B also sees C
  s.d += std::rotl(s.a, kRotCross2);       // D also sees the closed ring
}

}  // namespace

Digest256 custom_hash(std::span<const std::uint8_t> input) {
  State state{kLaneInit0, kLaneInit1, kLaneInit2, kLaneInit3};

  const std::uint64_t length = static_cast<std::uint64_t>(input.size());
  const std::size_t whole_blocks = input.size() / kBlockBytes;
  const std::size_t tail_size = input.size() % kBlockBytes;

  // Stage 1: every whole 16 byte block, tagged with its own position so that
  // the same block contributes differently at different offsets.
  for (std::size_t i = 0; i < whole_blocks; ++i) {
    const std::uint8_t* block = input.data() + i * kBlockBytes;
    const std::uint64_t position = static_cast<std::uint64_t>(i) + 1;
    mix(state, load_be64(block), load_be64(block + 8), position * kTagStep);
  }

  // Stage 2: exactly one tail step, performed even when the input divides
  // evenly into blocks.  The leftover bytes are placed at the front of a zero
  // filled block; no delimiter byte is appended.  Instead the number of
  // leftover bytes is folded into the tag, which is what keeps "ab" and
  // "ab\0" apart.
  std::uint8_t tail[kBlockBytes] = {};
  for (std::size_t i = 0; i < tail_size; ++i) {
    tail[i] = input[whole_blocks * kBlockBytes + i];
  }
  const std::uint64_t tail_position = static_cast<std::uint64_t>(whole_blocks) + 1;
  const std::uint64_t tail_tag = tail_position * kTagStep +
                                 (static_cast<std::uint64_t>(tail_size) + 1) * kTagTail;
  mix(state, load_be64(tail), load_be64(tail + 8), tail_tag);

  // Stage 3: the total byte count is injected as a pair of words with its own
  // constant tag, so a digest always depends on the exact input length.
  mix(state, length, std::rotl(length, 32), kTagEnd);

  // Stage 4: a few more steps with no message input, so that the last bytes of
  // the input are mixed as thoroughly as the first ones.  The tag keeps
  // changing, otherwise these steps would all be identical.
  for (int step = 1; step <= kDrainSteps; ++step) {
    mix(state, 0, 0, kTagEnd + static_cast<std::uint64_t>(step) * kTagStep);
  }

  // Stage 5: the four lanes are written out big-endian, most significant byte
  // first, which fixes the digest byte order on every platform.
  Digest256 digest{};
  store_be64(state.a, digest.data() + 0);
  store_be64(state.b, digest.data() + 8);
  store_be64(state.c, digest.data() + 16);
  store_be64(state.d, digest.data() + 24);
  return digest;
}

std::string to_hex(const Digest256& digest) {
  static constexpr char kHexDigits[] = "0123456789abcdef";
  std::string text;
  text.reserve(digest.size() * 2);
  for (const std::uint8_t byte : digest) {
    text.push_back(kHexDigits[byte >> 4]);
    text.push_back(kHexDigits[byte & 0x0F]);
  }
  return text;
}

}  // namespace eduhash
