// phase2_demo.cpp - Phase 2 (Core Graphics) demonstration.
//
// Draws one picture per algorithm using ONLY our own code, and saves each as a
// .bmp file in the output/ folder:
//   p2_1_lines.bmp         Bresenham line, all 8 octants
//   p2_2_circles.bmp       midpoint circle (outline and filled)
//   p2_3_polygon_fill.bmp  scanline polygon fill (convex, concave, star)
//   p2_4_flood_fill.bmp    flood fill inside outlined shapes
//   p2_5_combined.bmp      all the above in one scene
#include <filesystem>
#include <iostream>

#include "algorithms.hpp"
#include "framebuffer.hpp"

using namespace gfx;

static const int kSize = 64;      // the "screen" is 64 x 64 pixels
static const int kScale = 10;     // each pixel is saved as a 10 x 10 block
static const Color WHITE{255, 255, 255, 255};
static const Color BLACK{0, 0, 0, 255};
static const Color RED = hexToColor("#d9381e");
static const Color BLUE = hexToColor("#1e6fd9");
static const Color GREEN = hexToColor("#2e9e4f");
static const Color GOLD = hexToColor("#f2a900");
static const Color PURPLE = hexToColor("#7b3fb8");

// Small helpers that connect an algorithm's callbacks to the framebuffer.
static auto plotter(Framebuffer& fb, Color c) { return [&fb, c](int x, int y) { fb.set(x, y, c); }; }
static auto spanner(Framebuffer& fb, Color c) {
    return [&fb, c](int x0, int x1, int y) { for (int x = x0; x <= x1; ++x) fb.set(x, y, c); };
}

static void save(const Framebuffer& fb, const std::string& name) {
    const std::string path = "output/" + name;
    if (fb.saveBMP(path, kScale)) std::cout << "  wrote " << path << "\n";
    else std::cout << "  FAILED to write " << path << "\n";
}

static void drawLines(Framebuffer& fb) {
    // A star of lines: 16 directions cover every octant.
    const Color colours[] = {BLACK, RED, BLUE, GREEN};
    for (int i = 0; i < 16; ++i) {
        const double a = toRadians(i * 22.5);
        const int x = roundi(32 + 28 * std::cos(a)), y = roundi(32 + 28 * std::sin(a));
        bresenham(32, 32, x, y, plotter(fb, colours[i % 4]));
    }
}

static void drawCircles(Framebuffer& fb) {
    midpointCircle(20, 20, 16, plotter(fb, BLUE));
    midpointCircle(20, 20, 10, plotter(fb, RED));
    midpointCircle(20, 20, 4, plotter(fb, BLACK));
    fillCircle(46, 20, 12, spanner(fb, GOLD));
    midpointCircle(46, 20, 12, plotter(fb, BLACK));
    midpointCircle(32, 48, 13, plotter(fb, GREEN));
    fillCircle(32, 48, 6, spanner(fb, PURPLE));
}

static void drawPolygons(Framebuffer& fb) {
    // convex: triangle
    scanlineFill({{4, 28}, {18, 4}, {32, 28}}, spanner(fb, GREEN));
    // concave: an "L" shape (even-odd rule fills it correctly)
    scanlineFill({{38, 4}, {60, 4}, {60, 14}, {48, 14}, {48, 28}, {38, 28}}, spanner(fb, BLUE));
    // self-crossing star: even-odd leaves the centre pentagon empty
    std::vector<Vec2> star;
    for (int i = 0; i < 5; ++i) {
        const double a = toRadians(-90 + i * 144);
        star.push_back({32 + 20 * std::cos(a), 46 + 17 * std::sin(a)});
    }
    scanlineFill(star, spanner(fb, RED));
}

static void drawFloodFill(Framebuffer& fb) {
    // Three outlined shapes drawn with Bresenham / midpoint, then filled with flood fill.
    auto box = [&](int x0, int y0, int x1, int y1) {
        bresenham(x0, y0, x1, y0, plotter(fb, BLACK)); bresenham(x1, y0, x1, y1, plotter(fb, BLACK));
        bresenham(x1, y1, x0, y1, plotter(fb, BLACK)); bresenham(x0, y1, x0, y0, plotter(fb, BLACK));
    };
    box(4, 4, 28, 28);
    midpointCircle(46, 16, 12, plotter(fb, BLACK));
    // a triangle
    bresenham(4, 60, 30, 36, plotter(fb, BLACK));
    bresenham(30, 36, 30, 60, plotter(fb, BLACK));
    bresenham(30, 60, 4, 60, plotter(fb, BLACK));
    // an outlined box with a smaller box inside (fill only the ring between them)
    box(38, 36, 60, 60);
    box(44, 42, 54, 54);

    auto fillAt = [&](int x, int y, Color c) {
        floodFill(fb.width(), fb.height(), [&](int px, int py) { return fb.key(px, py); }, x, y, plotter(fb, c));
    };
    fillAt(16, 16, RED);
    fillAt(46, 16, GOLD);
    fillAt(26, 56, GREEN);
    fillAt(40, 38, BLUE);   // the ring
    fillAt(48, 48, PURPLE); // the inner box
}

static void drawCombined(Framebuffer& fb) {
    scanlineFill({{0, 42}, {63, 42}, {63, 63}, {0, 63}}, spanner(fb, GREEN));                 // grass
    scanlineFill({{14, 28}, {38, 28}, {38, 50}, {14, 50}}, spanner(fb, hexToColor("#8a5a2b"))); // house
    scanlineFill({{11, 28}, {26, 14}, {41, 28}}, spanner(fb, RED));                           // roof
    scanlineFill({{23, 38}, {29, 38}, {29, 50}, {23, 50}}, spanner(fb, hexToColor("#3b2a18"))); // door
    fillCircle(52, 10, 6, spanner(fb, GOLD));                                                  // sun
    for (int i = 0; i < 8; ++i) {                                                              // sun rays
        const double a = toRadians(i * 45.0);
        bresenham(roundi(52 + 9 * std::cos(a)), roundi(10 + 9 * std::sin(a)),
                  roundi(52 + 13 * std::cos(a)), roundi(10 + 13 * std::sin(a)), plotter(fb, GOLD));
    }
    midpointCircle(46, 24, 5, plotter(fb, PURPLE));
    // flood fill the sky (background) last: it spreads until it meets the other shapes
    floodFill(fb.width(), fb.height(), [&](int x, int y) { return fb.key(x, y); }, 2, 2,
              plotter(fb, hexToColor("#87ceeb")));
}

int main() {
    std::filesystem::create_directories("output");
    std::cout << "PixelLab Phase 2: core graphics\n";

    struct Scene { const char* file; void (*draw)(Framebuffer&); };
    const Scene scenes[] = {
        {"p2_1_lines.bmp", drawLines},
        {"p2_2_circles.bmp", drawCircles},
        {"p2_3_polygon_fill.bmp", drawPolygons},
        {"p2_4_flood_fill.bmp", drawFloodFill},
        {"p2_5_combined.bmp", drawCombined},
    };
    for (const Scene& s : scenes) {
        Framebuffer fb(kSize, kSize);
        fb.clear(WHITE);
        s.draw(fb);
        save(fb, s.file);
    }
    std::cout << "Done. Open the .bmp files in the output folder.\n";
    return 0;
}
