#include <cstdint>
#include <fstream>
#include <iostream>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "custom_hash.hpp"

namespace {

void print_usage(std::ostream& out, const char* program) {
  out << "Naudojimas:\n"
      << "  " << program << "                     tekstas įvedamas ranka, Enter baigia įvestį\n"
      << "  " << program << " --text \"<tekstas>\"  maišomi argumento baitai\n"
      << "  " << program << " --file <kelias>     maišomas failo turinys\n";
}

// Reads the whole file in binary mode; false on open or read failure.
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

void print_digest(std::span<const std::uint8_t> bytes) {
  std::cout << eduhash::to_hex(eduhash::custom_hash(bytes)) << '\n';
}

std::span<const std::uint8_t> as_bytes(std::string_view text) {
  return {reinterpret_cast<const std::uint8_t*>(text.data()), text.size()};
}

}  // namespace

int main(int argc, char** argv) {
  const char* program = (argc > 0 && argv[0] != nullptr) ? argv[0] : "hash-generator";

  if (argc == 1) {
    // One typed line; the newline produced by Enter is not part of the input.
    std::cerr << "Režimas: ranka įvestas tekstas (Enter neįtraukiamas)\nĮveskite tekstą: ";
    std::string line;
    if (!std::getline(std::cin, line)) {
      std::cerr << "\nklaida: nepavyko nuskaityti įvesties\n";
      return 2;
    }
    print_digest(as_bytes(line));
    return 0;
  }

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
    std::cerr << "Režimas: tekstas iš argumento\n";
    print_digest(as_bytes(argv[2]));
    return 0;
  }

  if (mode == "--file") {
    std::cerr << "Režimas: failo turinys (" << argv[2] << ")\n";
    std::vector<std::uint8_t> bytes;
    if (!read_file_bytes(argv[2], bytes)) {
      std::cerr << "klaida: nepavyko perskaityti failo: " << argv[2] << '\n';
      return 2;
    }
    print_digest(bytes);
    return 0;
  }

  print_usage(std::cerr, program);
  return 1;
}
