// Greitos teisingumo patikros per API (ne statistiniai eksperimentai).
// Paleidimas: build/sanity  (grąžina 0, jei visos patikros praėjo).

#include <iostream>
#include <string>

#include "ratas.hpp"

namespace {

int checks = 0, failures = 0;

void check(bool ok, const std::string& what) {
    ++checks;
    if (!ok) ++failures;
    std::cout << (ok ? "ok    " : "FAIL  ") << what << '\n';
}

std::string H(const std::string& s) {
    return ratas::to_hex(ratas::hash(s));
}

bool is_hex64(const std::string& h) {
    if (h.size() != 64) return false;
    for (char c : h) if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) return false;
    return true;
}

}  // namespace

int main() {
    // Forma
    check(ratas::hash("").size() == 32, "tuscia ivestis duoda 32 baitus");
    check(is_hex64(H("")), "tuscia ivestis: 64 mazosios hex raides");
    check(is_hex64(H("a")) && is_hex64(H(std::string(100000, 'z'))), "hex forma trumpai ir ilgai ivesciai");

    // Determinizmas
    check(H("labas") == H("labas"), "pakartotinis kvietimas sutampa");
    std::string a1 = H("A");
    H("B");
    check(H("A") == a1, "seka A, B, A - jokios isliekancios busenos");

    // Jautrumas įvesčiai
    check(H("a") != H("b"), "a != b");
    check(H("abc") != H("cba"), "simboliu tvarka svarbi");
    check(H("tekstas") != H("tekstas\n"), "naujos eilutes simbolis keicia santrauka");
    check(H("tekstas") != H(" tekstas") && H("tekstas") != H("tekstas "), "tarpai pradzioje/pabaigoje svarbus");
    check(H("Labas") != H("labas"), "raidziu registras svarbus");
    check(H("ab") != H(std::string("ab\0", 3)), "ab != ab\\0 (uzpildas)");
    check(H("") != H(std::string(1, '\0')), "tuscia != vienas nulinis baitas");
    check(H(std::string(16, 'x')) != H(std::string(15, 'x')) && H(std::string(16, 'x')) != H(std::string(17, 'x')),
          "bloko ribos 15/16/17");
    check(H(std::string(32, 'x')) != H(std::string(16, 'x')), "du vienodi blokai != vienas blokas");
    // Užpildo apgaulė: "ab" + 14 baitų 0x0E turi skirtis nuo "ab" (užpildas dedamas visada, plius ilgis).
    check(H("ab") != H("ab" + std::string(14, static_cast<char>(0x0E))), "ab != ab + 14 x 0x0E");

    // Rezultatas nuskaitomas dviem pusėmis – jos neturi sutapti.
    std::string h = H("puses");
    check(h.substr(0, 32) != h.substr(32), "abi santraukos puses skirtingos");

    std::cout << checks << " patikru, " << failures << " nepavyko\n";
    return failures == 0 ? 0 : 1;
}
