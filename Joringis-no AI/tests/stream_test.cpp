#define main ratas_cli_main_unused
#include "../ratas.cpp"
#undef main

#include <random>

int main() {
    std::mt19937_64 rng(20260930);
    std::vector<size_t> lengths;
    for (size_t n = 0; n <= 600; ++n) lengths.push_back(n);
    for (size_t n : {4095, 4096, 4097, 65535, 65536, 65537, 1000003}) lengths.push_back(n);
    int bad = 0;
    for (size_t n : lengths) {
        std::vector<uint8_t> data(n);
        for (size_t i = 0; i < n; ++i) data[i] = static_cast<uint8_t>(i * 131 + n);
        const std::string one = to_hex(ratas256(data.data(), n));
        for (int t = 0; t < 5; ++t) {
            Ratas256 h;
            size_t pos = 0;
            while (pos < n) {
                size_t step = static_cast<size_t>(rng() % 41);
                if (step > n - pos) step = n - pos;
                h.update(data.data() + pos, step);
                pos += step;
            }
            h.update(nullptr, 0);
            if (to_hex(h.finish()) != one) ++bad;
        }
        std::cout << n << ' ' << one << '\n';
    }
    std::cout << "chunk_mismatches " << bad << '\n';
    return bad == 0 ? 0 : 1;
}
