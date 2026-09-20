// Minimal correctness checks for the hashing core.
//
// These are deliberately small: they establish that the implementation runs,
// is deterministic, produces the right shape of output and reacts to input
// changes.  The statistical experiments (collisions, avalanche, benchmarks)
// belong to a later stage of the assignment and are not implemented here.

#include <cstdint>
#include <iostream>
#include <span>
#include <string>
#include <string_view>
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

  std::cout << '\n'
            << (failures == 0 ? "ALL CHECKS PASSED" : "SOME CHECKS FAILED") << "  ("
            << (checks - failures) << "/" << checks << ")\n";
  return failures == 0 ? 0 : 1;
}
