#!/bin/sh
# Build and run everything (Git Bash / Linux / macOS). Needs g++ (C++17).
set -e
FLAGS="-std=c++17 -Wall -Wextra -O2 -I src"
g++ $FLAGS tests/tests.cpp -o run_tests
g++ $FLAGS src/phase2_demo.cpp -o phase2_demo
g++ $FLAGS src/phase3_demo.cpp -o phase3_demo
./run_tests
./phase2_demo
./phase3_demo
