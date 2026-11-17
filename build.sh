#!/bin/sh
# Build and run the tests and the picture-making demos (Git Bash / MSYS2 / Linux / macOS).
# Needs g++ (C++17). The live OpenGL window is built separately: sh build_window.sh
set -e
FLAGS="-std=c++17 -Wall -Wextra -O2 -I src"
g++ $FLAGS tests/tests.cpp -o run_tests
g++ $FLAGS tests/viewer_tests.cpp -o run_viewer_tests
g++ $FLAGS src/phase2_demo.cpp -o phase2_demo
g++ $FLAGS src/phase3_demo.cpp -o phase3_demo
./run_tests
./run_viewer_tests
./phase2_demo
./phase3_demo
