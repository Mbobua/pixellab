#!/bin/sh
# Build and run the live OpenGL window. Needs GLFW:
#   MSYS2 UCRT64:  pacman -S --needed mingw-w64-ucrt-x86_64-glfw mingw-w64-ucrt-x86_64-pkgconf
#   Ubuntu/Debian: sudo apt install libglfw3-dev
#   macOS:         brew install glfw
set -e
case "$(uname -s)" in
  MINGW*|MSYS*) GL="-lopengl32 -lgdi32" ;;
  Darwin)       GL="-framework OpenGL" ;;
  *)            GL="-lGL" ;;
esac
if pkg-config --exists glfw3 2>/dev/null; then
  GLFW="$(pkg-config --cflags --libs glfw3)"
else
  GLFW="-lglfw3"   # fallback when pkg-config is not installed
fi
g++ -std=c++17 -O2 -I src src/window_demo.cpp -o window_demo $GLFW $GL
echo "Built window_demo. Starting it..."
./window_demo
