import struct
import sys

M = 0xFFFFFFFF
IV = list(struct.unpack("<8I", b"Vilniaus universitetas, BGT 2026"))
ROUNDS_PER_BLOCK = 3
ROUNDS_FINAL = 4
ROUNDS_SQUEEZE = 2


def rotl(x, n):
    return ((x << n) | (x >> (32 - n))) & M


def turn(s, r):
    rc = (IV[r % 8] * (2 * r + 1) + r) & M
    for i in range(8):
        b, c, d = s[(i + 1) % 8], s[(i + 3) % 8], s[(i + 6) % 8]
        a = (s[i] + (b ^ c)) & M
        a = rotl(a, (d & 31) or 1)
        a ^= (d + rc) & M
        s[i] = a
        s[(i + 1) % 8] = (s[(i + 1) % 8] + rotl(a, 9)) & M


class Ratas:
    def __init__(self):
        self.s = IV[:]
        self.r = 0

    def turns(self, n):
        for _ in range(n):
            turn(self.s, self.r)
            self.r += 1

    def block(self, blk, n):
        h = self.s[:]
        w = struct.unpack("<4I", blk)
        for i in range(4):
            self.s[i] ^= w[i]
        self.s[6] = (self.s[6] + (n >> 32)) & M
        self.s[7] = (self.s[7] + n) & M
        self.turns(ROUNDS_PER_BLOCK)
        self.s = [(x + y) & M for x, y in zip(self.s, h)]


def ratas256(data):
    st = Ratas()
    full = len(data) // 16
    for k in range(full):
        st.block(data[16 * k:16 * k + 16], k + 1)
    rest = data[16 * full:]
    pad = 16 - len(rest)
    st.block(rest + bytes([pad]) * pad, full + 1)
    L = len(data)
    st.s[4] ^= L & M
    st.s[5] ^= (L >> 32) & M
    st.s[6] ^= M
    f = st.s[:]
    st.turns(ROUNDS_FINAL)
    st.s = [(x + y) & M for x, y in zip(st.s, f)]
    out = struct.pack("<4I", *st.s[:4])
    st.turns(ROUNDS_SQUEEZE)
    out += struct.pack("<4I", *st.s[:4])
    return out.hex()


if __name__ == "__main__":
    for p in sys.argv[1:]:
        print(ratas256(open(p, "rb").read()), p)
