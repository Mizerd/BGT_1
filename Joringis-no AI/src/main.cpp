// Komandinės eilutės programa: surenka baitus ir spausdina Ratas-256 santrauką.
//
// Režimai:
//   ratas                       – tekstas įvedamas ranka (Enter naujos eilutės NEįtraukiama)
//   ratas --text "<tekstas>"    – argumento baitai (UTF-8) maišomi tokie, kokie yra
//   ratas --file <kelias>       – failo TURINYS (ne pavadinimas), tikslūs baitai
//   ratas --stdin               – visi standartinės įvesties baitai (dvejetainiu režimu)
// Papildomai: --quiet (spausdinti tik hex).
//
// Išėjimo kodai: 0 – pavyko, 1 – blogi argumentai, 2 – failo nepavyko perskaityti.

#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "ratas.hpp"

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#endif

namespace {

void usage(std::ostream& out) {
    out << "Naudojimas:\n"
           "  ratas                      tekstas ivedamas ranka (Enter neitraukiamas)\n"
           "  ratas --text \"<tekstas>\"   maisomi argumento baitai (UTF-8)\n"
           "  ratas --file <kelias>      maisomas failo turinys (tikslus baitai)\n"
           "  ratas --stdin              maisomi visi stdin baitai\n"
           "Parinktys: --quiet (spausdinti tik hex)\n";
}

// Failas skaitomas dvejetainiu režimu, be jokio eilučių pabaigų keitimo.
// Neatidaromas ar nepilnai perskaitytas failas -> false (ne tuščia įvestis!).
bool read_file(const std::string& path, std::vector<std::uint8_t>& bytes) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return false;
    char buf[1 << 16];
    while (f.read(buf, sizeof(buf)) || f.gcount() > 0)
        bytes.insert(bytes.end(), buf, buf + f.gcount());
    return !f.bad();
}

bool read_stdin(std::vector<std::uint8_t>& bytes) {
#ifdef _WIN32
    _setmode(_fileno(stdin), _O_BINARY);
#endif
    char buf[1 << 16];
    std::size_t n;
    while ((n = std::fread(buf, 1, sizeof(buf), stdin)) > 0)
        bytes.insert(bytes.end(), buf, buf + n);
    return !std::ferror(stdin);
}

#ifdef _WIN32
std::string utf16_to_utf8(const wchar_t* w, int wlen) {
    if (wlen == 0) return std::string();
    const int n = WideCharToMultiByte(CP_UTF8, 0, w, wlen, nullptr, 0, nullptr, nullptr);
    std::string s(static_cast<std::size_t>(n), '\0');
    WideCharToMultiByte(CP_UTF8, 0, w, wlen, &s[0], n, nullptr, nullptr);
    return s;
}
#endif

// Rankinis įvedimas: viena eilutė iki Enter. Naujos eilutės simbolis (\n arba
// \r\n) į maišą NEĮTRAUKIAMAS. Windows konsolėje skaitoma UTF-16 ir verčiama
// į UTF-8, kad ne ASCII raidės (ą, č, ...) būtų maišomos taip pat kaip faile.
bool read_line_manual(std::string& line) {
#ifdef _WIN32
    HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode = 0;
    if (GetConsoleMode(h, &mode)) {
        std::wstring w;
        wchar_t buf[512];
        DWORD got = 0;
        for (;;) {
            if (!ReadConsoleW(h, buf, 512, &got, nullptr) || got == 0) break;
            w.append(buf, got);
            if (!w.empty() && w.back() == L'\n') break;
        }
        while (!w.empty() && (w.back() == L'\n' || w.back() == L'\r')) w.pop_back();
        line = utf16_to_utf8(w.data(), static_cast<int>(w.size()));
        return true;
    }
#endif
    if (!std::getline(std::cin, line)) line.clear();
    if (!line.empty() && line.back() == '\r') line.pop_back();
    return true;
}

int run(const std::vector<std::string>& args) {
    std::string mode = "manual";
    std::string value;
    bool quiet = false;

    for (std::size_t i = 1; i < args.size(); ++i) {
        const std::string& a = args[i];
        if (a == "--help" || a == "-h") { usage(std::cout); return 0; }
        else if (a == "--quiet") quiet = true;
        else if (a == "--stdin") mode = "stdin";
        else if ((a == "--text" || a == "--file") && i + 1 < args.size()) {
            mode = a.substr(2);
            value = args[++i];
        } else { usage(std::cerr); return 1; }
    }

    std::vector<std::uint8_t> bytes;
    if (mode == "text") {
        bytes.assign(value.begin(), value.end());
    } else if (mode == "file") {
        if (!read_file(value, bytes)) {
            std::cerr << "Klaida: nepavyko perskaityti failo: " << value << '\n';
            return 2;
        }
    } else if (mode == "stdin") {
        if (!read_stdin(bytes)) {
            std::cerr << "Klaida: nepavyko perskaityti stdin\n";
            return 2;
        }
    } else {
        if (!quiet) std::cout << "Iveskite teksta ir spauskite Enter (Enter neitraukiamas):\n";
        std::string line;
        read_line_manual(line);
        bytes.assign(line.begin(), line.end());
    }

    const std::string hex = ratas::to_hex(ratas::hash(bytes));
    if (quiet) {
        std::cout << hex << '\n';
    } else {
        std::cout << "Rezimas: " << mode << "  | versija: " << ratas::kVersion
                  << "  | ivesties baitu: " << bytes.size() << '\n'
                  << hex << '\n';
    }
    return 0;
}

}  // namespace

#ifdef _WIN32
// Windows'e argumentai paimami UTF-16 pavidalu ir verčiami į UTF-8, kad
// --text "ąčę" maišytų tuos pačius baitus kaip UTF-8 failas su tuo tekstu.
int wmain(int argc, wchar_t** argv) {
    std::vector<std::string> args;
    for (int i = 0; i < argc; ++i)
        args.push_back(utf16_to_utf8(argv[i], static_cast<int>(wcslen(argv[i]))));
    return run(args);
}
#else
int main(int argc, char** argv) {
    std::vector<std::string> args(argv, argv + argc);
    return run(args);
}
#endif
