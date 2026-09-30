#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#pragma comment(lib, "shell32.lib")
#endif

using namespace std;

namespace {

constexpr size_t kSpokes = 8;
constexpr size_t kBlockBytes = 16;
constexpr size_t kDigestBytes = 32;

constexpr int kRoundsPerBlock = 2;
constexpr int kRoundsFinal = 4;
constexpr int kRoundsSqueeze = 2;

constexpr char kSeedPhrase[kSpokes * 4 + 1] = "Vilniaus universitetas, BGT 2026";

using State = uint32_t[kSpokes];

uint32_t rotl32(uint32_t x, unsigned n) {
    n &= 31u;
    if (n == 0) return x;
    return (x << n) | (x >> (32u - n));
}

uint32_t load_le32(const uint8_t* b) {
    return static_cast<uint32_t>(b[0]) |
           (static_cast<uint32_t>(b[1]) << 8) |
           (static_cast<uint32_t>(b[2]) << 16) |
           (static_cast<uint32_t>(b[3]) << 24);
}

void store_le32(uint32_t w, uint8_t* b) {
    b[0] = static_cast<uint8_t>(w);
    b[1] = static_cast<uint8_t>(w >> 8);
    b[2] = static_cast<uint8_t>(w >> 16);
    b[3] = static_cast<uint8_t>(w >> 24);
}

constexpr uint32_t seed_word(size_t i) {
    return static_cast<uint32_t>(static_cast<unsigned char>(kSeedPhrase[4 * i])) |
           (static_cast<uint32_t>(static_cast<unsigned char>(kSeedPhrase[4 * i + 1])) << 8) |
           (static_cast<uint32_t>(static_cast<unsigned char>(kSeedPhrase[4 * i + 2])) << 16) |
           (static_cast<uint32_t>(static_cast<unsigned char>(kSeedPhrase[4 * i + 3])) << 24);
}

constexpr uint32_t kIV[kSpokes] = {
    seed_word(0), seed_word(1), seed_word(2), seed_word(3),
    seed_word(4), seed_word(5), seed_word(6), seed_word(7),
};

uint32_t round_const(unsigned r) {
    return kIV[r % kSpokes] * (2u * r + 1u) + r;
}

void turn(State& s, unsigned r) {
    const uint32_t rc = round_const(r);
    for (size_t i = 0; i < kSpokes; ++i) {
        const uint32_t b = s[(i + 1) % kSpokes];
        const uint32_t c = s[(i + 3) % kSpokes];
        const uint32_t d = s[(i + 6) % kSpokes];
        uint32_t a = s[i];
        a += (b ^ c);
        const unsigned rot = d & 31u;
        a = rotl32(a, rot + (rot == 0u));
        a ^= (d + rc);
        s[i] = a;
        s[(i + 1) % kSpokes] += rotl32(a, 9);
    }
}

void turns(State& s, int count, unsigned& counter) {
    for (int k = 0; k < count; ++k) turn(s, counter++);
}

void absorb_block(State& s, const uint8_t* block, uint64_t block_index,
                  unsigned& counter) {
    State h;
    for (size_t i = 0; i < kSpokes; ++i) h[i] = s[i];
    s[0] ^= load_le32(block);
    s[1] ^= load_le32(block + 4);
    s[2] ^= load_le32(block + 8);
    s[3] ^= load_le32(block + 12);
    s[6] += static_cast<uint32_t>(block_index >> 32);
    s[7] += static_cast<uint32_t>(block_index);
    turns(s, kRoundsPerBlock, counter);
    for (size_t i = 0; i < kSpokes; ++i) s[i] += h[i];
}

vector<uint8_t> ratas256(const uint8_t* data, size_t size) {
    State s;
    for (size_t i = 0; i < kSpokes; ++i) s[i] = kIV[i];
    unsigned counter = 0;

    const size_t full_blocks = size / kBlockBytes;
    uint64_t index = 1;
    for (size_t i = 0; i < full_blocks; ++i, ++index)
        absorb_block(s, data + i * kBlockBytes, index, counter);

    uint8_t last[kBlockBytes];
    const size_t rest = size - full_blocks * kBlockBytes;
    const uint8_t pad = static_cast<uint8_t>(kBlockBytes - rest);
    for (size_t i = 0; i < kBlockBytes; ++i)
        last[i] = (i < rest) ? data[full_blocks * kBlockBytes + i] : pad;
    absorb_block(s, last, index, counter);

    const uint64_t len = static_cast<uint64_t>(size);
    s[4] ^= static_cast<uint32_t>(len);
    s[5] ^= static_cast<uint32_t>(len >> 32);
    s[6] ^= 0xFFFFFFFFu;
    turns(s, kRoundsFinal, counter);

    vector<uint8_t> out(kDigestBytes);
    for (size_t i = 0; i < 4; ++i) store_le32(s[i], &out[4 * i]);
    turns(s, kRoundsSqueeze, counter);
    for (size_t i = 0; i < 4; ++i) store_le32(s[i], &out[16 + 4 * i]);
    return out;
}

string to_hex(const vector<uint8_t>& d) {
    static const char* digits = "0123456789abcdef";
    string s;
    s.reserve(d.size() * 2);
    for (uint8_t b : d) {
        s.push_back(digits[b >> 4]);
        s.push_back(digits[b & 0x0F]);
    }
    return s;
}

#ifdef _WIN32
string utf8_from_wide(const wstring& w) {
    if (w.empty()) return string();
    const int n = WideCharToMultiByte(CP_UTF8, 0, w.data(), static_cast<int>(w.size()),
                                      nullptr, 0, nullptr, nullptr);
    string s(static_cast<size_t>(n), 0);
    WideCharToMultiByte(CP_UTF8, 0, w.data(), static_cast<int>(w.size()),
                        &s[0], n, nullptr, nullptr);
    return s;
}

wstring wide_from_utf8(const string& s) {
    if (s.empty()) return wstring();
    const int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()),
                                      nullptr, 0);
    wstring w(static_cast<size_t>(n), 0);
    MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), &w[0], n);
    return w;
}
#endif

filesystem::path to_path(const string& utf8) {
#ifdef _WIN32
    return filesystem::path(wide_from_utf8(utf8));
#else
    return filesystem::path(utf8);
#endif
}

bool read_file(const string& path, vector<uint8_t>& bytes) {
    ifstream f(to_path(path), ios::binary);
    if (!f) return false;
    char buf[1 << 16];
    while (f.read(buf, sizeof(buf)) || f.gcount() > 0)
        bytes.insert(bytes.end(), buf, buf + f.gcount());
    return !f.bad();
}

string read_line() {
#ifdef _WIN32
    HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode = 0;
    if (GetConsoleMode(h, &mode)) {
        wstring w;
        wchar_t buf[512];
        DWORD got = 0;
        while (ReadConsoleW(h, buf, 512, &got, nullptr) && got > 0) {
            w.append(buf, got);
            if (w.back() == L'\n') break;
        }
        while (!w.empty() && (w.back() == L'\n' || w.back() == L'\r')) w.pop_back();
        return utf8_from_wide(w);
    }
#endif
    string line;
    if (!getline(cin, line)) line.clear();
    if (!line.empty() && line.back() == '\r') line.pop_back();
    return line;
}
void hold_console() {
#ifdef _WIN32
    DWORD pids[2];
    if (GetConsoleProcessList(pids, 2) == 1) {
        cout << "\nSpauskite Enter, kad uzdarytumete langa...";
        cin.get();
    }
#endif
}

int run(int argc, char** argv) {
    vector<uint8_t> bytes;
    string mode;

    if (argc == 2) {
        string path = argv[1];
#ifdef _WIN32
        int wargc = 0;
        if (LPWSTR* wargv = CommandLineToArgvW(GetCommandLineW(), &wargc)) {
            if (wargc == 2) path = utf8_from_wide(wargv[1]);
            LocalFree(wargv);
        }
#endif
        mode = "failas: " + path;
        if (!read_file(path, bytes)) {
            cerr << "Klaida: nepavyko perskaityti failo: " << path << '\n';
            return 2;
        }
    } else if (argc == 1) {
        cout << "Pasirinkite rezima:\n"
                "  1 - ivesti teksta ranka\n"
                "  2 - maisyti faila\n"
                "Pasirinkimas: ";
        const string choice = read_line();

        if (choice == "1") {
            mode = "rankinis ivedimas";
            cout << "Iveskite teksta ir spauskite Enter (Enter neitraukiamas):\n";
            const string line = read_line();
            bytes.assign(line.begin(), line.end());
        } else if (choice == "2") {
            cout << "Failo kelias: ";
            string path = read_line();
            if (path.size() >= 2 && path.front() == '"' && path.back() == '"')
                path = path.substr(1, path.size() - 2);
            mode = "failas: " + path;
            if (!read_file(path, bytes)) {
                cerr << "Klaida: nepavyko perskaityti failo: " << path << '\n';
                return 2;
            }
        } else {
            cerr << "Klaida: reikia pasirinkti 1 arba 2\n";
            return 1;
        }
    } else {
        cerr << "Naudojimas: ratas [failas]\n";
        return 1;
    }

    cout << "Rezimas: " << mode << '\n'
         << "Ivesties baitu: " << bytes.size() << '\n'
         << "Ratas-256: " << to_hex(ratas256(bytes.data(), bytes.size())) << '\n';
    return 0;
}

}

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    const int code = run(argc, argv);
    hold_console();
    return code;
}
