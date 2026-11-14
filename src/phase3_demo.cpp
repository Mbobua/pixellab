// phase3_demo.cpp - Phase 3 (Advanced Graphics) demonstration.
//
// Uses the Phase 2 algorithms plus Bezier curves and 2D matrix transformations.
// Saves pictures to the output/ folder:
//   p3_1_bezier.bmp          cubic Bezier curves with their control points
//   p3_2_transforms.bmp      the same house: original, translated, rotated, scaled
//   p3_3_pivot.bmp           rotating about the origin vs about the shape's centre
//   p3_4_order.bmp           matrix order matters: T*R vs R*T
//   p3_5_animation.bmp       8 frames of an animation in one filmstrip
//   p3_6_scene.bmp           final scene combining everything
//   anim/frame_000..035.bmp  36 animation frames (view as a slideshow)
// It also prints the matrices it uses, so you can show them in the video.
#include <array>
#include <cstdio>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <string>

#include "framebuffer.hpp"
#include "shapes.hpp"

using namespace gfx;

static const int kSize = 64;
static const int kScale = 10;
static const Color WHITE{255, 255, 255, 255};
static const Color BLACK{0, 0, 0, 255};
static const Color GREY{170, 170, 170, 255};
static const Color RED = hexToColor("#d9381e");
static const Color BLUE = hexToColor("#1e6fd9");
static const Color GREEN = hexToColor("#2e9e4f");
static const Color GOLD = hexToColor("#f2a900");
static const Color PURPLE = hexToColor("#7b3fb8");
static const Color BROWN = hexToColor("#8a5a2b");

static void save(const Framebuffer& fb, const std::string& path, int scale = kScale) {
    if (fb.saveBMP(path, scale)) std::cout << "  wrote " << path << "\n";
    else std::cout << "  FAILED to write " << path << "\n";
}

static void printMatrix(const std::string& name, const Mat3& M) {
    std::cout << "  " << name << "\n" << std::fixed << std::setprecision(2);
    for (int r = 0; r < 3; ++r)
        std::cout << "    | " << std::setw(7) << M.m[r * 3] << " " << std::setw(7) << M.m[r * 3 + 1] << " "
                  << std::setw(7) << M.m[r * 3 + 2] << " |\n";
}

// A small house made of three polygon shapes and a circle: a shape GROUP we can transform together.
static std::vector<Shape> makeHouse(Color wall, Color roof) {
    std::vector<Shape> h;
    h.push_back(makeRect(0, 8, 14, 22, wall, true));                       // wall
    h.push_back(makePolygon({{-2, 8}, {7, 0}, {16, 8}}, roof, true));      // roof
    h.push_back(makeRect(5, 14, 9, 22, hexToColor("#3b2a18"), true));      // door
    return h;
}

static void transformAll(std::vector<Shape>& shapes, const Mat3& M) {
    for (Shape& s : shapes) transform(s, M);
}
static void drawAll(Framebuffer& fb, const std::vector<Shape>& shapes) {
    for (const Shape& s : shapes) draw(fb, s);
}
static Vec2 groupCentre(const std::vector<Shape>& shapes) {
    Vec2 sum{0, 0};
    int n = 0;
    for (const Shape& s : shapes)
        for (const Vec2& p : s.pts) { sum.x += p.x; sum.y += p.y; ++n; }
    return {sum.x / n, sum.y / n};
}

// ---------------------------------------------------------------- scenes
static void sceneBezier(Framebuffer& fb) {
    struct Curve { std::array<Vec2, 4> p; Color c; };
    const Curve curves[] = {
        {{{{4, 56}, {20, 4}, {44, 4}, {60, 56}}}, RED},
        {{{{4, 10}, {30, 60}, {34, 0}, {60, 30}}}, BLUE},
    };
    for (const Curve& cv : curves) {
        // control polygon (grey) and control points (small squares) so the idea is visible
        for (int i = 0; i < 3; ++i)
            bresenham(roundi(cv.p[i].x), roundi(cv.p[i].y), roundi(cv.p[i + 1].x), roundi(cv.p[i + 1].y),
                      [&](int x, int y) { fb.set(x, y, GREY); });
        draw(fb, makeBezier(cv.p[0], cv.p[1], cv.p[2], cv.p[3], cv.c));
        for (const Vec2& q : cv.p)
            for (int dy = -1; dy <= 1; ++dy)
                for (int dx = -1; dx <= 1; ++dx) fb.set(roundi(q.x) + dx, roundi(q.y) + dy, BLACK);
    }
}

static void sceneTransforms(Framebuffer& fb) {
    // Four copies of the same house, each moved by a different matrix.
    // This picture uses a 128 x 128 screen: rotation looks much cleaner with more pixels,
    // so every matrix below ends with scale(k, k), where k = fb.width() / 64.
    const double k = fb.width() / 64.0;
    const Mat3 S = Mat3::scale(k, k);
    std::vector<Shape> original = makeHouse(BROWN, RED);
    {   // 1: translate
        auto h = original;
        transformAll(h, S * Mat3::translate(4, 4));
        drawAll(fb, h);
    }
    {   // 2: rotate 20 degrees about its own centre
        auto h = original;
        const Vec2 c = groupCentre(h);
        transformAll(h, S * Mat3::translate(36, 4) * Mat3::about(Mat3::rotate(toRadians(20)), c.x, c.y));
        drawAll(fb, h);
    }
    {   // 3: scale 1.4x about its own centre
        auto h = original;
        const Vec2 c = groupCentre(h);
        transformAll(h, S * Mat3::translate(8, 34) * Mat3::about(Mat3::scale(1.4, 1.4), c.x, c.y));
        drawAll(fb, h);
    }
    {   // 4: flip horizontally (scale x by -1) about its centre
        auto h = makeHouse(BLUE, PURPLE);
        const Vec2 c = groupCentre(h);
        transformAll(h, S * Mat3::translate(36, 36) * Mat3::about(Mat3::scale(-1, 1), c.x, c.y));
        drawAll(fb, h);
    }
}

static void scenePivot(Framebuffer& fb) {
    // Left half: rotate about the ORIGIN of the screen (0,0) -> the shape swings far away.
    // Right half: rotate about the shape's own centre -> the shape spins in place.
    const Shape square = makeRect(20, 6, 34, 20, GREY, false);
    for (int k = 0; k <= 3; ++k) {
        Shape s = square;
        transform(s, Mat3::rotate(toRadians(k * 12.0)));
        s.color = (k == 3) ? RED : GREY;
        draw(fb, s);
    }
    const Vec2 pivot = centroid(Shape{ShapeType::Polygon, {{44, 26}, {58, 26}, {58, 40}, {44, 40}}, 0, BLACK, false});
    for (int k = 0; k <= 3; ++k) {
        Shape s = makeRect(44, 26, 58, 40, (k == 3) ? GREEN : GREY, false);
        transform(s, Mat3::about(Mat3::rotate(toRadians(k * 15.0)), pivot.x, pivot.y));
        draw(fb, s);
    }
    // mark the origin (0,0) and the pivot
    for (int i = 0; i < 3; ++i) { fb.set(i, 0, BLACK); fb.set(0, i, BLACK); }
    fb.set(roundi(pivot.x), roundi(pivot.y), BLACK);
    // dividing line
    bresenham(32, 0, 32, 63, [&](int x, int y) { if (y % 2 == 0) fb.set(x, y, BLACK); });
}

static void sceneOrder(Framebuffer& fb) {
    // Same two operations, different order, different result.
    const Mat3 T = Mat3::translate(26, 0);
    const Mat3 R = Mat3::rotate(toRadians(30));
    Shape a = makeRect(4, 8, 14, 18, BLUE, true);     // rotate FIRST, then translate:  T * R
    Shape b = makeRect(4, 8, 14, 18, RED, true);      // translate FIRST, then rotate:  R * T
    Shape ghost = makeRect(4, 8, 14, 18, GREY, false);
    draw(fb, ghost);
    transform(a, T * R);
    transform(b, R * T);
    draw(fb, a);
    draw(fb, b);
    std::cout << "\nOrder of matrices (T = translate 26,0   R = rotate 30 deg):\n";
    printMatrix("T * R  (rotate first, then translate)  [blue]", T * R);
    printMatrix("R * T  (translate first, then rotate)  [red]", R * T);
}

static void drawAnimatedFrame(Framebuffer& fb, int frame, int totalFrames) {
    const double angle = 360.0 * frame / totalFrames;
    // spinning square about its own centre
    Shape sq = makeRect(10, 10, 26, 26, RED, true);
    transform(sq, Mat3::about(Mat3::rotate(toRadians(angle * 2)), 18, 18));
    draw(fb, sq);
    // a circle orbiting the screen centre
    Shape c = makeCircle({32, 10}, 5, GOLD, true);
    transform(c, Mat3::about(Mat3::rotate(toRadians(angle)), 32, 32));
    draw(fb, c);
    // a growing and shrinking circle (scale about its centre)
    const double k = 1.0 + 0.5 * std::sin(toRadians(angle * 2));
    Shape g = makeCircle({46, 46}, 7, GREEN, false);
    transform(g, Mat3::about(Mat3::scale(k, k), 46, 46));
    draw(fb, g);
    // a line like a clock hand
    Shape hand = makeLine({18, 46}, {18, 36}, PURPLE);
    transform(hand, Mat3::about(Mat3::rotate(toRadians(angle)), 18, 46));
    draw(fb, hand);
    midpointCircle(18, 46, 11, [&](int x, int y) { fb.set(x, y, GREY); });
}

static void sceneAnimationStrip() {
    const int frames = 8;
    Framebuffer strip(kSize * 4, kSize * 2);
    strip.clear(WHITE);
    for (int i = 0; i < frames; ++i) {
        Framebuffer g(kSize, kSize);
        g.clear(WHITE);
        drawAnimatedFrame(g, i * 4, 36);  // frames 0, 4, 8 ... 28 of the 36-frame loop
        strip.blit(g, (i % 4) * kSize, (i / 4) * kSize);
        for (int k = 0; k < kSize; ++k) {  // thin frame borders
            strip.set((i % 4) * kSize + k, (i / 4) * kSize, GREY);
            strip.set((i % 4) * kSize, (i / 4) * kSize + k, GREY);
        }
    }
    save(strip, "output/p3_5_animation.bmp", 5);
}

static void sceneFinal(Framebuffer& fb) {
    std::vector<Shape> scene;
    scene.push_back(makeRect(0, 0, 63, 42, hexToColor("#87ceeb"), true));          // sky
    scene.push_back(makeRect(0, 42, 63, 63, GREEN, true));                         // grass
    auto house = makeHouse(BROWN, RED);
    transformAll(house, Mat3::translate(10, 26) * Mat3::scale(1.2, 1.2));          // place and enlarge the house
    for (const Shape& s : house) scene.push_back(s);
    scene.push_back(makeCircle({52, 10}, 6, GOLD, true));                          // sun
    scene.push_back(makeBezier({0, 58}, {20, 46}, {40, 62}, {63, 52}, BLUE));      // a path / river
    scene.push_back(makeLine({48, 26}, {48, 50}, BLACK));                          // a pole
    scene.push_back(makeCircle({48, 22}, 4, PURPLE, false));                       // a ring on the pole
    for (const Shape& s : scene) draw(fb, s);
}

int main() {
    std::filesystem::create_directories("output/anim");
    std::cout << "PixelLab Phase 3: transformations, curves, animation\n";

    struct Scene { const char* file; void (*draw)(Framebuffer&); };
    const Scene scenes[] = {
        {"p3_1_bezier.bmp", sceneBezier},
        {"p3_2_transforms.bmp", sceneTransforms},
        {"p3_3_pivot.bmp", scenePivot},
        {"p3_4_order.bmp", sceneOrder},
        {"p3_6_scene.bmp", sceneFinal},
    };
    for (const Scene& s : scenes) {
        const int size = (s.draw == sceneTransforms) ? 128 : kSize;  // transforms picture: 128 x 128
        Framebuffer fb(size, size);
        fb.clear(WHITE);
        s.draw(fb);
        save(fb, std::string("output/") + s.file, 640 / size);
    }

    sceneAnimationStrip();

    for (int i = 0; i < 36; ++i) {
        Framebuffer fb(kSize, kSize);
        fb.clear(WHITE);
        drawAnimatedFrame(fb, i, 36);
        char name[64];
        std::snprintf(name, sizeof(name), "output/anim/frame_%03d.bmp", i);
        fb.saveBMP(name, kScale);
    }
    std::cout << "  wrote output/anim/frame_000.bmp ... frame_035.bmp\n";

    std::cout << "\nBasic matrices used:\n";
    printMatrix("translate(4, 4)", Mat3::translate(4, 4));
    printMatrix("rotate(30 degrees)", Mat3::rotate(toRadians(30)));
    printMatrix("scale(1.4, 1.4)", Mat3::scale(1.4, 1.4));
    printMatrix("rotate 90 about pivot (10, 10) = T(10,10) * R(90) * T(-10,-10)",
                Mat3::about(Mat3::rotate(toRadians(90)), 10, 10));

    std::cout << "\nDone. Open the .bmp files in the output folder.\n";
    return 0;
}
