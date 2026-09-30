// Quick correctness checks for the hash core. Statistics (collisions, avalanche,
// speed) are in experiments/.

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

dihash::Digest256 hash_of(std::string_view text) {
  const auto* first = reinterpret_cast<const std::uint8_t*>(text.data());
  return dihash::custom_hash({first, text.size()});
}

std::string hex_of(std::string_view text) { return dihash::to_hex(hash_of(text)); }

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
  const std::vector<std::string> inputs = {"", "a", "b", "hello", "Hello", "abc", "cba"};
  for (const std::string& input : inputs) {
    const dihash::Digest256 digest = hash_of(input);
    const std::string hex = dihash::to_hex(digest);
    const std::string label = input.empty() ? std::string("<empty>") : input;
    check(digest.size() == 32, "digest of \"" + label + "\" is 32 bytes");
    check(hex.size() == 64, "digest of \"" + label + "\" is 64 hex characters");
    check(is_lowercase_hex(hex), "digest of \"" + label + "\" is lowercase hex");
  }

  // A, B, A catches state leaking from one call into the next.
  const std::string first_a = hex_of("A");
  const std::string only_b = hex_of("B");
  const std::string second_a = hex_of("A");
  check(first_a == second_a, "hashing A, B, A gives identical results for A");
  check(first_a != only_b, "A and B give different results");

  bool stable = true;
  const std::string reference = hex_of("stability");
  for (int i = 0; i < 1000; ++i) {
    stable = stable && (hex_of("stability") == reference);
  }
  check(stable, "1000 repeated calls return the same digest");

  check(hex_of("hello") != hex_of("Hello"), "hello differs from Hello");
  check(hex_of("abc") != hex_of("cba"), "abc differs from cba (order matters)");
  check(hex_of("hello") != hex_of("hello\n"), "hello differs from hello with newline");
  check(hex_of("") != hex_of(std::string_view("\0", 1)), "empty differs from one zero byte");
  check(hex_of(std::string_view("a\0", 2)) != hex_of("a"), "trailing zero byte changes the digest");

  // The middle of a long input and the last byte of a full block must count too.
  const std::string long_input(1000, 'x');
  std::string long_changed = long_input;
  long_changed[500] = 'y';
  check(hex_of(long_input) != hex_of(long_changed), "a change in the middle of a long input matters");
  const std::string aligned(32, 'z');
  std::string aligned_changed = aligned;
  aligned_changed[31] = 'Z';
  check(hex_of(aligned) != hex_of(aligned_changed), "the last byte of a block-aligned input matters");

  std::vector<std::uint8_t> binary(256);
  for (std::size_t i = 0; i < binary.size(); ++i) {
    binary[i] = static_cast<std::uint8_t>(i);
  }
  const std::string binary_hex = dihash::to_hex(dihash::custom_hash(binary));
  check(binary_hex.size() == 64 && is_lowercase_hex(binary_hex), "all 256 byte values hash to 64 hex characters");
  std::vector<std::uint8_t> binary_swapped = binary;
  std::swap(binary_swapped[0], binary_swapped[255]);
  check(dihash::to_hex(dihash::custom_hash(binary_swapped)) != binary_hex,
        "reordering binary bytes changes the digest");

  check(hex_of(std::string(16, '\0')) != hex_of(std::string(17, '\0')),
        "16 zero bytes differ from 17 zero bytes");

  // Lengths around the 32-byte block boundary.
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

  // Known answers from the Python version in tests/reference_check.py. They
  // change only if the algorithm changes.
  std::string pattern1000;
  for (int i = 0; i < 1000; ++i) pattern1000.push_back(static_cast<char>((i * 7 + 3) & 0xff));
  std::string all_bytes;
  for (int i = 0; i < 256; ++i) all_bytes.push_back(static_cast<char>(i));
  const std::vector<std::pair<std::string, std::string>> known = {
      {"", "f83958be8ca002bca110726b8b5c768ebc1a8cc5a2f2e183538c783707d97286"},
      {"a", "23c85b745d9b90784895ef9c56457094ed1f5fb0bf9a07bca618013466a8f59d"},
      {"abc", "a881978cbef8990b0868ec299003e871d8523ccd3f7cfb8f9bd667b568f476bf"},
      {"hello", "f89c7a5af119d3d76f90f84157cecbaa50ddb5dd2a5458eceee37c3c88375bf1"},
      {std::string(31, 'x'), "2c8dc9996b724fd4b022b16190491cd746f9753d01229b8ce187caa420ed107e"},
      {std::string(32, 'x'), "3eebed151c765901c76a2d79b87650de3c40c6ab4f85a8f84e9e35d8e5472573"},
      {std::string(33, 'x'), "6abd69d10fd08e6881d586509647dab2aebbc27e1b69d7c355117c10001d5c2a"},
      {"tekstas\r\n", "8736b5476001cd12c126359e77b61219cd347627eea1529da385cf7e02eef487"},
      {all_bytes, "acacaec70fc5d4592aac3830823a43cf7ef7c632a5e9a81d2f9f34de8f03be45"},
      {pattern1000, "64409476c8f77de470535c5fbff999c1ae8e63fd33ff53d04d8d5c5b441793e4"},
  };
  bool all_known = true;
  for (const auto& [input, expected] : known) {
    all_known = all_known && hex_of(input) == expected;
  }
  check(all_known, "10 known-answer vectors match the reference implementation");

  dihash::Digest256 zeros{};
  zeros[1] = 0x0a;
  zeros[31] = 0x01;
  check(dihash::to_hex(zeros) == std::string("000a") + std::string(58, '0') + "01",
        "to_hex keeps leading zero bytes and nibbles");

  std::vector<std::string> single;
  for (int i = 0; i < 256; ++i) single.push_back(hex_of(std::string(1, static_cast<char>(i))));
  std::sort(single.begin(), single.end());
  check(std::adjacent_find(single.begin(), single.end()) == single.end(), "all 256 one-byte inputs give distinct digests");

  // Hasher must give the same digest however the input is split.
  auto streamed = [](std::string_view text, const std::vector<std::size_t>& pieces) {
    dihash::Hasher hasher;
    std::size_t at = 0;
    for (std::size_t i = 0; at < text.size(); ++i) {
      const std::string_view part = text.substr(at, pieces[i % pieces.size()]);
      hasher.update({reinterpret_cast<const std::uint8_t*>(part.data()), part.size()});
      at += part.size();
    }
    return dihash::to_hex(hasher.finish());
  };
  bool two_pieces = true;
  for (std::size_t n = 0; n <= 100; ++n) {
    const std::string text = pattern1000.substr(0, n);
    for (std::size_t k = 0; k <= n; ++k) {
      two_pieces = two_pieces && streamed(text, {k, n}) == hex_of(text);
    }
  }
  check(two_pieces, "Hasher: every split of 0-100 byte inputs into two pieces matches custom_hash");
  bool fixed_pieces = true;
  for (std::size_t piece = 1; piece <= 70; ++piece) {
    fixed_pieces = fixed_pieces && streamed(pattern1000, {piece}) == hex_of(pattern1000);
  }
  check(fixed_pieces, "Hasher: 1000 bytes fed in pieces of 1-70 bytes match custom_hash");
  std::string big;
  for (std::size_t i = 0; i < 100000; ++i) big.push_back(static_cast<char>((i * 131 + 7) % 251));
  std::vector<std::size_t> random_pieces;
  for (std::uint64_t x = 1; random_pieces.size() < 200;) {
    x = x * 6364136223846793005ull + 1442695040888963407ull;
    random_pieces.push_back(static_cast<std::size_t>(x >> 33) % 1500);  // includes empty pieces
  }
  check(streamed(big, random_pieces) == hex_of(big), "Hasher: 100 000 bytes in irregular pieces (0-1499 B) match custom_hash");
  dihash::Hasher hasher;
  const std::string first_part = "hello, ", second_part = "world";
  hasher.update({reinterpret_cast<const std::uint8_t*>(first_part.data()), first_part.size()});
  const bool same_twice = dihash::to_hex(hasher.finish()) == hex_of(first_part) &&
                          dihash::to_hex(hasher.finish()) == hex_of(first_part);
  hasher.update({reinterpret_cast<const std::uint8_t*>(second_part.data()), second_part.size()});
  check(same_twice && dihash::to_hex(hasher.finish()) == hex_of(first_part + second_part),
        "Hasher: finish() leaves the hasher unchanged, more input can follow");
  check(dihash::to_hex(dihash::Hasher().finish()) == hex_of(""), "Hasher: no input gives the empty-input digest");

  // The 512 -> 256 fold is many-to-one: a digest does not pin down the state.
  // (It says nothing about how hard it is to find an input for a digest.)
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
