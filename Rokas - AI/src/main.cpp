// Minimal command line front end.  It only collects bytes and prints a digest;
// all hashing logic lives in custom_hash.cpp.

#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "custom_hash.hpp"

namespace {

void print_usage(std::ostream& out, const char* program) {
  out << "usage:\n"
      << "  " << program << " --text \"<string>\"   hash the exact bytes of the argument\n"
      << "  " << program << " --file <path>        hash the exact contents of a file\n";
}

/// Reads a whole file in binary mode.  Returns false if the file cannot be
/// opened or if reading fails part way through, so that a failure is never
/// mistaken for an empty input.
bool read_file_bytes(const std::string& path, std::vector<std::uint8_t>& bytes) {
  std::ifstream file(path, std::ios::binary);
  if (!file) {
    return false;
  }
  char buffer[4096];
  while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
    bytes.insert(bytes.end(), buffer, buffer + file.gcount());
    if (!file) {
      break;
    }
  }
  return !file.bad();
}

}  // namespace

int main(int argc, char** argv) {
  const char* program = (argc > 0 && argv[0] != nullptr) ? argv[0] : "hash-generator";

  if (argc == 2) {
    const std::string_view option = argv[1];
    if (option == "--help" || option == "-h") {
      print_usage(std::cout, program);
      return 0;
    }
  }

  if (argc != 3) {
    print_usage(std::cerr, program);
    return 1;
  }

  const std::string_view mode = argv[1];

  if (mode == "--text") {
    // The argument bytes are hashed exactly as the shell delivered them:
    // nothing is trimmed, re-cased, normalised, or terminated with a newline.
    const std::string_view text = argv[2];
    const auto* first = reinterpret_cast<const std::uint8_t*>(text.data());
    std::cout << eduhash::to_hex(eduhash::custom_hash({first, text.size()})) << '\n';
    return 0;
  }

  if (mode == "--file") {
    std::vector<std::uint8_t> bytes;
    if (!read_file_bytes(argv[2], bytes)) {
      std::cerr << "error: cannot read file: " << argv[2] << '\n';
      return 2;
    }
    std::cout << eduhash::to_hex(eduhash::custom_hash(bytes)) << '\n';
    return 0;
  }

  print_usage(std::cerr, program);
  return 1;
}
