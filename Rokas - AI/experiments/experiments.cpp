#include <algorithm>
#include <array>
#include <bit>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <random>
#include <string>
#include <string_view>
#include <vector>

#include "impl.hpp"

namespace {

using Bytes = std::vector<std::uint8_t>;
using Clock = std::chrono::steady_clock;

constexpr std::uint64_t kSeed = 20260920;
constexpr std::uint8_t kFirst = '!';
constexpr unsigned kAlphabet = 94;  // '!'..'~'
constexpr std::size_t kLengths[] = {10, 100, 500, 1000};

volatile std::uint8_t g_sink = 0;

impl::Digest H(const Bytes& b) { return impl::hash(b.data(), b.size()); }

Bytes to_bytes(std::string_view s) { return Bytes(s.begin(), s.end()); }

Bytes concat(const Bytes& a, const Bytes& b) {
  Bytes out(a);
  out.insert(out.end(), b.begin(), b.end());
  return out;
}

Bytes read_file(const std::string& path) {
  std::ifstream in(path, std::ios::binary);
  return Bytes(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

void random_ascii(std::mt19937_64& rng, std::uint8_t* out, std::size_t n) {
  for (std::size_t i = 0; i < n; ++i) {
    out[i] = static_cast<std::uint8_t>(kFirst + rng() % kAlphabet);
  }
}

Bytes random_bytes(std::mt19937_64& rng, std::size_t n) {
  Bytes out(n);
  for (auto& b : out) b = static_cast<std::uint8_t>(rng());
  return out;
}

std::string bytes_hex(const Bytes& b) {
  static constexpr char kDigits[] = "0123456789abcdef";
  std::string s;
  for (std::uint8_t x : b) {
    s += kDigits[x >> 4];
    s += kDigits[x & 15];
  }
  return s;
}

int hex_value(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return -1;
}

bool decode_hex(const std::string& hex, impl::Digest& out) {
  if (hex.size() != 2 * out.size()) return false;
  for (std::size_t i = 0; i < out.size(); ++i) {
    const int hi = hex_value(hex[2 * i]), lo = hex_value(hex[2 * i + 1]);
    if (hi < 0 || lo < 0) return false;
    out[i] = static_cast<std::uint8_t>(hi * 16 + lo);
  }
  return true;
}

// 64 hex digits in one consistent letter case that decode back to the digest.
bool format_ok(const std::string& hex, const impl::Digest& digest) {
  const bool lower = std::none_of(hex.begin(), hex.end(), [](char c) { return c >= 'A' && c <= 'F'; });
  const bool upper = std::none_of(hex.begin(), hex.end(), [](char c) { return c >= 'a' && c <= 'f'; });
  impl::Digest back{};
  return hex.size() == 64 && (lower || upper) && decode_hex(hex, back) && back == digest;
}

std::size_t utf8_chars(const Bytes& b) {
  return static_cast<std::size_t>(std::count_if(b.begin(), b.end(), [](std::uint8_t x) { return (x & 0xC0) != 0x80; }));
}

double micros(Clock::duration d) { return std::chrono::duration<double, std::micro>(d).count(); }

// 1-3: prepared inputs, output format and determinism.
int cmd_inputs(const std::string& dir) {
  std::vector<std::pair<std::string, Bytes>> inputs;
  std::vector<std::filesystem::path> files;
  for (const auto& e : std::filesystem::directory_iterator(dir)) files.push_back(e.path());
  std::sort(files.begin(), files.end());
  for (const auto& f : files) inputs.emplace_back(f.filename().string(), read_file(f.string()));
  inputs.emplace_back("crlf_atmintyje", to_bytes("tekstas\r\n"));

  for (const auto& [name, bytes] : inputs) {
    const impl::Digest d1 = H(bytes), d2 = H(bytes), d3 = H(bytes);
    const std::string hex = impl::to_hex(d1);
    std::printf("input,%s,%s,%zu,%zu,%s,%d,%d\n", impl::kName, name.c_str(), bytes.size(), utf8_chars(bytes),
                hex.c_str(), format_ok(hex, d1), d1 == d2 && d2 == d3);
  }

  const impl::Digest a1 = H(to_bytes("A")), b = H(to_bytes("B")), a2 = H(to_bytes("A"));
  std::printf("aba,%s,%d\n", impl::kName, a1 == a2 && a1 != b);

  const auto it = std::max_element(inputs.begin(), inputs.end(), [](auto& x, auto& y) { return x.second.size() < y.second.size(); });
  const impl::Digest ref = H(it->second);
  bool same = true;
  for (int i = 0; i < 1000; ++i) same = same && H(it->second) == ref;
  std::printf("repeat,%s,%s,1000,%d\n", impl::kName, it->first.c_str(), same);

  std::size_t zero1 = 0, zero2 = 0;
  char cand[5];
  for (int i = 0; i < 10000; ++i) {
    std::snprintf(cand, sizeof(cand), "%04d", i);
    const impl::Digest d = H(to_bytes(cand));
    const std::string hex = impl::to_hex(d);
    zero1 += hex[0] == '0';
    if (hex.compare(0, 2, "00") == 0) {
      if (++zero2 <= 3) std::printf("leadex,%s,%s,%s,%d\n", impl::kName, cand, hex.c_str(), format_ok(hex, d));
    }
  }
  std::printf("lead,%s,10000,%zu,%zu\n", impl::kName, zero1, zero2);
  return 0;
}

// 4: time per hash for 1, 2, 4, ... line prefixes of a text file and the whole file.
int cmd_speed(const std::string& path) {
  const Bytes text = read_file(path);
  std::vector<std::size_t> ends;
  for (std::size_t i = 0; i < text.size(); ++i) {
    if (text[i] == '\n') ends.push_back(i + 1);
  }
  if (!text.empty() && text.back() != '\n') ends.push_back(text.size());

  std::vector<std::size_t> counts;
  for (std::size_t k = 1; k < ends.size(); k *= 2) counts.push_back(k);
  counts.push_back(ends.size());

  for (const std::size_t lines : counts) {
    const Bytes input(text.begin(), text.begin() + static_cast<std::ptrdiff_t>(ends[lines - 1]));
    const auto run = [&](std::size_t reps) {
      const auto t0 = Clock::now();
      for (std::size_t r = 0; r < reps; ++r) g_sink = g_sink ^ H(input)[r & 31];
      return Clock::now() - t0;
    };
    std::size_t reps = 1;
    while (run(reps) < std::chrono::milliseconds(20)) reps *= 2;
    for (int w = 0; w < 3; ++w) run(reps);
    for (int m = 0; m < 10; ++m) {
      const double ns = std::chrono::duration<double, std::nano>(run(reps)).count();
      std::printf("speed,%s,%zu,%zu,%d,%zu,%.0f,%.3f\n", impl::kName, lines, input.size(), m, reps, ns, ns / static_cast<double>(reps));
    }
  }
  return 0;
}

std::uint64_t prefix_bits(const impl::Digest& d, unsigned bits) {
  std::uint64_t v = 0;
  for (int i = 0; i < 8; ++i) v = (v << 8) | d[static_cast<std::size_t>(i)];
  return v >> (64 - bits);
}

// Counts pairs of equal keys: a group of g equal keys contributes g(g-1)/2.
std::uint64_t equal_pairs(std::vector<std::uint64_t> keys) {
  std::sort(keys.begin(), keys.end());
  std::uint64_t pairs = 0;
  for (std::size_t i = 0, j; i < keys.size(); i = j) {
    for (j = i + 1; j < keys.size() && keys[j] == keys[i]; ++j) {}
    pairs += (j - i) * (j - i - 1) / 2;
  }
  return pairs;
}

// 5: random pairs of each length, pairwise and across the whole set.
int cmd_collisions() {
  constexpr std::size_t kPairs = 100000;
  for (const std::size_t L : kLengths) {
    std::mt19937_64 rng(kSeed + L);
    const std::size_t count = 2 * kPairs;
    Bytes buf(count * L);
    std::size_t regenerated = 0;
    for (std::size_t i = 0; i < kPairs; ++i) {
      std::uint8_t* a = &buf[2 * i * L];
      std::uint8_t* b = a + L;
      random_ascii(rng, a, L);
      random_ascii(rng, b, L);
      while (std::equal(a, a + L, b)) {
        random_ascii(rng, b, L);
        ++regenerated;
      }
    }
    std::vector<impl::Digest> d(count);
    for (std::size_t i = 0; i < count; ++i) d[i] = impl::hash(&buf[i * L], L);

    std::size_t pair_collisions = 0;
    for (std::size_t i = 0; i < kPairs; ++i) pair_collisions += d[2 * i] == d[2 * i + 1];

    const auto at = [&](std::uint32_t i) { return &buf[i * L]; };
    std::vector<std::uint32_t> idx(count);
    std::iota(idx.begin(), idx.end(), 0u);
    std::sort(idx.begin(), idx.end(), [&](auto x, auto y) { return std::lexicographical_compare(at(x), at(x) + L, at(y), at(y) + L); });
    std::vector<std::uint32_t> distinct;
    for (std::size_t i = 0; i < idx.size(); ++i) {
      if (i == 0 || !std::equal(at(idx[i]), at(idx[i]) + L, at(idx[i - 1]))) distinct.push_back(idx[i]);
    }
    std::sort(distinct.begin(), distinct.end(), [&](auto x, auto y) { return d[x] < d[y]; });
    std::size_t groups = 0;
    for (std::size_t i = 0, j; i < distinct.size(); i = j) {
      for (j = i + 1; j < distinct.size() && d[distinct[j]] == d[distinct[i]]; ++j) {}
      if (j - i > 1) {
        ++groups;
        std::printf("example,%s,%zu,%s,%.*s,%.*s\n", impl::kName, L, impl::to_hex(d[distinct[i]]).c_str(),
                    static_cast<int>(L), reinterpret_cast<const char*>(at(distinct[i])), static_cast<int>(L),
                    reinterpret_cast<const char*>(at(distinct[i + 1])));
      }
    }
    std::printf("pairs,%s,%zu,%zu,%zu,%zu,%zu,%zu,%zu\n", impl::kName, L, kPairs, regenerated, pair_collisions, count,
                distinct.size(), groups);

    const double m = static_cast<double>(distinct.size());
    for (const unsigned bits : {24u, 32u, 40u}) {
      std::vector<std::uint64_t> keys;
      keys.reserve(distinct.size());
      for (const auto i : distinct) keys.push_back(prefix_bits(d[i], bits));
      std::printf("trunc,%s,%zu,%u,%zu,%llu,%.3f\n", impl::kName, L, bits, distinct.size(),
                  static_cast<unsigned long long>(equal_pairs(keys)), m * (m - 1) / 2 / std::ldexp(1.0, static_cast<int>(bits)));
    }
  }
  return 0;
}

void report_set(const std::string& name, std::vector<Bytes> set) {
  const std::size_t total = set.size();
  std::sort(set.begin(), set.end());
  set.erase(std::unique(set.begin(), set.end()), set.end());
  std::vector<impl::Digest> d;
  d.reserve(set.size());
  for (const auto& b : set) d.push_back(H(b));
  std::sort(d.begin(), d.end());
  std::size_t groups = 0;
  for (std::size_t i = 0, j; i < d.size(); i = j) {
    for (j = i + 1; j < d.size() && d[j] == d[i]; ++j) {}
    groups += j - i > 1;
  }
  std::printf("struct,%s,%s,%zu,%zu,%zu\n", impl::kName, name.c_str(), total, set.size(), groups);
}

// 5: structured inputs that random strings rarely produce.
int cmd_structured() {
  std::mt19937_64 rng(kSeed + 300);
  std::vector<std::pair<std::string, std::vector<Bytes>>> cats;

  std::vector<Bytes> s;
  for (int i = 0; i < 256; ++i) s.push_back({static_cast<std::uint8_t>(i)});
  cats.emplace_back("visi 1 baito", s);

  s.clear();
  for (int i = 0; i < 65536; ++i) s.push_back({static_cast<std::uint8_t>(i >> 8), static_cast<std::uint8_t>(i)});
  cats.emplace_back("visi 2 baitų", s);

  s.clear();
  for (std::size_t n = 0; n <= 4096; ++n) s.push_back(Bytes(n, 0));
  cats.emplace_back("nuliniai baitai (0–4096)", s);

  s.clear();
  for (std::size_t n = 0; n <= 4096; ++n) s.push_back(Bytes(n, 'a'));
  cats.emplace_back("'a' kartojimas (0–4096)", s);

  s.clear();
  Bytes block(32);
  random_ascii(rng, block.data(), block.size());
  for (int k = 1; k <= 512; ++k) {
    Bytes b;
    for (int j = 0; j < k; ++j) b.insert(b.end(), block.begin(), block.end());
    s.push_back(b);
  }
  cats.emplace_back("32 B blokas ×1–512", s);

  s.clear();
  std::string perm = "abcdefgh";
  do { s.push_back(to_bytes(perm)); } while (std::next_permutation(perm.begin(), perm.end()));
  cats.emplace_back("'abcdefgh' perstatymai", s);

  s.clear();
  for (int k = 0; k < 1000; ++k) {
    Bytes x(32), y(32);
    random_ascii(rng, x.data(), 32);
    do { random_ascii(rng, y.data(), 32); } while (x == y);
    s.push_back(concat(x, y));
    s.push_back(concat(y, x));
  }
  cats.emplace_back("32 B blokų tvarka XY / YX", s);

  s.clear();
  for (std::size_t k = 0; k <= 1024; ++k) {
    s.push_back(concat(to_bytes("ab"), Bytes(k, 0)));
    s.push_back(concat(Bytes(k, 0), to_bytes("ab")));
  }
  cats.emplace_back("'ab' su nuliais priekyje / gale", s);

  s.clear();
  Bytes base(64);
  random_ascii(rng, base.data(), base.size());
  s.push_back(base);
  for (std::size_t bit = 0; bit < base.size() * 8; ++bit) {
    Bytes b = base;
    b[bit / 8] ^= static_cast<std::uint8_t>(1u << (bit % 8));
    s.push_back(b);
  }
  cats.emplace_back("vieno bito pakeitimai 64 B įvestyje", s);

  std::vector<Bytes> all;
  for (const auto& [name, set] : cats) {
    report_set(name, set);
    all.insert(all.end(), set.begin(), set.end());
  }
  report_set("visi kartu", all);
  return 0;
}

struct Stats {
  double min = 1e9, max = -1e9, sum = 0, sq = 0;
  std::size_t n = 0;
  void add(double v) {
    min = std::min(min, v);
    max = std::max(max, v);
    sum += v;
    sq += v * v;
    ++n;
  }
  double mean() const { return sum / static_cast<double>(n); }
  double sd() const { return std::sqrt(std::max(0.0, sq / static_cast<double>(n) - mean() * mean())); }
};

void print_aval(const char* mode, const std::string& length, const Stats& bits, const Stats& hex) {
  std::printf("aval,%s,%s,%s,%zu,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f\n", impl::kName, mode, length.c_str(), bits.n,
              bits.min, bits.max, bits.mean(), bits.sd(), hex.min, hex.max, hex.mean(), hex.sd());
}

// 6: one changed character (and, separately, one flipped bit) per pair.
int cmd_avalanche() {
  constexpr std::size_t kPerLength = 25000;
  for (int mode = 0; mode < 2; ++mode) {
    const char* name = mode == 0 ? "simbolis" : "bitas";
    Stats all_bits, all_hex;
    std::array<std::uint64_t, 257> hist{};
    for (const std::size_t L : kLengths) {
      std::mt19937_64 rng(kSeed + (mode == 0 ? 100 : 200) + L);
      Stats bits, hex;
      Bytes s(L), t;
      for (std::size_t i = 0; i < kPerLength; ++i) {
        random_ascii(rng, s.data(), L);
        t = s;
        const std::size_t p = rng() % L;
        if (mode == 0) {
          auto c = static_cast<std::uint8_t>(kFirst + rng() % (kAlphabet - 1));
          if (c >= s[p]) ++c;
          t[p] = c;
        } else {
          t[p] ^= static_cast<std::uint8_t>(1u << (rng() % 8));
        }
        const std::string ha = impl::to_hex(H(s)), hb = impl::to_hex(H(t));
        impl::Digest xa{}, xb{};
        decode_hex(ha, xa);
        decode_hex(hb, xb);
        int diff_bits = 0, diff_hex = 0;
        for (std::size_t k = 0; k < xa.size(); ++k) diff_bits += std::popcount(static_cast<unsigned>(xa[k] ^ xb[k]));
        for (std::size_t k = 0; k < ha.size(); ++k) diff_hex += ha[k] != hb[k];
        const double bp = 100.0 * diff_bits / 256, hp = 100.0 * diff_hex / 64;
        bits.add(bp);
        hex.add(hp);
        all_bits.add(bp);
        all_hex.add(hp);
        ++hist[static_cast<std::size_t>(diff_bits)];
      }
      print_aval(name, std::to_string(L), bits, hex);
    }
    print_aval(name, "visi", all_bits, all_hex);
    for (std::size_t k = 0; k < hist.size(); ++k) {
      if (hist[k]) std::printf("hist,%s,%s,%zu,%llu\n", impl::kName, name, k, static_cast<unsigned long long>(hist[k]));
    }
  }
  return 0;
}

// 7: exhaustive guessing over 0000-9999 without salt, with a public salt, and with a secret r.
int cmd_guess() {
  std::vector<Bytes> cand;
  char buf[5];
  for (int i = 0; i < 10000; ++i) {
    std::snprintf(buf, sizeof(buf), "%04d", i);
    cand.push_back(to_bytes(buf));
  }
  const auto str = [&](std::size_t i) { return std::string(cand[i].begin(), cand[i].end()); };
  std::mt19937_64 rng(kSeed + 7);

  const std::size_t target = rng() % cand.size();
  const impl::Digest goal = H(cand[target]);
  auto t0 = Clock::now();
  std::size_t first = 0;
  std::string matches;
  for (std::size_t i = 0; i < cand.size(); ++i) {
    if (H(cand[i]) == goal) {
      if (!first) first = i + 1;
      matches += (matches.empty() ? "" : ";") + str(i);
    }
  }
  std::printf("nosalt,%s,%s,%zu,%zu,%s,%.1f\n", impl::kName, str(target).c_str(), first, cand.size(), matches.c_str(), micros(Clock::now() - t0));

  std::vector<std::size_t> targets(5);
  for (auto& t : targets) t = rng() % cand.size();
  t0 = Clock::now();
  std::map<impl::Digest, std::size_t> table;
  for (std::size_t i = 0; i < cand.size(); ++i) table.emplace(H(cand[i]), i);
  std::size_t cracked = 0;
  for (const auto t : targets) {
    const auto it = table.find(H(cand[t]));
    cracked += it != table.end() && it->second == t;
  }
  std::printf("table,%s,%zu,%zu,%.1f,%zu\n", impl::kName, targets.size(), cand.size(), micros(Clock::now() - t0), cracked);

  for (std::size_t j = 0; j < targets.size(); ++j) {
    const Bytes salt = random_bytes(rng, 16);
    const impl::Digest salted = H(concat(cand[targets[j]], salt));
    t0 = Clock::now();
    std::size_t found = 0, count = 0;
    for (std::size_t i = 0; i < cand.size(); ++i) {
      if (H(concat(cand[i], salt)) == salted) {
        ++count;
        if (!found) found = i + 1;
      }
    }
    std::printf("salt,%s,%zu,%s,%s,%zu,%zu,%.1f\n", impl::kName, j + 1, str(targets[j]).c_str(), bytes_hex(salt).c_str(), found, count,
                micros(Clock::now() - t0));
  }

  const Bytes r = random_bytes(rng, 16);
  const impl::Digest commitment = H(concat(cand[target], r));
  const bool verified = H(concat(cand[target], r)) == commitment;
  const bool wrong = H(concat(cand[(target + 1) % cand.size()], r)) != commitment;
  std::printf("commit,%s,%s,%s,%d,%d\n", impl::kName, str(target).c_str(), bytes_hex(r).c_str(), verified, wrong);
  return 0;
}

}  // namespace

int main(int argc, char** argv) {
  const std::string cmd = argc > 1 ? argv[1] : "";
  if (cmd == "inputs" && argc == 3) return cmd_inputs(argv[2]);
  if (cmd == "speed" && argc == 3) return cmd_speed(argv[2]);
  if (cmd == "collisions") return cmd_collisions();
  if (cmd == "structured") return cmd_structured();
  if (cmd == "avalanche") return cmd_avalanche();
  if (cmd == "guess") return cmd_guess();
  std::cerr << "naudojimas: experiments inputs <katalogas> | speed <failas> | collisions | structured | avalanche | guess\n";
  return 1;
}
