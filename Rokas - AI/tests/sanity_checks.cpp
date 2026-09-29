// Minimal correctness checks for the hashing core.
//
// These are deliberately small: they establish that the implementation runs,
// is deterministic, produces the right shape of output and reacts to input
// changes.  The statistical experiments (collisions, avalanche, benchmarks)
// belong to a later stage of the assignment and are not implemented here.

#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <iostream>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "custom_hash.hpp"

namespace {

int failures = 0;
int checks = 0;

void check(bool condition, const std::string& description) {
  ++checks;
  if (!condition) {
    ++failures;
    std::cout << "FAIL  " << description << '\n';
  } else {
    std::cout << "ok    " << description << '\n';
  }
}

eduhash::Digest256 hash_of(std::string_view text) {
  const auto* first = reinterpret_cast<const std::uint8_t*>(text.data());
  return eduhash::custom_hash({first, text.size()});
}

std::string hex_of(std::string_view text) { return eduhash::to_hex(hash_of(text)); }

bool is_lowercase_hex(const std::string& text) {
  for (const char c : text) {
    const bool digit = c >= '0' && c <= '9';
    const bool lower = c >= 'a' && c <= 'f';
    if (!digit && !lower) {
      return false;
    }
  }
  return true;
}

}  // namespace

int main() {
  // Accepted inputs, including the empty one.
  const std::vector<std::string> inputs = {"", "a", "b", "hello", "Hello", "abc", "cba"};
  for (const std::string& input : inputs) {
    const eduhash::Digest256 digest = hash_of(input);
    const std::string hex = eduhash::to_hex(digest);
    const std::string label = input.empty() ? std::string("<empty>") : input;
    check(digest.size() == 32, "digest of \"" + label + "\" is 32 bytes");
    check(hex.size() == 64, "digest of \"" + label + "\" is 64 hex characters");
    check(is_lowercase_hex(hex), "digest of \"" + label + "\" is lowercase hex");
  }

  // Determinism inside one process: hash A, B, A.
  const std::string first_a = hex_of("A");
  const std::string only_b = hex_of("B");
  const std::string second_a = hex_of("A");
  check(first_a == second_a, "hashing A, B, A gives identical results for A");
  check(first_a != only_b, "A and B give different results");

  // Repeating the same input many times must not drift.
  bool stable = true;
  const std::string reference = hex_of("stability");
  for (int i = 0; i < 1000; ++i) {
    stable = stable && (hex_of("stability") == reference);
  }
  check(stable, "1000 repeated calls return the same digest");

  // Input sensitivity.
  check(hex_of("hello") != hex_of("Hello"), "hello differs from Hello");
  check(hex_of("abc") != hex_of("cba"), "abc differs from cba (order matters)");
  check(hex_of("hello") != hex_of("hello\n"), "hello differs from hello with newline");
  check(hex_of("") != hex_of(std::string_view("\0", 1)), "empty differs from one zero byte");
  check(hex_of(std::string_view("a\0", 2)) != hex_of("a"), "trailing zero byte changes the digest");

  // Every input byte is used, including bytes far inside a long input and the
  // very last byte of a block-aligned input.
  const std::string long_input(1000, 'x');
  std::string long_changed = long_input;
  long_changed[500] = 'y';
  check(hex_of(long_input) != hex_of(long_changed), "a change in the middle of a long input matters");
  const std::string aligned(32, 'z');
  std::string aligned_changed = aligned;
  aligned_changed[31] = 'Z';
  check(hex_of(aligned) != hex_of(aligned_changed), "the last byte of a block-aligned input matters");

  // Binary input, including bytes that are not printable ASCII.
  std::vector<std::uint8_t> binary(256);
  for (std::size_t i = 0; i < binary.size(); ++i) {
    binary[i] = static_cast<std::uint8_t>(i);
  }
  const std::string binary_hex = eduhash::to_hex(eduhash::custom_hash(binary));
  check(binary_hex.size() == 64 && is_lowercase_hex(binary_hex), "all 256 byte values hash to 64 hex characters");
  std::vector<std::uint8_t> binary_swapped = binary;
  std::swap(binary_swapped[0], binary_swapped[255]);
  check(eduhash::to_hex(eduhash::custom_hash(binary_swapped)) != binary_hex,
        "reordering binary bytes changes the digest");

  // Length is incorporated: inputs that differ only in length must differ.
  check(hex_of(std::string(16, '\0')) != hex_of(std::string(17, '\0')),
        "16 zero bytes differ from 17 zero bytes");

  // Block boundary lengths, and changes at the beginning, middle and end.
  auto pattern = [](std::size_t n) {
    std::string text;
    for (std::size_t i = 0; i < n; ++i) {
      text.push_back(static_cast<char>('A' + (i % 26)));
    }
    return text;
  };
  std::vector<std::string> boundary_digests;
  for (const std::size_t length : {15u, 16u, 17u, 31u, 32u, 33u}) {
    const std::string base = pattern(length);
    const std::string base_hex = hex_of(base);
    const std::string label = std::to_string(length) + " byte input";
    check(base_hex.size() == 64 && is_lowercase_hex(base_hex), label + " hashes to 64 lowercase hex characters");
    boundary_digests.push_back(base_hex);

    std::string at_start = base, at_middle = base, at_end = base;
    at_start[0] = static_cast<char>(at_start[0] ^ 0x01);
    at_middle[length / 2] = static_cast<char>(at_middle[length / 2] ^ 0x01);
    at_end[length - 1] = static_cast<char>(at_end[length - 1] ^ 0x01);
    const std::string start_hex = hex_of(at_start);
    const std::string middle_hex = hex_of(at_middle);
    const std::string end_hex = hex_of(at_end);
    check(start_hex != base_hex && middle_hex != base_hex && end_hex != base_hex,
          label + ": a flipped bit at the start, middle and end each change the digest");
    check(start_hex != middle_hex && middle_hex != end_hex && start_hex != end_hex,
          label + ": the three single-bit changes give three different digests");
  }
  bool boundary_distinct = true;
  for (std::size_t i = 0; i < boundary_digests.size(); ++i) {
    for (std::size_t j = i + 1; j < boundary_digests.size(); ++j) {
      boundary_distinct = boundary_distinct && (boundary_digests[i] != boundary_digests[j]);
    }
  }
  check(boundary_distinct, "the six boundary lengths give six different digests");

  // Known answers, computed with an independent Python reimplementation.
  // Any accidental change to the algorithm or output format breaks these.
  std::string pattern1000;
  for (int i = 0; i < 1000; ++i) pattern1000.push_back(static_cast<char>((i * 7 + 3) & 0xff));
  std::string all_bytes;
  for (int i = 0; i < 256; ++i) all_bytes.push_back(static_cast<char>(i));
  const std::vector<std::pair<std::string, std::string>> known = {
      {"", "547ac2e87baff5183c537ca0a12efbcd18a385a59f3c1b39c6e332388b34350c"},
      {"a", "9572b1ea73ece20fd3cb0c6b79a1f24a62e3b7b4871b0c19d2e210061eb2d980"},
      {"abc", "060f1c0f305405e02d5faa3d12af3e4f326e5aad3fe8097b357614a4c12b55b0"},
      {"hello", "2607ba4e2a9bd8521178536b84dffc11bf933871f0e95eeca4357c2e43936c54"},
      {std::string(31, 'x'), "c024d9d1a379ec5e0435cef26fc1cef761803b2c5c98e894e8b58dcf44b73ee6"},
      {std::string(32, 'x'), "faddd9ac674668d2f8e201ddfe440dfc7b45c7ffad8589ad37db7e70201b4d76"},
      {std::string(33, 'x'), "6a928d2364a27401cffb911635b2ef9416ca870f0c22a386541d987c657d82f8"},
      {"tekstas\r\n", "d07259291e5a06212f9861cfa48a417883f264c63f14ddebb76d3a7328476841"},
      {all_bytes, "c98c398ab567d02e9d56854894a6dd2c16d91b01ac80de02264f2bfd685ef69e"},
      {pattern1000, "158e7e70f832db35bd5e5229abf7ea0984f3857585ae3ca474dfa5577b25878b"},
  };
  bool all_known = true;
  for (const auto& [input, expected] : known) {
    all_known = all_known && hex_of(input) == expected;
  }
  check(all_known, "10 known-answer vectors match the reference implementation");

  // Leading zero bytes and nibbles must survive hex formatting.
  eduhash::Digest256 zeros{};
  zeros[1] = 0x0a;
  zeros[31] = 0x01;
  check(eduhash::to_hex(zeros) == std::string("000a") + std::string(58, '0') + "01",
        "to_hex keeps leading zero bytes and nibbles");

  // Every single-byte input gets its own digest.
  std::vector<std::string> single;
  for (int i = 0; i < 256; ++i) single.push_back(hex_of(std::string(1, static_cast<char>(i))));
  std::sort(single.begin(), single.end());
  check(std::adjacent_find(single.begin(), single.end()) == single.end(), "all 256 one-byte inputs give distinct digests");

  // Structural illustration of the change made in this revision.  The digest is
  // a fold of a 512 bit state, output[i] = lane[i] XOR rotl(lane[i + 4], 40),
  // so many internal states share one digest.  Previously the state was 256
  // bits and was written out unchanged, which meant a digest named the final
  // state exactly and the computation could be unwound from it directly.  This
  // check only shows that the fold is many-to-one; it says nothing about how
  // hard it is to find an input for a given digest.
  auto fold = [](const std::array<std::uint64_t, 8>& lanes) {
    std::array<std::uint64_t, 4> out{};
    for (std::size_t i = 0; i < 4; ++i) {
      out[i] = lanes[i] ^ std::rotl(lanes[i + 4], 40);
    }
    return out;
  };
  const std::array<std::uint64_t, 8> state_one = {1, 2, 3, 4, 5, 6, 7, 8};
  std::array<std::uint64_t, 8> state_two = state_one;
  state_two[4] = 0x0123456789ABCDEFull;
  state_two[0] = state_one[0] ^ std::rotl(state_one[4], 40) ^ std::rotl(state_two[4], 40);
  check(state_one != state_two && fold(state_one) == fold(state_two),
        "the 512 to 256 bit fold maps different internal states onto one digest");

  std::cout << '\n'
            << (failures == 0 ? "ALL CHECKS PASSED" : "SOME CHECKS FAILED") << "  ("
            << (checks - failures) << "/" << checks << ")\n";
  return failures == 0 ? 0 : 1;
}
