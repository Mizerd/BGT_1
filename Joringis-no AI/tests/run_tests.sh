#!/bin/sh
set -e
cd "$(dirname "$0")/.."
mkdir -p build
CXX="${CXX:-g++}"
$CXX -std=c++20 -O2 -Wall -Wextra -o build/ratas ratas.cpp
$CXX -std=c++20 -O2 -Wall -Wextra -o build/stream_test tests/stream_test.cpp
$CXX -std=c++20 -O2 -Wall -Wextra -o build/sac tests/sac.cpp
python3 tests/check.py
./build/sac
