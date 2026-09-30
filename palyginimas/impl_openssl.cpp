// Standartinės maišos (OpenSSL) bendrai eksperimentų programai: MD5, SHA-1, SHA-256.
// Kompiliuojama po kartą kiekvienam algoritmui, pvz.:
//   -DIMPL_NAME='"sha256"' -DALGO='"SHA256"' -DDIGEST_BYTES=32

#include <openssl/evp.h>

#include <cstdio>
#include <cstdlib>

#include "impl.hpp"

namespace impl {

const char* const kName = IMPL_NAME;

Digest hash(const std::uint8_t* data, std::size_t size) {
  // Algoritmas surandamas vieną kartą, todėl matuojamas tik maišos skaičiavimas.
  static EVP_MD* const md = EVP_MD_fetch(nullptr, ALGO, nullptr);
  static EVP_MD_CTX* const ctx = EVP_MD_CTX_new();
  Digest digest{};
  unsigned int length = 0;
  if (md == nullptr || ctx == nullptr || !EVP_DigestInit_ex2(ctx, md, nullptr) ||
      !EVP_DigestUpdate(ctx, data, size) || !EVP_DigestFinal_ex(ctx, digest.data(), &length) ||
      length != digest.size()) {
    std::fprintf(stderr, "OpenSSL %s: klaida\n", ALGO);
    std::exit(2);
  }
  return digest;
}

std::string to_hex(const Digest& digest) {
  static constexpr char kDigits[] = "0123456789abcdef";
  std::string text;
  for (const std::uint8_t byte : digest) {
    text.push_back(kDigits[byte >> 4]);
    text.push_back(kDigits[byte & 0x0F]);
  }
  return text;
}

}  // namespace impl
