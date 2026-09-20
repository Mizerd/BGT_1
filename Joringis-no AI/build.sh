#!/bin/sh
# Kompiliavimas su g++/clang++ (Linux, macOS, MSYS2). Paleisti iš projekto katalogo.
set -e
mkdir -p build
CXX=${CXX:-g++}
FLAGS="-std=c++17 -O2 -Wall -Wextra -Wpedantic -Iinclude"
$CXX $FLAGS src/ratas.cpp src/main.cpp -o build/ratas
$CXX $FLAGS src/ratas.cpp tests/sanity.cpp -o build/sanity
# Standartinių maišų palyginimui ne Windows sistemose reikia OpenSSL (-lcrypto).
$CXX $FLAGS src/ratas.cpp src/std_hashes.cpp src/experiments.cpp -o build/experiments -lcrypto
echo OK
