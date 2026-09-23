#include "custom_hash.hpp"
#include "impl.hpp"

#ifndef IMPL_NAME
#define IMPL_NAME "di"
#endif

namespace impl {

const char* const kName = IMPL_NAME;

Digest hash(const std::uint8_t* data, std::size_t size) {
  return eduhash::custom_hash({data, size});
}

std::string to_hex(const Digest& digest) { return eduhash::to_hex(digest); }

}  // namespace impl
