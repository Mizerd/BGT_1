// Eksperimentai pagal užduoties 4–7 skyrius. Rezultatai rašomi į results/,
// testinės įvestys – į data/exp1/. Viskas atkartojama: naudojamas savas
// xorshift generatorius su fiksuota pradine reikšme (seed), kad rezultatai
// nepriklausytų nuo kompiliatoriaus standartinės bibliotekos.
//
// Paleidimas:  experiments <inputs|correctness|determinism|bench|collisions|
//                           avalanche|diffusion|preimage|all>  [--no-std]
// --no-std išjungia standartinių maišų (MD5/SHA-1/SHA-256) palyginimą.

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "ratas.hpp"
#include "std_hashes.hpp"

namespace {

// ---------------------------------------------------------------------------
// Bendros priemonės
// ---------------------------------------------------------------------------

using Bytes = std::vector<std::uint8_t>;

// xorshift64* – paprastas, visur vienodai veikiantis pseudoatsitiktinis
// generatorius. Ne kriptografinis, tik testinėms įvestims gaminti.
struct Rng {
    std::uint64_t s;
    explicit Rng(std::uint64_t seed) : s(seed ? seed : 1) {}
    std::uint64_t next() {
        s ^= s >> 12;
        s ^= s << 25;
        s ^= s >> 27;
        return s * 2685821657736338717ull;
    }
    std::uint64_t below(std::uint64_t n) { return next() % n; }  // nedidelis poslinkis (bias) toleruojamas
};

constexpr std::uint64_t kSeed = 20260920;  // 2026-09-20
// Abėcėlė: spausdinami ASCII simboliai nuo '!' (0x21) iki '~' (0x7E) – 94 simboliai, po 1 baitą.
const std::string kAlphabet = []() {
    std::string a;
    for (int c = 0x21; c <= 0x7E; ++c) a.push_back(static_cast<char>(c));
    return a;
}();

std::string random_string(Rng& rng, std::size_t len) {
    std::string s(len, ' ');
    for (auto& ch : s) ch = kAlphabet[rng.below(kAlphabet.size())];
    return s;
}

struct HashFn {
    std::string name;
    std::size_t bits;
    std::function<Bytes(const std::uint8_t*, std::size_t)> fn;
    Bytes operator()(const std::string& s) const {
        return fn(reinterpret_cast<const std::uint8_t*>(s.data()), s.size());
    }
    Bytes operator()(const Bytes& b) const { return fn(b.data(), b.size()); }
};

bool g_use_std = true;

std::vector<HashFn> ratas_versions() {
    return {
        {"Ratas v0.1", 256, [](const std::uint8_t* d, std::size_t n) { return ratas::hash(d, n); }},
    };
}

std::vector<HashFn> all_hashes() {
    auto v = ratas_versions();
    if (g_use_std) {
        for (auto k : {stdhash::Kind::MD5, stdhash::Kind::SHA1, stdhash::Kind::SHA256}) {
            v.push_back({stdhash::name(k), stdhash::digest_bytes(k) * 8,
                         [k](const std::uint8_t* d, std::size_t n) { return stdhash::digest(k, d, n); }});
        }
    }
    return v;
}

std::string hex(const Bytes& b) {
    static const char* digits = "0123456789abcdef";
    std::string s;
    for (auto x : b) { s.push_back(digits[x >> 4]); s.push_back(digits[x & 15]); }
    return s;
}

int popcount8(std::uint8_t x) {
    int c = 0;
    while (x) { c += x & 1; x >>= 1; }
    return c;
}

// Bitų skirtumas skaičiuojamas iš pačių baitų (ne iš hex teksto).
int bit_diff(const Bytes& a, const Bytes& b) {
    int d = 0;
    for (std::size_t i = 0; i < a.size(); ++i) d += popcount8(static_cast<std::uint8_t>(a[i] ^ b[i]));
    return d;
}

int hex_diff(const Bytes& a, const Bytes& b) {
    int d = 0;
    for (std::size_t i = 0; i < a.size(); ++i) {
        if ((a[i] >> 4) != (b[i] >> 4)) ++d;
        if ((a[i] & 15) != (b[i] & 15)) ++d;
    }
    return d;
}

bool write_file(const std::string& path, const std::string& content) {
    std::ofstream f(path, std::ios::binary);
    f.write(content.data(), static_cast<std::streamsize>(content.size()));
    return static_cast<bool>(f);
}

bool read_file(const std::string& path, Bytes& out) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return false;
    out.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
    return true;
}

std::string escape(const std::string& s) {  // kad lentelėse matytųsi \n ir pan.
    std::string o;
    for (unsigned char c : s) {
        if (c == '\n') o += "\\n";
        else if (c == '\r') o += "\\r";
        else if (c == '\t') o += "\\t";
        else if (c == '|') o += "\\|";
        else if (c < 0x20) { char b[8]; std::snprintf(b, sizeof b, "\\x%02x", c); o += b; }
        else o.push_back(static_cast<char>(c));
    }
    return o;
}

std::size_t utf8_chars(const std::string& s) {
    std::size_t n = 0;
    for (unsigned char c : s) if ((c & 0xC0) != 0x80) ++n;
    return n;
}

const std::string kData = "data/exp1/";
const std::string kResults = "results/";

std::string fmt(double v, int prec = 2) {
    std::ostringstream o;
    o << std::fixed << std::setprecision(prec) << v;
    return o.str();
}

// ---------------------------------------------------------------------------
// 1 eksperimentas – testinės įvestys
// ---------------------------------------------------------------------------

struct InputCase { std::string file, content, description; };

std::vector<InputCase> build_inputs() {
    Rng rng(kSeed);
    std::vector<InputCase> v;
    v.push_back({"empty.bin", "", "tuščias failas"});
    v.push_back({"a.bin", "a", "vienas baitas 'a', be naujos eilutės"});
    v.push_back({"b.bin", "b", "vienas baitas 'b', be naujos eilutės"});

    const std::size_t sizes[3] = {1500, 2048, 4096};
    for (int i = 0; i < 3; ++i) {
        std::string base = random_string(rng, sizes[i]);
        std::string id = std::to_string(i + 1);
        v.push_back({"random_" + id + ".txt", base, "atsitiktinis ASCII (abėcėlė '!'..'~'), seed " + std::to_string(kSeed)});
        const char* where[3] = {"start", "middle", "end"};
        const std::size_t pos[3] = {0, base.size() / 2, base.size() - 1};
        for (int k = 0; k < 3; ++k) {
            std::string copy = base;
            char nc;
            do { nc = kAlphabet[rng.below(kAlphabet.size())]; } while (nc == copy[pos[k]]);
            copy[pos[k]] = nc;
            v.push_back({"random_" + id + "_" + where[k] + ".txt", copy,
                         "random_" + id + " su pakeistu 1 baitu pozicijoje " + std::to_string(pos[k])});
        }
    }
    v.push_back({"struct_repeat_a.txt", std::string(32, 'a'), "32 kartus 'a'"});
    v.push_back({"struct_repeat_ab.txt", "abababababababababababababababab", "16 kartų 'ab'"});
    v.push_back({"struct_order_abc.txt", "abc", "simbolių tvarka"});
    v.push_back({"struct_order_cba.txt", "cba", "ta pati aibė, kita tvarka"});
    v.push_back({"struct_order_words1.txt", "labas rytas", "žodžių tvarka"});
    v.push_back({"struct_order_words2.txt", "rytas labas", "sukeisti žodžiai"});
    v.push_back({"struct_space_none.txt", "tekstas", "be tarpų"});
    v.push_back({"struct_space_none_copy.txt", "tekstas", "tie patys baitai kitu failo vardu (santrauka turi sutapti)"});
    v.push_back({"struct_space_lead.txt", " tekstas", "tarpas pradžioje"});
    v.push_back({"struct_space_trail.txt", "tekstas ", "tarpas pabaigoje"});
    v.push_back({"struct_newline_lf.txt", "tekstas\n", "su LF pabaigoje"});
    v.push_back({"struct_newline_crlf.txt", "tekstas\r\n", "su CRLF pabaigoje"});
    v.push_back({"struct_len15.txt", std::string(15, 'x'), "bloko riba: 15 baitų"});
    v.push_back({"struct_len16.txt", std::string(16, 'x'), "bloko riba: 16 baitų"});
    v.push_back({"struct_len17.txt", std::string(17, 'x'), "bloko riba: 17 baitų"});
    v.push_back({"struct_pad_ab.txt", "ab", "užpildo patikra: 'ab'"});
    v.push_back({"struct_pad_ab0.txt", std::string("ab\0", 3), "užpildo patikra: 'ab\\0'"});
    v.push_back({"utf8_lt.txt", "ąčęėįšųūž Žąsis", "UTF-8 su ne ASCII simboliais"});
    v.push_back({"utf8_mixed.txt", "Vilnius – Lietuva €", "UTF-8: brūkšnys ir euro ženklas"});
    return v;
}

void exp_inputs() {
    auto cases = build_inputs();
    std::ostringstream md;
    md << "# 1 eksperimentas. Testinės įvestys\n\n"
       << "Sugeneruota programa `experiments inputs`, seed = " << kSeed
       << ", abėcėlė atsitiktiniams failams: ASCII `!`..`~` (94 simboliai, 1 simbolis = 1 baitas).\n"
       << "Failai rašomi dvejetainiu režimu, be jokių eilučių pabaigų keitimų.\n\n"
       << "| Failas | Baitų | Simbolių (UTF-8) | Aprašymas |\n|---|---:|---:|---|\n";
    for (auto& c : cases) {
        if (!write_file(kData + c.file, c.content)) { std::cerr << "Nepavyko įrašyti " << c.file << '\n'; }
        md << "| `" << c.file << "` | " << c.content.size() << " | " << utf8_chars(c.content) << " | " << c.description << " |\n";
    }
    write_file(kResults + "exp1_inputs.md", md.str());
    std::cout << "Įrašyta " << cases.size() << " failų į " << kData << '\n';
}

// ---------------------------------------------------------------------------
// 2 eksperimentas – išvesties formatas
// ---------------------------------------------------------------------------

bool valid_hex64(const std::string& h) {
    if (h.size() != ratas::kDigestHexLen) return false;
    for (char c : h) if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) return false;
    return true;
}

void exp_correctness() {
    auto cases = build_inputs();
    std::ostringstream md;
    md << "# 2 eksperimentas. Išvesties formatas\n\n"
       << "Deklaruotas ilgis: 256 bitų = 64 hex simboliai, mažosios raidės, su pradiniais nuliais.\n"
       << "Tikrinama per API (tas pats kodas, kurį naudoja komandinė eilutė); komandinės eilutės\n"
       << "režimų (rankinis / --text / --file) sutapimas tikrinamas `tests/cli_checks.sh`.\n\n"
       << "| Failas | Baitų | 64 hex? | Ratas v0.1 |\n|---|---:|:---:|---|\n";
    int bad = 0;
    std::map<std::string, std::string> first_seen;
    std::vector<std::string> same_content;
    for (auto& c : cases) {
        Bytes bytes;
        if (!read_file(kData + c.file, bytes)) { std::cerr << "trūksta " << c.file << "\n"; continue; }
        std::string h11 = ratas::to_hex(ratas::hash(bytes));
        bool ok = valid_hex64(h11);
        if (!ok) ++bad;
        md << "| `" << c.file << "` | " << bytes.size() << " | " << (ok ? "taip" : "NE") << " | `" << h11 << "` |\n";
        auto it = first_seen.find(h11);
        if (it != first_seen.end()) same_content.push_back(c.file + " = " + it->second);
        else first_seen[h11] = c.file;
    }
    md << "\nNetinkamo formato rezultatų: " << bad << ".\n";
    md << "Sutampančios santraukos tarp failų (turi sutapti tik failai su vienodais baitais): ";
    if (same_content.empty()) md << "nėra.\n";
    else { md << "\n"; for (auto& s : same_content) md << "- " << s << "\n"; }
    // Pradinių nulių demonstracija: ieškome įvesties, kurios santrauka prasideda '0'.
    for (int i = 0; i < 100000; ++i) {
        std::string s = "nulis" + std::to_string(i);
        std::string h = ratas::to_hex(ratas::hash(s));
        if (h[0] == '0') { md << "\nPradinio nulio pavyzdys: `" << s << "` -> `" << h << "` (64 simboliai).\n"; break; }
    }
    write_file(kResults + "exp2_format.md", md.str());
    std::cout << "exp2: blogų formatų " << bad << '\n';
}

// ---------------------------------------------------------------------------
// 3 eksperimentas – determinizmas (API lygmuo; atskiri paleidimai – scenarijuje)
// ---------------------------------------------------------------------------

void exp_determinism() {
    auto cases = build_inputs();
    std::ostringstream md;
    md << "# 3 eksperimentas. Determinizmas (API)\n\n"
       << "Kiekviena įvestis maišoma 3 kartus iš eilės, tada seka A, B, A (B – kita įvestis),\n"
       << "tada A po 4096 baitų failo. Visi rezultatai turi sutapti. Atskirų programos paleidimų\n"
       << "palyginimas – `tests/cli_checks.sh` (žr. `exp3_cli.md`).\n\n"
       << "| Failas | 3 kartai | A,B,A | po didelio failo |\n|---|:---:|:---:|:---:|\n";
    int failures = 0;
    Bytes big;
    read_file(kData + "random_3.txt", big);
    Bytes b_bytes;
    read_file(kData + "b.bin", b_bytes);
    for (auto& c : cases) {
        Bytes a;
        read_file(kData + c.file, a);
        std::string h1 = hex(ratas::hash(a)), h2 = hex(ratas::hash(a)), h3 = hex(ratas::hash(a));
        bool rep = h1 == h2 && h2 == h3;
        ratas::hash(b_bytes);
        bool aba = hex(ratas::hash(a)) == h1;
        ratas::hash(big);
        bool after_big = hex(ratas::hash(a)) == h1;
        if (!(rep && aba && after_big)) ++failures;
        md << "| `" << c.file << "` | " << (rep ? "ok" : "NE") << " | " << (aba ? "ok" : "NE") << " | " << (after_big ? "ok" : "NE") << " |\n";
    }
    md << "\nNeatitikimų: " << failures << ".\n";
    write_file(kResults + "exp3_determinism.md", md.str());
    std::cout << "exp3: neatitikimų " << failures << '\n';
}

// ---------------------------------------------------------------------------
// 4 eksperimentas – sparta
// ---------------------------------------------------------------------------

volatile std::uint8_t g_sink = 0;  // kad kompiliatorius nepašalintų skaičiavimų

struct Timing { double mean_ns, min_ns, max_ns; std::size_t reps; };

Timing time_hash(const HashFn& h, const Bytes& input) {
    using clock = std::chrono::steady_clock;
    auto once = [&](std::size_t reps) {
        auto t0 = clock::now();
        for (std::size_t r = 0; r < reps; ++r) {
            Bytes d = h(input);
            g_sink = static_cast<std::uint8_t>(g_sink ^ d[0] ^ d[d.size() - 1]);
        }
        auto t1 = clock::now();
        return std::chrono::duration<double, std::nano>(t1 - t0).count() / static_cast<double>(reps);
    };
    for (int w = 0; w < 3; ++w) once(1);                  // apšilimas
    double probe = once(1);
    std::size_t reps = static_cast<std::size_t>(std::max(1.0, 20e6 / std::max(probe, 1.0)));  // grupė ~20 ms
    reps = std::min<std::size_t>(reps, 200000);
    const int measurements = 7;
    Timing t{0, 1e300, 0, reps};
    for (int m = 0; m < measurements; ++m) {
        double ns = once(reps);
        t.mean_ns += ns;
        t.min_ns = std::min(t.min_ns, ns);
        t.max_ns = std::max(t.max_ns, ns);
    }
    t.mean_ns /= measurements;
    return t;
}

void exp_bench() {
    Bytes text;
    if (!read_file("data/konstitucija.txt", text)) { std::cerr << "nėra data/konstitucija.txt\n"; return; }
    std::size_t total_lines = 0;
    for (auto c : text) if (c == '\n') ++total_lines;
    if (!text.empty() && text.back() != '\n') ++total_lines;

    // Ištraukos: pirmos 1, 2, 4, ... eilutės (su eilučių skirtukais), plius visas failas.
    std::vector<std::pair<std::size_t, Bytes>> pieces;
    for (std::size_t lines = 1; lines < total_lines; lines *= 2) {
        std::size_t seen = 0, end = text.size();
        for (std::size_t i = 0; i < text.size(); ++i)
            if (text[i] == '\n' && ++seen == lines) { end = i + 1; break; }
        pieces.push_back({lines, Bytes(text.begin(), text.begin() + static_cast<std::ptrdiff_t>(end))});
    }
    pieces.push_back({total_lines, text});

    auto hashes = all_hashes();
    std::ofstream csv(kResults + "exp4_bench.csv");
    csv << "hash,lines,bytes,reps,mean_ns,min_ns,max_ns,mb_per_s\n";
    std::ostringstream md;
    md << "# 4 eksperimentas. Sparta\n\n"
       << "Failas: `data/konstitucija.txt` (" << text.size() << " baitų, " << total_lines << " eilučių, UTF-8, LF).\n"
       << "Ištraukos sudaromos prieš matavimą; matuojamas tik `hash()` kvietimas (be failų I/O ir spausdinimo).\n"
       << "Laikmatis: `std::chrono::steady_clock`. 3 apšilimo kvietimai, tada 7 matavimai; kiekvienas matavimas –\n"
       << "grupė iš N kvietimų (N parenkamas taip, kad grupė truktų ~20 ms), laikas dalijamas iš N.\n"
       << "Rezultatai naudojami per `volatile` kintamąjį, kad optimizatorius neišmestų skaičiavimų.\n"
       << "Vienetai: mikrosekundės (µs) vienam kvietimui; vidurkis / min / max iš 7 matavimų.\n\n";
    for (auto& h : hashes) {
        md << "## " << h.name << "\n\n| Eilučių | Baitų | N kvietimų | vid. µs | min µs | max µs | MB/s |\n|---:|---:|---:|---:|---:|---:|---:|\n";
        for (auto& p : pieces) {
            Timing t = time_hash(h, p.second);
            double mbps = p.second.size() / (t.mean_ns / 1e9) / 1e6;
            csv << h.name << ',' << p.first << ',' << p.second.size() << ',' << t.reps << ',' << fmt(t.mean_ns, 1)
                << ',' << fmt(t.min_ns, 1) << ',' << fmt(t.max_ns, 1) << ',' << fmt(mbps, 2) << '\n';
            md << "| " << p.first << " | " << p.second.size() << " | " << t.reps << " | " << fmt(t.mean_ns / 1000, 3)
               << " | " << fmt(t.min_ns / 1000, 3) << " | " << fmt(t.max_ns / 1000, 3) << " | " << fmt(mbps, 1) << " |\n";
            std::cout << h.name << " " << p.second.size() << " B: " << fmt(t.mean_ns / 1000, 3) << " us\n";
        }
        md << "\n";
    }
    write_file(kResults + "exp4_bench.md", md.str());
}

// ---------------------------------------------------------------------------
// 5 eksperimentas – kolizijos
// ---------------------------------------------------------------------------

void exp_collisions() {
    const std::size_t lengths[4] = {10, 100, 500, 1000};
    const std::size_t pairs = 100000;
    auto versions = ratas_versions();
    std::ostringstream md;
    md << "# 5 eksperimentas. Kolizijų paieška\n\n"
       << "Kiekvienam ilgiui sugeneruota " << pairs << " porų (" << 2 * pairs << " eilučių) iš abėcėlės `!`..`~`\n"
       << "(94 ASCII simboliai, 1 simbolis = 1 baitas), seed = " << kSeed << " (generatorius xorshift64*, kiekvienam ilgiui\n"
       << "seed + ilgis). Užtikrinta, kad poros narės skiriasi. Tos pačios įvestys naudojamos visoms funkcijoms.\n\n"
       << "| Versija | Ilgis | Porų | Kolizijų porose | Skirtingų įvesčių | Grupių su bendra santrauka |\n|---|---:|---:|---:|---:|---:|\n";
    std::ostringstream examples;
    for (std::size_t L : lengths) {
        // Įvestys generuojamos vieną kartą ir laikomos atmintyje; tos pačios visiems matavimams.
        Rng rng(kSeed + L);
        std::vector<std::string> inputs;
        inputs.reserve(2 * pairs);
        for (std::size_t p = 0; p < pairs; ++p) {
            std::string a = random_string(rng, L), b;
            do { b = random_string(rng, L); } while (b == a);
            inputs.push_back(std::move(a));
            inputs.push_back(std::move(b));
        }
        std::unordered_set<std::string_view> distinct(inputs.begin(), inputs.end());
        for (auto& h : versions) {
            std::size_t pair_collisions = 0;
            std::vector<std::string> digests(inputs.size());
            for (std::size_t i = 0; i < inputs.size(); ++i) {
                Bytes d = h(inputs[i]);
                digests[i].assign(d.begin(), d.end());
            }
            for (std::size_t p = 0; p < pairs; ++p)
                if (digests[2 * p] == digests[2 * p + 1]) ++pair_collisions;
            std::unordered_map<std::string, std::vector<std::size_t>> by_digest;
            by_digest.reserve(inputs.size() * 2);
            for (std::size_t i = 0; i < inputs.size(); ++i) by_digest[digests[i]].push_back(i);
            std::size_t groups = 0;
            for (auto& kv : by_digest) {
                if (kv.second.size() < 2) continue;
                std::unordered_set<std::string_view> members;
                for (auto i : kv.second) members.insert(inputs[i]);
                if (members.size() >= 2) {
                    ++groups;
                    examples << h.name << " L=" << L << " santrauka " << hex(Bytes(kv.first.begin(), kv.first.end())) << ":\n";
                    for (auto& m : members) examples << "  " << m << "\n";
                }
            }
            md << "| " << h.name << " | " << L << " | " << pairs << " | " << pair_collisions << " | " << distinct.size() << " | " << groups << " |\n";
            std::cout << h.name << " L=" << L << ": porose " << pair_collisions << ", grupių " << groups << '\n';
        }
    }

    // Struktūruotos įvestys: perstatymai, pasikartojantys šablonai, trumpos eilutės,
    // užpildo "apgaulės" (baitai, lygūs užpildo reikšmei).
    std::vector<std::string> structured;
    std::string perm = "abcdefgh";
    do structured.push_back(perm); while (std::next_permutation(perm.begin(), perm.end()));
    for (std::size_t k = 0; k <= 300; ++k) structured.push_back(std::string(k, 'a'));
    for (std::size_t k = 1; k <= 150; ++k) { std::string s; for (std::size_t i = 0; i < k; ++i) s += "ab"; structured.push_back(s); }
    for (std::size_t k = 1; k <= 100; ++k) { std::string s; for (std::size_t i = 0; i < k; ++i) s += "abc"; structured.push_back(s); }
    for (char c1 : kAlphabet) for (char c2 : kAlphabet) structured.push_back(std::string{c1, c2});
    for (int c = 0; c < 256; ++c) structured.push_back(std::string(1, static_cast<char>(c)));
    for (std::size_t k = 1; k <= 16; ++k) {  // "ab" + k baitų, lygių užpildo reikšmei
        std::string s = "ab";
        s.append(k, static_cast<char>(16 - ((2 + k) % 16)));
        structured.push_back(s);
    }
    for (std::size_t k = 0; k <= 64; ++k) structured.push_back(std::string(k, '\0'));
    std::unordered_set<std::string> distinct_structured(structured.begin(), structured.end());
    md << "\n## Struktūruotos įvestys\n\nRinkinys: visi `abcdefgh` perstatymai (40320), `a`×k (k=0..300), `ab`×k (k=1..150), `abc`×k (k=1..100),\n"
       << "visos 2 simbolių eilutės iš abėcėlės (8836), visi 256 vieno baito atvejai, `ab` + užpildo reikšmę atitinkantys baitai,\n"
       << "`\\0`×k (k=0..64). Iš viso " << structured.size() << " eilučių, skirtingų: " << distinct_structured.size() << ".\n\n"
       << "| Versija | Skirtingų įvesčių | Grupių su bendra santrauka |\n|---|---:|---:|\n";
    for (auto& h : versions) {
        std::unordered_map<std::string, std::unordered_set<std::string>> by_digest;
        for (auto& s : structured) { Bytes d = h(s); by_digest[std::string(d.begin(), d.end())].insert(s); }
        std::size_t groups = 0;
        for (auto& kv : by_digest) if (kv.second.size() >= 2) {
            ++groups;
            examples << h.name << " struktūruota, santrauka " << hex(Bytes(kv.first.begin(), kv.first.end())) << ":\n";
            for (auto& m : kv.second) examples << "  \"" << escape(m) << "\"\n";
        }
        md << "| " << h.name << " | " << distinct_structured.size() << " | " << groups << " |\n";
    }
    md << "\nKolizijų pavyzdžiai (jei rasta): `exp5_collision_examples.txt`.\n";
    write_file(kResults + "exp5_collisions.md", md.str());
    write_file(kResults + "exp5_collision_examples.txt", examples.str().empty() ? "Kolizijų nerasta.\n" : examples.str());
}

// ---------------------------------------------------------------------------
// 6 eksperimentas – lavinos efektas
// ---------------------------------------------------------------------------

struct Stat {
    double sum = 0, mn = 1e9, mx = -1;
    std::size_t n = 0;
    void add(double v) { sum += v; mn = std::min(mn, v); mx = std::max(mx, v); ++n; }
    double mean() const { return n ? sum / n : 0; }
};

void exp_avalanche() {
    const std::size_t lengths[4] = {10, 100, 500, 1000};
    const std::size_t per_length = 25000;  // 4 x 25 000 = 100 000 porų
    auto hashes = all_hashes();

    std::ofstream hist_csv(kResults + "exp6_hist.csv");
    hist_csv << "hash,mode,bits,bin_bits,bin_percent,count\n";
    std::ostringstream md;
    md << "# 6 eksperimentas. Lavinos efektas\n\n"
       << "100 000 porų (po 25 000 kiekvienam ilgiui 10, 100, 500, 1000), abėcėlė `!`..`~`, seed = " << kSeed << " + ilgis + 1.\n"
       << "Kiekvienoje poroje vienas atsitiktinai parinktas simbolis pakeistas kitu abėcėlės simboliu (ilgis nekinta).\n"
       << "Bitų skirtumas skaičiuojamas iš dekoduotų baitų (XOR + bitų skaičiavimas); hex skirtumas – pagal 4 bitų grupes.\n"
       << "Procentai normalizuoti pagal kiekvienos funkcijos ilgį (Ratas 256, MD5 128, SHA-1 160, SHA-256 256 bitų).\n"
       << "Papildomai: tos pačios poros, bet apverčiamas tiksliai vienas atsitiktinis įvesties bitas (baitų režimas).\n\n";

    for (const char* mode : {"simbolis", "bitas"}) {
        md << "## Pakeitimas: vienas " << mode << "\n\n"
           << "| Funkcija | Ilgis | Porų | Bitų skirt. min % | vid % | max % | Hex skirt. min % | vid % | max % |\n"
           << "|---|---:|---:|---:|---:|---:|---:|---:|---:|\n";
        for (auto& h : hashes) {
            Stat all_bits, all_hex;
            std::vector<std::size_t> hist(h.bits + 1, 0);  // pagal tikslų besiskiriančių bitų skaičių
            std::ostringstream rows;
            for (std::size_t L : lengths) {
                Rng rng(kSeed + L + 1);
                Stat sb, sh;
                for (std::size_t p = 0; p < per_length; ++p) {
                    std::string a = random_string(rng, L), b = a;
                    std::size_t pos = rng.below(L);
                    if (std::strcmp(mode, "simbolis") == 0) {
                        char nc;
                        do { nc = kAlphabet[rng.below(kAlphabet.size())]; } while (nc == a[pos]);
                        b[pos] = nc;
                    } else {
                        b[pos] = static_cast<char>(b[pos] ^ (1 << rng.below(8)));
                    }
                    Bytes da = h(a), db = h(b);
                    double bp = 100.0 * bit_diff(da, db) / static_cast<double>(h.bits);
                    double hp = 100.0 * hex_diff(da, db) / static_cast<double>(h.bits / 4);
                    sb.add(bp); sh.add(hp); all_bits.add(bp); all_hex.add(hp);
                    ++hist[static_cast<std::size_t>(bit_diff(da, db))];
                }
                rows << "| " << h.name << " | " << L << " | " << per_length << " | " << fmt(sb.mn) << " | " << fmt(sb.mean()) << " | " << fmt(sb.mx)
                     << " | " << fmt(sh.mn) << " | " << fmt(sh.mean()) << " | " << fmt(sh.mx) << " |\n";
            }
            rows << "| **" << h.name << "** | **visi** | " << all_bits.n << " | " << fmt(all_bits.mn) << " | **" << fmt(all_bits.mean()) << "** | " << fmt(all_bits.mx)
                 << " | " << fmt(all_hex.mn) << " | **" << fmt(all_hex.mean()) << "** | " << fmt(all_hex.mx) << " |\n";
            md << rows.str();
            for (std::size_t b = 0; b <= h.bits; ++b)
                if (hist[b]) hist_csv << h.name << ',' << mode << ',' << h.bits << ',' << b << ','
                                      << fmt(100.0 * b / h.bits, 3) << ',' << hist[b] << '\n';
            std::cout << h.name << " (" << mode << "): bitų vid. " << fmt(all_bits.mean()) << "%, hex vid. " << fmt(all_hex.mean()) << "%\n";
        }
        md << "\n";
    }
    md << "Histogramos duomenys: `exp6_hist.csv` (kiekviena eilutė – tikslus besiskiriančių bitų skaičius `bin_bits`, jo procentas ir porų skaičius).\n";
    write_file(kResults + "exp6_avalanche.md", md.str());
}


// ---------------------------------------------------------------------------
// Papildomas matavimas – difuzija per vieną pasukimą (pagrindžia pasukimų skaičių)
// ---------------------------------------------------------------------------

void exp_diffusion() {
    // Atsitiktinė būsena, į ją įvedamas vieno bito skirtumas (viena iš 128 bloko
    // pozicijų, t. y. stipinai 0..3), tada abu variantai sukami t kartų ir
    // skaičiuojama, kiek iš 256 būsenos bitų skiriasi. Idealu – apie 50 %.
    Rng rng(kSeed + 6);
    const int samples = 2000, max_turns = 4;
    std::ostringstream md;
    md << "# Difuzija per pasukimus (papildomas matavimas)\n\n"
       << "Atsitiktinė 256 bitų būsena (seed " << kSeed + 6 << "), vienas bito skirtumas bloko įvedimo vietoje\n"
       << "(stipinai 0..3, visos 128 pozicijos po lygiai), abu variantai pasukami t kartų. Skaičiuojama, kiek iš 256\n"
       << "būsenos bitų skiriasi (" << samples << " imtys kiekvienam t). Tai vidinės būsenos, ne santraukos, matavimas.\n\n"
       << "| Pasukimų t | Skirt. bitų min | vid. | max | vid. % |\n|---:|---:|---:|---:|---:|\n";
    for (int t = 1; t <= max_turns; ++t) {
        Stat st;
        for (int n = 0; n < samples; ++n) {
            std::uint32_t a[8], b[8];
            for (auto& w : a) w = static_cast<std::uint32_t>(rng.next());
            std::memcpy(b, a, sizeof a);
            std::size_t bit = static_cast<std::size_t>(n) % 128;
            b[bit / 32] ^= 1u << (bit % 32);
            for (int k = 0; k < t; ++k) { ratas::detail::turn(a, static_cast<unsigned>(k)); ratas::detail::turn(b, static_cast<unsigned>(k)); }
            int d = 0;
            for (int i = 0; i < 8; ++i) { std::uint32_t x = a[i] ^ b[i]; while (x) { d += x & 1; x >>= 1; } }
            st.add(d);
        }
        md << "| " << t << " | " << st.mn << " | " << fmt(st.mean(), 1) << " | " << st.mx << " | " << fmt(100.0 * st.mean() / 256) << " |\n";
        std::cout << "difuzija t=" << t << ": vid. " << fmt(st.mean(), 1) << " bitų\n";
    }
    write_file(kResults + "exp_diffusion.md", md.str());
}

// ---------------------------------------------------------------------------
// 7 eksperimentas – spėjimas, vieša druska, slaptas atsitiktinumas
// ---------------------------------------------------------------------------

void exp_preimage() {
    using clock = std::chrono::steady_clock;
    const std::string target = "7391";  // pasirinkta tikslinė įvestis (užpuolikas jos nežino)
    std::vector<std::string> candidates;
    for (int i = 0; i < 10000; ++i) { char b[8]; std::snprintf(b, sizeof b, "%04d", i); candidates.push_back(b); }

    std::ostringstream md;
    md << "# 7 eksperimentas. Spėjimas, vieša druska ir slaptas atsitiktinumas\n\n"
       << "Kandidatų rinkinys: visos keturių skaitmenų eilutės `0000`..`9999` (10 000). Tikslinė įvestis pasirinkta iš\n"
       << "anksto; atakai duodama tik jos santrauka (ir druska, kai ji vieša). Maiša: Ratas v0.1.\n\n";

    // 1. Be druskos.
    {
        Bytes goal = ratas::hash(target);
        auto t0 = clock::now();
        std::size_t tries = 0, first = 0;
        std::vector<std::string> matches;
        for (auto& c : candidates) { ++tries; if (ratas::hash(c) == goal) { if (matches.empty()) first = tries; matches.push_back(c); } }
        double us = std::chrono::duration<double, std::micro>(clock::now() - t0).count();
        md << "## 1. Be druskos: H(input)\n\n"
           << "- Tikslinė santrauka: `" << hex(goal) << "`\n"
           << "- Bandymų: " << tries << " (visas rinkinys perrinktas), pirmas sutapimas ties bandymu nr. " << first << "\n"
           << "- Laikas: " << fmt(us, 0) << " µs (" << fmt(us / tries, 2) << " µs vienam kandidatui)\n"
           << "- Sutampantys kandidatai: ";
        for (auto& m : matches) md << "`" << m << "` ";
        md << "\n\nSutapimas nebūtinai identifikuoja pradinę įvestį: rasta tik įvestis iš kandidatų rinkinio, kurios\n"
           << "santrauka sutampa. Tai gali būti ir kita įvestis su ta pačia santrauka (kolizija), tačiau 256 bitų\n"
           << "maišai su 10 000 kandidatų atsitiktinė kolizija itin mažai tikėtina, todėl praktiškai tai ta pati įvestis.\n\n";
    }

    // 2. Vieša druska: 8 baitų, iš seed generatoriaus, pridedama kaip tikslūs baitai (input || salt).
    {
        Rng rng(kSeed + 7);
        std::string salt;
        for (int i = 0; i < 8; ++i) salt.push_back(static_cast<char>(rng.below(256)));
        Bytes salt_bytes(salt.begin(), salt.end());
        Bytes goal = ratas::hash(target + salt);
        auto t0 = clock::now();
        std::size_t tries = 0;
        std::vector<std::string> matches;
        for (auto& c : candidates) { ++tries; if (ratas::hash(c + salt) == goal) matches.push_back(c); }
        double us = std::chrono::duration<double, std::micro>(clock::now() - t0).count();

        // Iš anksto apskaičiuota lentelė vienai druskai – ar tinka kitai?
        std::unordered_map<std::string, std::string> table;
        for (auto& c : candidates) { Bytes d = ratas::hash(c + salt); table[std::string(d.begin(), d.end())] = c; }
        std::string salt2;
        for (int i = 0; i < 8; ++i) salt2.push_back(static_cast<char>(rng.below(256)));
        Bytes goal2 = ratas::hash(target + salt2);
        bool reusable = table.count(std::string(goal2.begin(), goal2.end())) > 0;

        md << "## 2. Vieša druska: H(input || salt)\n\n"
           << "- Druska: 8 atsitiktiniai baitai (seed " << kSeed + 7 << "), pridedami PO įvesties kaip tikslūs baitai (ne hex tekstas).\n"
           << "  Užrašoma hex: `" << hex(salt_bytes) << "`. Užpuolikas druską žino.\n"
           << "- Tikslinė santrauka: `" << hex(goal) << "`\n"
           << "- Bandymų: " << tries << ", laikas: " << fmt(us, 0) << " µs, sutapimai: ";
        for (auto& m : matches) md << "`" << m << "` ";
        md << "\n- Pastangos vienam taikiniui: tokios pat kaip be druskos (10 000 maišų) – vieša druska nuo perrinkimo neapsaugo.\n"
           << "- Iš anksto apskaičiuota 10 000 įrašų lentelė su druska `" << hex(salt_bytes) << "` pritaikyta taikiniui su kita druska `"
           << hex(Bytes(salt2.begin(), salt2.end())) << "`: " << (reusable ? "RASTA (netikėta!)" : "nerasta")
           << ". Kiekvienai druskai lentelę reikia skaičiuoti iš naujo, todėl druska naikina pakartotinį naudojimą (rainbow tables), bet ne perrinkimą.\n\n";
    }

    // 3. Slaptas atsitiktinumas r (2 baitai = 65 536 galimybių): H(input || r).
    {
        Rng rng(kSeed + 77);
        std::string r;
        for (int i = 0; i < 2; ++i) r.push_back(static_cast<char>(rng.below(256)));
        Bytes goal = ratas::hash(target + r);
        // Dalinis perrinkimas: tik 1/64 r erdvės (1024 r reikšmės) x 10 000 kandidatų, laikas ekstrapoliuojamas.
        auto t0 = clock::now();
        std::size_t tries = 0;
        bool found = false;
        for (int rv = 0; rv < 1024 && !found; ++rv) {
            std::string rr{static_cast<char>(rv & 255), static_cast<char>(rv >> 8)};
            for (auto& c : candidates) { ++tries; if (ratas::hash(c + rr) == goal) { found = true; break; } }
        }
        double us = std::chrono::duration<double, std::micro>(clock::now() - t0).count();
        double full_s = us / tries * (10000.0 * 65536.0) / 1e6;
        bool verify = ratas::hash(target + r) == goal;
        md << "## 3. Slaptas atsitiktinumas: H(input || r), r nežinomas\n\n"
           << "- r – 2 slapti baitai (65 536 galimybių). Paieškos erdvė: 10 000 × 65 536 = 655 360 000 porų (input, r).\n"
           << "- Perrinkta 1/64 r erdvės: " << tries << " bandymų per " << fmt(us / 1000, 1) << " ms, tikslas " << (found ? "rastas" : "nerastas")
           << "; visa erdvė užtruktų apie " << fmt(full_s, 1) << " s (65 536 kartų daugiau nei be r).\n"
           << "- Su 16 slaptų baitų r (2^128) perrinkimas būtų praktiškai neįmanomas – r paslepia lengvai spėjamą įvestį.\n"
           << "- Atskleidus r = `" << hex(Bytes(r.begin(), r.end())) << "`, patikrinimas yra viena maiša: H(`" << target << "` || r) == tikslas -> "
           << (verify ? "taip" : "ne") << ".\n\n"
           << "Tai iliustruoja įsipareigojimo (commitment) idėją: paskelbus H(input || r) įvestis lieka paslėpta, o vėliau atskleidus\n"
           << "(input, r) kiekvienas gali ją patikrinti. Tai NEĮRODO, kad Ratas-256 saugiai slepia pranešimą ar neleidžia rasti kitos\n"
           << "poros (input', r') su ta pačia santrauka – tam reiktų kriptografinės analizės.\n";
    }
    write_file(kResults + "exp7_preimage.md", md.str());
    std::cout << "exp7 baigtas\n";
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "naudojimas: experiments <inputs|correctness|determinism|bench|collisions|avalanche|diffusion|preimage|all> [--no-std]\n";
        return 1;
    }
    for (int i = 2; i < argc; ++i) if (std::string(argv[i]) == "--no-std") g_use_std = false;
    std::string cmd = argv[1];
    bool all = cmd == "all";
    if (all || cmd == "inputs") exp_inputs();
    if (all || cmd == "correctness") exp_correctness();
    if (all || cmd == "determinism") exp_determinism();
    if (all || cmd == "bench") exp_bench();
    if (all || cmd == "collisions") exp_collisions();
    if (all || cmd == "avalanche") exp_avalanche();
    if (all || cmd == "diffusion") exp_diffusion();
    if (all || cmd == "preimage") exp_preimage();
    return 0;
}
