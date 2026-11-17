# PixelLab - Raster Graphics in Pure C++

PixelLab is a small computer graphics project written in **pure C++17** with no graphics
library. The "screen" is a 64 x 64 array of pixels owned by the program. Every pixel is chosen
by graphics algorithms written from scratch in this repository, and the finished picture is
saved as a `.bmp` image.

The aim is to show *how* computer graphics works: rasterisation, curves, 2D transformations
with matrices, and animation.

## What the project does

- Draws **lines, circles, polygons and cubic Bezier curves** pixel by pixel.
- **Fills** shapes (scanline fill) and areas (flood fill).
- **Moves, rotates, scales and flips** shapes using 3x3 matrices, and combines transformations.
- Produces an **animation** as a sequence of frames.
- Saves every picture as a `.bmp` file (opens in any image viewer).
- Optional **live OpenGL window**: move, rotate, scale and animate a shape in real time, draw with
  the mouse and flood-fill areas. OpenGL only *shows* the finished pixels; all drawing is still done
  by the algorithms in this repository.

## How to build and run

You need a C++17 compiler (g++ 7 or newer, clang, or MSVC). Nothing else.

**Git Bash / Linux / macOS**

```bash
sh build.sh
```

**Windows (MinGW g++)**

```bat
build.bat
```

**By hand**

```bash
g++ -std=c++17 -O2 -I src src/phase2_demo.cpp -o phase2_demo
g++ -std=c++17 -O2 -I src src/phase3_demo.cpp -o phase3_demo
g++ -std=c++17 -O2 -I src tests/tests.cpp   -o run_tests
./phase2_demo     # writes output/p2_*.bmp
./phase3_demo     # writes output/p3_*.bmp and output/anim/frame_*.bmp
./run_tests       # 20 checks of the algorithms against known maths results
./run_viewer_tests  # 17 checks of the interactive logic (no window needed)
```

**Live OpenGL window**

Needs the GLFW library (install once):

```bash
# MSYS2 UCRT64 window (Windows):
pacman -S --needed mingw-w64-ucrt-x86_64-glfw mingw-w64-ucrt-x86_64-pkgconf
# Ubuntu / Debian:  sudo apt install libglfw3-dev
# macOS:            brew install glfw
```

Then:

```bash
sh build_window.sh
```

| Control | Action |
|---|---|
| Arrow keys | move the house |
| Q / E | rotate left / right (15 degrees) |
| + / - | scale up / down |
| F | flip horizontally |
| Space | start / stop the animation (sun orbit, pulsing ring) |
| 1 / 2 | pencil / bucket (flood fill) tool |
| N | next colour |
| Left mouse button | draw with the pencil, or flood-fill with the bucket |
| C / R | clear paint / reset everything |
| S | save a screenshot (`screenshot_1.bmp`, ...) |
| Esc | quit |

Painting is a raster layer drawn on top of the scene, so it stays where you painted it while shapes
move. Press C to clear it.

**Visual Studio (Developer Command Prompt)**

```bat
cl /std:c++17 /EHsc /I src src\phase2_demo.cpp
```

Run the programs from the project folder so the `output/` folder is created there.
Sample results are already in `docs/screenshots/`.

## Graphics techniques implemented

| Technique | File | Used for | Phase |
|---|---|---|---|
| Framebuffer, clipping, BMP output | `src/framebuffer.hpp` | The pixel array and saving pictures | 2 |
| Bresenham line algorithm | `src/algorithms.hpp` | Lines, polygon outlines, curve segments | 2 |
| Midpoint circle algorithm | `src/algorithms.hpp` | Circle outline and filled disc | 2 |
| Scanline polygon fill (even-odd) | `src/algorithms.hpp` | Filled polygons, rectangles | 2 |
| Flood fill (4-connected, stack) | `src/algorithms.hpp` | Bucket fill of closed areas | 2 |
| Colour (RGBA, hex conversion) | `src/types.hpp` | All colours | 2 |
| Cubic Bezier (de Casteljau) | `src/curves.hpp` | Smooth curves | 3 |
| 2D transformations (3x3 homogeneous matrices) | `src/matrix.hpp` | Translate, rotate, scale, flip | 3 |
| Rotation about a pivot, `T(c) R T(-c)` | `src/matrix.hpp` | Spinning shapes in place | 3 |
| Shape model (geometry, then raster) | `src/shapes.hpp` | Transforming then re-drawing shapes | 3 |
| Animation (per-frame matrices) | `src/phase3_demo.cpp`, `src/viewer.hpp` | Spin, orbit, pulse frames; live animation | 3 |
| Interactive viewer (commands, mouse, paint layer) | `src/viewer.hpp` | Scene logic with no window code | 4 |
| OpenGL display (pixels uploaded as a texture) | `src/window_demo.cpp` | Live window | 4 |

## Project structure

```
src/types.hpp         Vec2, Color, hex colours
src/framebuffer.hpp   pixel array + BMP writer
src/algorithms.hpp    Bresenham, midpoint circle, scanline fill, flood fill
src/curves.hpp        cubic Bezier (de Casteljau)
src/matrix.hpp        3x3 matrices
src/shapes.hpp        Line / Circle / Polygon / Bezier shapes, transform, draw
src/phase2_demo.cpp   core graphics pictures
src/phase3_demo.cpp   transformations, curves, animation pictures
src/viewer.hpp        interactive scene: commands, mouse, paint layer (no window code)
src/window_demo.cpp   live OpenGL window (GLFW) that displays the viewer's pixels
tests/tests.cpp       20 checks of the algorithms
tests/viewer_tests.cpp  17 checks of the interactive logic
docs/                 showcase script, screenshots
```

## Data flow

```
shape points --(3x3 matrix)--> new points --(algorithms)--> pixels in framebuffer --> .bmp file
                                                                              \--> OpenGL texture --> window
```

## Contributors

| Name | GitHub | Contribution |
|---|---|---|
| William Githinji | Mbobua | Solo project: design, algorithms, transformations, testing, documentation |
