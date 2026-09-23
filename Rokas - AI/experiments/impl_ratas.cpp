#include <algorithm>

#include "impl.hpp"
#include "ratas.hpp"

namespace impl {

const char* const kName = "ratas";

Digest hash(const std::uint8_t* data, std::size_t size) {
  const ratas::Digest bytes = ratas::hash(data, size);
  Digest digest{};
  std::copy_n(bytes.begin(), digest.size(), digest.begin());
  return digest;
}

std::string to_hex(const Digest& digest) {
  return ratas::to_hex(ratas::Digest(digest.begin(), digest.end()));
}

}  // namespace impl
