#include "custom_hash.hpp"

#include <algorithm>
#include <bit>
#include <cstddef>

namespace eduhash {
namespace {

// ---------------------------------------------------------------------------
// Constants.
//
// Every constant below comes from one documented procedure unrelated to any
// existing hash function: the decimal digits of n^n (n = 2, 3, 4, ...) are
// concatenated, cut into 20 digit chunks, and each chunk is reduced modulo
// 2^64 with its lowest bit forced to 1.  A chunk is accepted only if its
// population count lies between 26 and 38.  The rotation amounts come from the
// following chunks, mapped to [9, 55] and required to be distinct.
// See README.md, "Constants".
// ---------------------------------------------------------------------------

// Starting values of the eight lanes.
constexpr std::array<std::uint64_t, 8> kLaneInit = {
    0x50EFEF418AACDDB3ull, 0x06837D22C9EC9F1Bull, 0x93ED2CF209AF0CA9ull,
    0x1511760347A610CBull, 0xE0613E1645D94F07ull, 0x7AA3E75445026145ull,
    0x0976B9398CFCD1FFull, 0x2B84EC77CA013781ull,
};

// Odd multipliers, one per sweep direction.
constexpr std::uint64_t kMulForward = 0x2AD26360F20D2A3Dull;
constexpr std::uint64_t kMulBackward = 0x65D3D764F8ED45F5ull;

// Tag constants: block position, tail length, finalization.
constexpr std::uint64_t kTagStep = 0xC5E512181F592E6Bull;
constexpr std::uint64_t kTagTail = 0x9C019A9E6A275255ull;
constexpr std::uint64_t kTagEnd = 0x0C2605DFE6C8B025ull;

// Rotation amounts: forward sweep, backward sweep, output fold.
constexpr int kRotForward = 41;
constexpr int kRotBackward = 53;
constexpr int kRotFold = 40;

constexpr std::size_t kLanes = 8;         // 512 bits of internal state
constexpr std::size_t kBlockBytes = 32;   // four 64 bit words per block
constexpr int kDrainSteps = 3;            // input-free steps before folding

using State = std::array<std::uint64_t, kLanes>;

/// Reads eight bytes as one big-endian 64 bit word.  Written with explicit
/// shifts so that the result does not depend on the byte order of the host.
std::uint64_t load_be64(const std::uint8_t* bytes) {
  std::uint64_t word = 0;
  for (std::size_t i = 0; i < 8; ++i) {
    word = (word << 8) | static_cast<std::uint64_t>(bytes[i]);
  }
  return word;
}

/// Writes one 64 bit word as eight big-endian bytes.
void store_be64(std::uint64_t word, std::uint8_t* bytes) {
  for (std::size_t i = 0; i < 8; ++i) {
    bytes[i] = static_cast<std::uint8_t>(word >> (56 - 8 * i));
  }
}

/// The single mixing step: it absorbs one block of four message words plus a
/// tag, then sweeps the eight lanes once upwards and once downwards.
///
/// The words enter four alternating lanes, using addition and XOR in turn so
/// that the entry points are not algebraically identical.  The forward sweep
/// carries everything up to lane 7, the backward sweep carries it back down to
/// lane 0, so after one step every lane depends on every other lane and on all
/// four words of the block.  The multiplications by fixed odd constants are the
/// non-linear part; the rotations feed the high bits a multiplication produces
/// back into low bit positions.  Odd multipliers keep each operation
/// invertible, so the state never collapses.
void step(State& s, std::uint64_t word0, std::uint64_t word1, std::uint64_t word2,
          std::uint64_t word3, std::uint64_t tag) {
  s[0] += word0 ^ tag;  // the position tag rides along with the first word
  s[2] ^= word1;
  s[4] += word2;
  s[6] ^= word3;

  for (std::size_t i = 1; i < kLanes; ++i) {  // upwards: lane 0 -> lane 7
    s[i] = (s[i] ^ std::rotl(s[i - 1], kRotForward)) * kMulForward;
  }
  for (std::size_t i = kLanes - 1; i-- > 0;) {  // downwards: lane 7 -> lane 0
    s[i] = (s[i] + std::rotl(s[i + 1], kRotBackward)) * kMulBackward;
  }
}

/// Stage 1: absorbs `count` whole 32 byte blocks.  Each block is tagged with
/// its position in the whole input (1, 2, 3, ...), so that the same block
/// contributes differently at different offsets.
void absorb_blocks(State& state, const std::uint8_t* data, std::size_t count, std::uint64_t first) {
  State s = state;  // a local copy stays in registers; input bytes cannot alias it
  for (std::size_t i = 0; i < count; ++i) {
    const std::uint8_t* block = data + i * kBlockBytes;
    step(s, load_be64(block), load_be64(block + 8), load_be64(block + 16),
         load_be64(block + 24), (first + i) * kTagStep);
  }
  state = s;
}

/// Stages 2-5, after all whole blocks: tail, length, drain and fold.
/// Inlined so that short inputs do not pay for a call and a state copy.
inline Digest256 finalize(State state, const std::uint8_t* rest, std::size_t rest_size, std::uint64_t blocks) {
  const std::uint64_t length = blocks * kBlockBytes + rest_size;

  // Stage 2: exactly one tail step, performed even when the input divides
  // evenly into blocks.  The leftover bytes go to the front of a zero filled
  // block; no delimiter byte is appended.  Instead the number of leftover bytes
  // is folded into the tag, which is what keeps "ab" and "ab\0" apart.
  std::uint8_t tail[kBlockBytes] = {};
  std::copy_n(rest, rest_size, tail);
  const std::uint64_t tail_tag = (blocks + 1) * kTagStep +
                                 (static_cast<std::uint64_t>(rest_size) + 1) * kTagTail;
  step(state, load_be64(tail), load_be64(tail + 8), load_be64(tail + 16),
       load_be64(tail + 24), tail_tag);

  // Stage 3: the total byte count gets its own step, so a digest always
  // depends on the exact input length.
  step(state, length, std::rotl(length, 32), 0, 0, kTagEnd);

  // Stage 4: a few more steps with no message input, so the last bytes of the
  // input are mixed as much as the first ones.  The changing tag keeps these
  // steps from being identical repetitions.
  for (int i = 1; i <= kDrainSteps; ++i) {
    step(state, 0, 0, 0, 0, kTagEnd + static_cast<std::uint64_t>(i) * kTagStep);
  }

  // Stage 5: fold the 512 bit state down to 256 bits.  Each output word pairs
  // a low lane with a rotated high lane, so the digest is a function of all
  // eight lanes while no lane is ever published on its own.
  Digest256 digest{};
  for (std::size_t i = 0; i < 4; ++i) {
    const std::uint64_t folded = state[i] ^ std::rotl(state[i + 4], kRotFold);
    store_be64(folded, digest.data() + i * 8);
  }
  return digest;
}

}  // namespace

Digest256 custom_hash(std::span<const std::uint8_t> input) {
  State state = kLaneInit;
  const std::size_t whole_blocks = input.size() / kBlockBytes;
  absorb_blocks(state, input.data(), whole_blocks, 1);
  return finalize(state, input.data() + whole_blocks * kBlockBytes, input.size() % kBlockBytes, whole_blocks);
}

Hasher::Hasher() : state_(kLaneInit) {}

void Hasher::update(std::span<const std::uint8_t> bytes) {
  if (pending_size_ > 0) {
    const std::size_t take = std::min(kBlockBytes - pending_size_, bytes.size());
    std::copy_n(bytes.begin(), take, pending_.begin() + pending_size_);
    pending_size_ += take;
    bytes = bytes.subspan(take);
    if (pending_size_ < kBlockBytes) {
      return;
    }
    absorb_blocks(state_, pending_.data(), 1, ++blocks_);
    pending_size_ = 0;
  }
  const std::size_t count = bytes.size() / kBlockBytes;
  absorb_blocks(state_, bytes.data(), count, blocks_ + 1);
  blocks_ += count;
  const std::span<const std::uint8_t> rest = bytes.subspan(count * kBlockBytes);
  std::copy(rest.begin(), rest.end(), pending_.begin());
  pending_size_ = rest.size();
}

Digest256 Hasher::finish() const { return finalize(state_, pending_.data(), pending_size_, blocks_); }

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
