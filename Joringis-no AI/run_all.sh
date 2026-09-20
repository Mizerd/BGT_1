#!/bin/sh
# Pilnas atkartojimas Linux/macOS/MSYS2 (g++): kompiliavimas, patikros, eksperimentai, grafikai.
set -e
sh build.sh
build/sanity
mkdir -p data/exp1
build/experiments all
sh tests/cli_checks.sh build
python3 tools/plot.py
echo "Viskas baigta. Rezultatai: results/"
