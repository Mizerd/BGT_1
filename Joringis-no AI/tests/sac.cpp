#define main ratas_cli_main_unused
#include "../ratas.cpp"
#undef main

#include <bit>
#include <cmath>
#include <cstdio>
#include <random>

void block_step(State& s, const uint8_t* block, int rounds, unsigned counter) {
    State h;
    for (size_t i = 0; i < kSpokes; ++i) h[i] = s[i];
    for (size_t i = 0; i < 4; ++i) s[i] ^= load_le32(block + 4 * i);
    s[7] += 5;
    turns(s, rounds, counter);
    for (size_t i = 0; i < kSpokes; ++i) s[i] += h[i];
}

int main() {
    const int trials = 4000;
    const int in_bits = static_cast<int>(kBlockBytes * 8);
    const int out_bits = 256;
    const double limit = 5 * 0.5 / std::sqrt(static_cast<double>(trials));
    std::printf("posukiai,vid_bitu,min_bitu,blogu_langeliu,langeliu,didziausias_nuokrypis\n");
    for (int rounds = 1; rounds <= 4; ++rounds) {
        std::mt19937_64 rng(20260930);
        std::vector<int> cnt(static_cast<size_t>(in_bits * out_bits), 0);
        double sum = 0;
        int min_bits = out_bits;
        for (int bit = 0; bit < in_bits; ++bit)
            for (int t = 0; t < trials; ++t) {
                State a, b;
                uint8_t m1[kBlockBytes], m2[kBlockBytes];
                for (size_t i = 0; i < kSpokes; ++i) a[i] = b[i] = static_cast<uint32_t>(rng());
                for (size_t i = 0; i < kBlockBytes; ++i) m1[i] = m2[i] = static_cast<uint8_t>(rng());
                m2[bit / 8] ^= static_cast<uint8_t>(1u << (bit % 8));
                block_step(a, m1, rounds, 7);
                block_step(b, m2, rounds, 7);
                int d = 0;
                for (int o = 0; o < out_bits; ++o) {
                    const int x = ((a[o / 32] ^ b[o / 32]) >> (o % 32)) & 1;
                    cnt[static_cast<size_t>(bit * out_bits + o)] += x;
                    d += x;
                }
                sum += d;
                if (d < min_bits) min_bits = d;
            }
        int biased = 0;
        double worst = 0;
        for (int c : cnt) {
            const double dev = std::fabs(c / static_cast<double>(trials) - 0.5);
            if (dev > limit) ++biased;
            if (dev > worst) worst = dev;
        }
        std::printf("%d,%.2f,%d,%d,%d,%.4f\n", rounds, sum / (static_cast<double>(trials) * in_bits),
                    min_bits, biased, in_bits * out_bits, worst);
    }
}
