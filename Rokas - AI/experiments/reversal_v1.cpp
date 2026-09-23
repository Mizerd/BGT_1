// Recovers messages of up to 15 bytes from a version 1 digest (256-bit state,
// every step invertible, whole state published). Linked against the real
// version 1 source so the targets are genuine version 1 digests.

#include <array>
#include <bit>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <optional>
#include <random>
#include <string>
#include <vector>

#include "custom_hash.hpp"

namespace {

using Bytes = std::vector<std::uint8_t>;

constexpr std::uint64_t kInit[4] = {0x50EFEF418AACDDB3ull, 0x06837D22C9EC9F1Bull, 0x93ED2CF209AF0CA9ull,
                                    0x1511760347A610CBull};
constexpr std::uint64_t kMulB = 0xE0613E1645D94F07ull;
constexpr std::uint64_t kMulD = 0x7AA3E75445026145ull;
constexpr std::uint64_t kTagStep = 0x0976B9398CFCD1FFull;
constexpr std::uint64_t kTagTail = 0x2B84EC77CA013781ull;
constexpr std::uint64_t kTagEnd = 0x2AD26360F20D2A3Dull;

struct State {
  std::uint64_t a, b, c, d;
};

constexpr std::uint64_t inverse(std::uint64_t m) {
  std::uint64_t x = m;
  for (int i = 0; i < 6; ++i) x *= 2 - m * x;
  return x;
}
constexpr std::uint64_t kInvB = inverse(kMulB);
constexpr std::uint64_t kInvD = inverse(kMulD);

// Exact inverse of one version 1 mixing step with known words and tag.
void unmix(State& s, std::uint64_t w0, std::uint64_t w1, std::uint64_t tag) {
  s.d -= std::rotl(s.a, 35);
  s.b ^= std::rotl(s.c, 17);
  s.a -= s.d;
  s.d = (s.d * kInvD) ^ s.c;
  s.c = (std::rotr(s.c, 10) - s.b) ^ w1;
  s.b = (s.b * kInvB) ^ s.a;
  s.a = std::rotr(s.a, 55) - (w0 ^ tag);
}

std::uint64_t load(const eduhash::Digest256& d, int i) {
  std::uint64_t v = 0;
  for (int k = 0; k < 8; ++k) v = (v << 8) | d[static_cast<std::size_t>(8 * i + k)];
  return v;
}

std::optional<Bytes> recover(const eduhash::Digest256& digest) {
  for (std::uint64_t len = 0; len < 16; ++len) {
    State s{load(digest, 0), load(digest, 1), load(digest, 2), load(digest, 3)};
    for (std::uint64_t j = 3; j >= 1; --j) unmix(s, 0, 0, kTagEnd + j * kTagStep);
    unmix(s, len, std::rotl(len, 32), kTagEnd);

    // Tail step with unknown words: the two lanes that never see a word must
    // come out equal to the known initial values.
    const std::uint64_t tag = kTagStep + (len + 1) * kTagTail;
    const std::uint64_t d1 = s.d - std::rotl(s.a, 35);
    const std::uint64_t b1 = s.b ^ std::rotl(s.c, 17);
    const std::uint64_t a2 = s.a - d1;
    if (((d1 * kInvD) ^ s.c) != kInit[3] || ((b1 * kInvB) ^ a2) != kInit[1]) continue;
    const std::uint64_t w1 = (std::rotr(s.c, 10) - b1) ^ kInit[2];
    const std::uint64_t w0 = (std::rotr(a2, 55) - kInit[0]) ^ tag;

    Bytes block(16);
    for (int k = 0; k < 8; ++k) {
      block[static_cast<std::size_t>(k)] = static_cast<std::uint8_t>(w0 >> (56 - 8 * k));
      block[static_cast<std::size_t>(8 + k)] = static_cast<std::uint8_t>(w1 >> (56 - 8 * k));
    }
    bool padded = true;
    for (std::size_t k = len; k < 16; ++k) padded = padded && block[k] == 0;
    if (!padded) continue;
    block.resize(len);
    if (eduhash::custom_hash(block) == digest) return block;
  }
  return std::nullopt;
}

std::string printable(const Bytes& b) {
  std::string s;
  char hex[5];
  for (std::uint8_t c : b) {
    if (c >= 0x20 && c < 0x7F && c != ',') {
      s += static_cast<char>(c);
    } else {
      std::snprintf(hex, sizeof(hex), "\\x%02x", c);
      s += hex;
    }
  }
  return s;
}

}  // namespace

int main() {
  const std::vector<std::string> named = {"", "a", "abc", "hello", "Lietuva", "0000", "4821", "slaptažodis", "123456789012345"};
  for (const auto& m : named) {
    const Bytes msg(m.begin(), m.end());
    const eduhash::Digest256 digest = eduhash::custom_hash(msg);
    const auto got = recover(digest);
    std::printf("example,%s,%s,%s\n", printable(msg).c_str(), eduhash::to_hex(digest).c_str(),
                got ? printable(*got).c_str() : "(nepavyko)");
  }

  std::mt19937_64 rng(20260920 + 8);
  constexpr int kTrials = 10000;
  int recovered = 0;
  double total_us = 0;
  for (int i = 0; i < kTrials; ++i) {
    Bytes msg(rng() % 16);
    for (auto& c : msg) c = static_cast<std::uint8_t>(rng());
    const eduhash::Digest256 digest = eduhash::custom_hash(msg);
    const auto t0 = std::chrono::steady_clock::now();
    const auto got = recover(digest);
    total_us += std::chrono::duration<double, std::micro>(std::chrono::steady_clock::now() - t0).count();
    recovered += got && *got == msg;
  }
  std::printf("summary,%d,%d,%.2f\n", kTrials, recovered, total_us / kTrials);
  return 0;
}
