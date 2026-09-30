#!/usr/bin/env python3
"""Breaks V0.12 (collision, second preimage, preimage) and checks the attacks fail on the current program.

V0.12 had one round per block. The words set lanes 0, 2, 4, 6 freely and every operation can be undone lane
by lane, so one block's words can be solved to put lanes 1, 3, 5, 7 anywhere; the next block sets the rest.

Usage: attack_v012.py [build directory] [data directory]   (defaults: build, ../Joringis-no AI/data)
"""

import os
import subprocess
import sys
import tempfile

MASK = (1 << 64) - 1
LANE_INIT = [0x50EFEF418AACDDB3, 0x06837D22C9EC9F1B, 0x93ED2CF209AF0CA9, 0x1511760347A610CB,
             0xE0613E1645D94F07, 0x7AA3E75445026145, 0x0976B9398CFCD1FF, 0x2B84EC77CA013781]
MUL_F, MUL_B = 0x2AD26360F20D2A3D, 0x65D3D764F8ED45F5
INV_F, INV_B = pow(MUL_F, -1, 1 << 64), pow(MUL_B, -1, 1 << 64)
TAG_STEP, TAG_TAIL, TAG_END = 0xC5E512181F592E6B, 0x9C019A9E6A275255, 0x0C2605DFE6C8B025


def rotl(x, n):
    return ((x << n) | (x >> (64 - n))) & MASK


def rotr(x, n):
    return rotl(x, 64 - n)


def inject(s, w, tag):
    return [(s[0] + (w[0] ^ tag)) & MASK, s[1], s[2] ^ w[1], s[3], (s[4] + w[2]) & MASK, s[5], s[6] ^ w[3], s[7]]


def round_v012(x):
    s = list(x)
    for i in range(1, 8):
        s[i] = ((s[i] ^ rotl(s[i - 1], 41)) * MUL_F) & MASK
    for i in range(6, -1, -1):
        s[i] = ((s[i] + rotl(s[i + 1], 53)) * MUL_B) & MASK
    return s


def unround_v012(b):
    f = list(b)
    for i in range(6, -1, -1):
        f[i] = (b[i] * INV_B - rotl(b[i + 1], 53)) & MASK
    return [f[0]] + [((f[i] * INV_F) & MASK) ^ rotl(f[i - 1], 41) for i in range(1, 8)]


def words_of(block):
    return [int.from_bytes(block[i:i + 8], "big") for i in range(0, 32, 8)]


def tag_of(position):
    return position * TAG_STEP & MASK


def hash_v012(data):
    s = list(LANE_INIT)
    n, r = divmod(len(data), 32)
    for i in range(n):
        s = round_v012(inject(s, words_of(data[32 * i:32 * i + 32]), tag_of(i + 1)))
    s = round_v012(inject(s, words_of(data[32 * n:] + bytes(32 - r)), (tag_of(n + 1) + (r + 1) * TAG_TAIL) & MASK))
    s = round_v012(inject(s, [len(data), rotl(len(data), 32), 0, 0], TAG_END))
    for j in range(1, 4):
        s = round_v012(inject(s, [0, 0, 0, 0], (TAG_END + j * TAG_STEP) & MASK))
    return b"".join((s[i] ^ rotl(s[i + 4], 40)).to_bytes(8, "big") for i in range(4)).hex()


def state_after(blocks):
    s = list(LANE_INIT)
    for i, w in enumerate(blocks):
        s = round_v012(inject(s, w, tag_of(i + 1)))
    return s


def steer(s, target, tag):
    """Block words that make lanes 1, 3, 5, 7 after the round equal target's."""
    f, b = [0] * 8, [0] * 8
    b[7] = f[7] = target[7]
    f[6] = rotr(((target[7] * INV_F) & MASK) ^ s[7], 41)
    for i in (6, 4, 2):
        b[i] = ((f[i] + rotl(b[i + 1], 53)) * MUL_B) & MASK
        b[i - 1] = target[i - 1]
        f[i - 1] = (target[i - 1] * INV_B - rotl(b[i], 53)) & MASK
        f[i - 2] = rotr(((f[i - 1] * INV_F) & MASK) ^ s[i - 1], 41)
    x = [f[0]] + [((f[k] * INV_F) & MASK) ^ rotl(f[k - 1], 41) for k in (2, 4, 6)]
    return [((x[0] - s[0]) & MASK) ^ tag, x[1] ^ s[2], (x[2] - s[4]) & MASK, x[3] ^ s[6]]


def settle(s, x, tag):
    """Block words that turn s into x; the odd lanes must already match."""
    return [((x[0] - s[0]) & MASK) ^ tag, x[2] ^ s[2], (x[4] - s[4]) & MASK, x[6] ^ s[6]]


def reach(first_block, x3):
    """Any first block, then two solved blocks so that block 3 starts from x3."""
    s1 = state_after([first_block])
    w2 = steer(s1, x3, tag_of(2))
    s2 = state_after([first_block, w2])
    return [first_block, w2, settle(s2, x3, tag_of(3))]


def to_bytes(blocks):
    return b"".join(x.to_bytes(8, "big") for w in blocks for x in w)


build = sys.argv[1] if len(sys.argv) > 1 else "build"
data = sys.argv[2] if len(sys.argv) > 2 else os.path.join("..", "Joringis-no AI", "data")
binary = os.path.join(build, "hash-generator")
book = open(os.path.join(data, "konstitucija.txt"), "rb").read()

# Collision: two different first blocks steered to the same state.
target = inject(state_after([words_of(b"A" * 32), words_of(b"B" * 32)]), words_of(b"C" * 32), tag_of(3))
suffix = "Bendra pabaiga gali būti bet kokia.\n".encode()
collision = [to_bytes(reach(words_of(first.ljust(32, b".")), target)) + suffix
             for first in (b"Pirmas failas", b"Antras failas")]

# Second preimage: a different start for konstitucija.txt, same state after 96 bytes.
original = [words_of(book[32 * i:32 * i + 32]) for i in range(3)]
goal = inject(state_after(original[:2]), original[2], tag_of(3))
forged = to_bytes(reach(words_of(b"PAKEISTA: tai ne originalas.....".ljust(32, b".")), goal)) + book[96:]

# Preimage: run the finalization backwards from digest 000...0 (hidden lanes chosen freely).
hidden = [0x1111111111111111 * k for k in (1, 2, 3, 4)]
s = [rotl(hidden[i], 40) for i in range(4)] + hidden
for j in (3, 2, 1):
    s = unround_v012(s)
    s[0] = (s[0] - ((TAG_END + j * TAG_STEP) & MASK)) & MASK
s = unround_v012(s)
s[0], s[2] = (s[0] - (96 ^ TAG_END)) & MASK, s[2] ^ rotl(96, 32)
s = unround_v012(s)
s[0] = (s[0] - ((tag_of(4) + TAG_TAIL) & MASK)) & MASK
preimage = to_bytes(reach(words_of(b"Sio failo V0.12 maisa - nuliai!!"), unround_v012(s)))


def current(content):
    with tempfile.NamedTemporaryFile() as f:
        f.write(content)
        f.flush()
        return subprocess.run([binary, "--file", f.name], capture_output=True, text=True, check=True).stdout.strip()


checks = [
    (collision[0] != collision[1] and hash_v012(collision[0]) == hash_v012(collision[1]),
     "V0.12: two different files with the same digest"),
    (forged != book and len(forged) == len(book) and hash_v012(forged) == hash_v012(book),
     "V0.12: a changed konstitucija.txt with the original digest"),
    (hash_v012(preimage) == "0" * 64, "V0.12: a 96 byte input whose digest is 64 zeros"),
    (current(collision[0]) != current(collision[1]), "current program: the collision pair gives different digests"),
    (current(forged) != current(book), "current program: the changed konstitucija.txt gives another digest"),
    (current(preimage) != "0" * 64, "current program: the V0.12 preimage no longer hashes to zeros"),
]
for ok, text in checks:
    print(("ok    " if ok else "FAIL  ") + text)
passed = sum(ok for ok, _ in checks)
print(f"\n{'ALL CHECKS PASSED' if passed == len(checks) else 'SOME CHECKS FAILED'}  ({passed}/{len(checks)})")
sys.exit(0 if passed == len(checks) else 1)
