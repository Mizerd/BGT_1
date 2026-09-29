#!/usr/bin/env python3
"""Checks that the shared test inputs contain exactly the bytes their names promise.

Usage: check_fixtures.py <data directory>   (the directory holding exp1/ and konstitucija.txt)
"""

import os
import sys

data = sys.argv[1] if len(sys.argv) > 1 else "."
exp1 = os.path.join(data, "exp1")
files = {f: open(os.path.join(exp1, f), "rb").read() for f in sorted(os.listdir(exp1))}
failures = 0


def check(condition, message):
    global failures
    print(("ok    " if condition else "FAIL  ") + message)
    failures += not condition


alphabet = set(range(0x21, 0x7F))
for k in (1, 2, 3):
    base = files[f"random_{k}.txt"]
    check(len(base) > 1000 and set(base) <= alphabet, f"random_{k}.txt: {len(base)} B, only '!'..'~'")
    for part, pos in (("start", 0), ("middle", len(base) // 2), ("end", len(base) - 1)):
        changed = files[f"random_{k}_{part}.txt"]
        diff = [i for i in range(len(base)) if len(changed) == len(base) and changed[i] != base[i]]
        check(diff == [pos] and set(changed) <= alphabet, f"random_{k}_{part}.txt: exactly byte {pos} differs")

expected = {
    "empty.bin": b"",
    "a.bin": b"a",
    "b.bin": b"b",
    "struct_len15.txt": b"x" * 15,
    "struct_len16.txt": b"x" * 16,
    "struct_len17.txt": b"x" * 17,
    "struct_newline_lf.txt": b"tekstas\n",
    "struct_newline_crlf.txt": b"tekstas\r\n",
    "struct_order_abc.txt": b"abc",
    "struct_order_cba.txt": b"cba",
    "struct_order_words1.txt": b"labas rytas",
    "struct_order_words2.txt": b"rytas labas",
    "struct_pad_ab.txt": b"ab",
    "struct_pad_ab0.txt": b"ab\0",
    "struct_repeat_a.txt": b"a" * 32,
    "struct_repeat_ab.txt": b"ab" * 16,
    "struct_space_none.txt": b"tekstas",
    "struct_space_none_copy.txt": b"tekstas",
    "struct_space_lead.txt": b" tekstas",
    "struct_space_trail.txt": b"tekstas ",
}
for name, content in expected.items():
    check(files.get(name) == content, f"{name} == {content!r}")

for name in ("utf8_lt.txt", "utf8_mixed.txt"):
    text = files[name].decode("utf-8")
    check(len(text) < len(files[name]), f"{name}: valid UTF-8, {len(text)} characters in {len(files[name])} B")

check(len(files) == 34, f"exp1 has 34 files ({len(files)})")
book = open(os.path.join(data, "konstitucija.txt"), "rb").read()
book.decode("utf-8")
check(len(book) == 75595 and book.count(b"\n") == 789 and b"\r" not in book,
      f"konstitucija.txt: {len(book)} B, {book.count(b'\n')} LF lines, no CR")

print(f"\n{'ALL FIXTURES OK' if not failures else 'FIXTURE PROBLEMS'}  ({failures} failures)")
sys.exit(1 if failures else 0)
