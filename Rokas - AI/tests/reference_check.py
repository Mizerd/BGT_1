#!/usr/bin/env python3
"""Compares hash-generator with an independent Python implementation of the same algorithm.

Usage: reference_check.py [build directory]   (default: build)
"""

import os
import random
import subprocess
import sys
import tempfile

MASK = (1 << 64) - 1
LANE_INIT = [0x50EFEF418AACDDB3, 0x06837D22C9EC9F1B, 0x93ED2CF209AF0CA9, 0x1511760347A610CB,
             0xE0613E1645D94F07, 0x7AA3E75445026145, 0x0976B9398CFCD1FF, 0x2B84EC77CA013781]
MUL_F, MUL_B = 0x2AD26360F20D2A3D, 0x65D3D764F8ED45F5
TAG_STEP, TAG_TAIL, TAG_END = 0xC5E512181F592E6B, 0x9C019A9E6A275255, 0x0C2605DFE6C8B025


def rotl(x, n):
    return ((x << n) | (x >> (64 - n))) & MASK


def step(s, words, tag):
    before = list(s)
    s[0] = (s[0] + (words[0] ^ tag)) & MASK
    s[2] ^= words[1]
    s[4] = (s[4] + words[2]) & MASK
    s[6] ^= words[3]
    for _ in range(2):
        for i in range(1, 8):
            s[i] = ((s[i] ^ rotl(s[i - 1], 41)) * MUL_F) & MASK
        for i in range(6, -1, -1):
            s[i] = ((s[i] + rotl(s[i + 1], 53)) * MUL_B) & MASK
    for i in range(8):
        s[i] ^= before[i]


def words_of(block):
    return [int.from_bytes(block[i:i + 8], "big") for i in range(0, 32, 8)]


def reference(data):
    s = list(LANE_INIT)
    n, r = divmod(len(data), 32)
    for i in range(n):
        step(s, words_of(data[32 * i:32 * i + 32]), (i + 1) * TAG_STEP & MASK)
    step(s, words_of(data[32 * n:] + bytes(32 - r)), ((n + 1) * TAG_STEP + (r + 1) * TAG_TAIL) & MASK)
    length = len(data) & MASK
    step(s, [length, rotl(length, 32), 0, 0], TAG_END)
    for j in range(1, 4):
        step(s, [0, 0, 0, 0], (TAG_END + j * TAG_STEP) & MASK)
    return b"".join((s[i] ^ rotl(s[i + 4], 40)).to_bytes(8, "big") for i in range(4)).hex()


rng = random.Random(20260920)
inputs = [rng.randbytes(n) for n in range(201)]                        # every length up to 200
inputs += [rng.randbytes(rng.randrange(201, 5000)) for _ in range(100)]
inputs += [bytes([v]) * n for v in (0x00, 0xFF) for n in (31, 32, 33, 64, 65)]
inputs += [rng.randbytes(200_003)]                                    # several 64 KiB file chunks

binary = os.path.join(sys.argv[1] if len(sys.argv) > 1 else "build", "hash-generator")
mismatches = 0
with tempfile.TemporaryDirectory() as work:
    path = os.path.join(work, "input")
    for data in inputs:
        with open(path, "wb") as f:
            f.write(data)
        got = subprocess.run([binary, "--file", path], capture_output=True, text=True, check=True).stdout.strip()
        if got != reference(data):
            mismatches += 1
            print(f"FAIL  {len(data)} B: {got} != {reference(data)}")

print(f"ok    {len(inputs) - mismatches}/{len(inputs)} inputs (0–200 003 B) match the Python implementation")
print(f"\n{'ALL CHECKS PASSED' if not mismatches else 'SOME CHECKS FAILED'}  ({len(inputs) - mismatches}/{len(inputs)})")
sys.exit(1 if mismatches else 0)
