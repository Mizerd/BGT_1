// Standartinės maišos funkcijos (MD5, SHA-1, SHA-256) TIK palyginimui
// papildomoje užduotyje. Naudojama operacinės sistemos / bibliotekos realizacija:
// Windows – CNG (bcrypt.h), kitur – OpenSSL. Ratas-256 jų niekaip nenaudoja.

#ifndef STD_HASHES_HPP
#define STD_HASHES_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace stdhash {

enum class Kind { MD5, SHA1, SHA256 };

// Grąžina santrauką baitais (16, 20 arba 32). Klaidos atveju – tuščią vektorių.
std::vector<std::uint8_t> digest(Kind kind, const std::uint8_t* data, std::size_t size);

const char* name(Kind kind);
std::size_t digest_bytes(Kind kind);

}  // namespace stdhash

#endif
