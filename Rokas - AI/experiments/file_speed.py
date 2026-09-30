#!/usr/bin/env python3
"""Whole program with I/O: streams 1 GiB into `--file /dev/stdin`, prints time and peak memory.

Usage: file_speed.py <command...>   (e.g. taskset -c 2 build/hash-generator)
"""

import subprocess
import sys
import time

SIZE = 1 << 30
chunk = bytes(1 << 20)
start = time.perf_counter()
child = subprocess.Popen(sys.argv[1:] + ["--file", "/dev/stdin"], stdin=subprocess.PIPE,
                         stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
for _ in range(SIZE // len(chunk)):
    child.stdin.write(chunk)
child.stdin.flush()
with open(f"/proc/{child.pid}/status") as status:  # the program's own peak, read before it exits
    peak_kb = next(int(line.split()[1]) for line in status if line.startswith("VmHWM:"))
child.stdin.close()
digest = child.stdout.read().decode().strip()
child.wait()
seconds = time.perf_counter() - start
if child.returncode != 0 or len(digest) != 64:
    sys.exit("file_speed.py: hash-generator failed")
print(f"file,{SIZE},{seconds:.3f},{peak_kb}")
