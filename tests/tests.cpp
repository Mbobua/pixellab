// tests.cpp - checks each algorithm against a known mathematical result.
// Build and run:  g++ -std=c++17 -I src tests/tests.cpp -o tests && ./tests
#include <cmath>
#include <iostream>
#include <set>
#include <utility>
#include <vector>

#include "algorithms.hpp"
#include "curves.hpp"
#include "framebuffer.hpp"
#include "matrix.hpp"
#include "shapes.hpp"

using namespace gfx;

static int passed = 0, failed = 0;

#define CHECK(name, cond)                                                   \
    do {                                                                    \
        if (cond) { ++passed; std::cout << "  ok    " << name << "\n"; }     \
        else { ++failed; std::cout << "  FAIL  " << name << "\n"; }         \
    } while (0)

static bool near(double a, double b, double eps = 1e-9) { return std::fabs(a - b) < eps; }
using Pts = std::vector<std::pair<int, int>>;

int main() {
    // ------------------------------------------------------------ Bresenham
    std::cout << "Bresenham line\n";
    {
        Pts p;
        bresenham(2, 3, 17, 9, [&](int x, int y) { p.emplace_back(x, y); });
        CHECK("includes both endpoints", p.front() == std::make_pair(2, 3) && p.back() == std::make_pair(17, 9));
    }
    {
        const int cases[][4] = {{0, 0, 10, 3}, {10, 3, 0, 0}, {0, 0, 3, 10}, {5, 5, 5, 5}, {0, 10, 10, 0}, {10, 0, 0, 10}};
        bool ok = true;
        for (auto& c : cases) {
            int n = 0;
            bresenham(c[0], c[1], c[2], c[3], [&](int, int) { ++n; });
            ok = ok && n == std::max(std::abs(c[2] - c[0]), std::abs(c[3] - c[1])) + 1;
        }
        CHECK("pixel count = max(|dx|,|dy|) + 1 in every direction", ok);
    }
    {
        bool ok = true;
        bresenham(0, 0, 30, 11, [&](int x, int y) { ok = ok && std::fabs(y - (11.0 / 30.0) * x) <= 0.5 + 1e-9; });
        CHECK("every pixel within 0.5 px of the true line", ok);
    }

    // ------------------------------------------------------------ circle
    std::cout << "Midpoint circle\n";
    {
        bool ok = true;
        for (int r : {1, 5, 12, 25})
            midpointCircle(0, 0, r, [&](int x, int y) { ok = ok && std::fabs(std::hypot(x, y) - r) < 0.75; });
        CHECK("every pixel within 0.75 px of the true radius", ok);
    }
    {
        std::set<std::pair<int, int>> s;
        midpointCircle(0, 0, 9, [&](int x, int y) { s.insert({x, y}); });
        bool ok = true;
        for (auto [x, y] : s) ok = ok && s.count({y, x}) && s.count({-x, y}) && s.count({x, -y});
        CHECK("circle is 8-way symmetric", ok);
    }
    {
        Framebuffer fb(64, 64);
        fillCircle(32, 32, 10, [&](int x0, int x1, int y) { for (int x = x0; x <= x1; ++x) fb.set(x, y, {1, 1, 1, 255}); });
        int n = 0;
        for (int y = 0; y < 64; ++y) for (int x = 0; x < 64; ++x) if (fb.get(x, y).a) ++n;
        const double expected = kPi * 10.5 * 10.5;
        CHECK("filled circle area close to pi (r+0.5)^2", std::fabs(n - expected) / expected < 0.03);
    }

    // ------------------------------------------------------------ scanline
    std::cout << "Scanline polygon fill\n";
    auto area = [](const std::vector<Vec2>& poly) {
        int n = 0;
        scanlineFill(poly, [&](int x0, int x1, int) { n += x1 - x0 + 1; });
        return n;
    };
    CHECK("4x4 square fills 16 pixels", area({{0, 0}, {4, 0}, {4, 4}, {0, 4}}) == 16);
    CHECK("right triangle area about half the square", std::abs(area({{0, 0}, {20, 0}, {0, 20}}) - 200) <= 20);
    CHECK("concave L shape uses even-odd correctly (area 12)",
          area({{0, 0}, {4, 0}, {4, 2}, {2, 2}, {2, 4}, {0, 4}}) == 12);

    // ------------------------------------------------------------ flood fill
    std::cout << "Flood fill\n";
    {
        const int w = 10, h = 10;
        std::vector<std::vector<int>> g(h, std::vector<int>(w, 0));
        for (int i = 2; i <= 7; ++i) { g[2][i] = 1; g[7][i] = 1; g[i][2] = 1; g[i][7] = 1; }  // a box
        const int n = floodFill(w, h, [&](int x, int y) { return g[y][x]; }, 4, 4, [&](int x, int y) { g[y][x] = 2; });
        CHECK("fills exactly the 4x4 inside of the box", n == 16);
        CHECK("outside of the box is untouched", g[0][0] == 0);
    }

    // ------------------------------------------------------------ Bezier
    std::cout << "Bezier curve\n";
    {
        const std::array<Vec2, 4> P{{{0, 0}, {10, 40}, {30, -20}, {50, 10}}};
        const Vec2 a = bezierPoint(P, 0), b = bezierPoint(P, 1);
        CHECK("curve starts at P0 and ends at P3", near(a.x, 0) && near(a.y, 0) && near(b.x, 50) && near(b.y, 10));
        const Vec2 q = bezierPoint(P, 0.5);
        const double bw[4] = {0.125, 0.375, 0.375, 0.125};
        double ex = 0, ey = 0;
        for (int i = 0; i < 4; ++i) { ex += bw[i] * P[i].x; ey += bw[i] * P[i].y; }
        CHECK("t = 0.5 matches the Bernstein formula", near(q.x, ex) && near(q.y, ey));
    }

    // ------------------------------------------------------------ matrices
    std::cout << "Matrices and shapes\n";
    {
        const Vec2 p = Mat3::rotate(kPi / 2).apply({1, 0});
        CHECK("rotate 90 deg maps (1,0) to (0,1)", near(p.x, 0) && near(p.y, 1));
        const Vec2 q = Mat3::about(Mat3::rotate(1.234), 10, 20).apply({10, 20});
        CHECK("rotation about a pivot keeps the pivot fixed", near(q.x, 10) && near(q.y, 20));
        const Vec2 s1 = (Mat3::translate(5, 0) * Mat3::scale(2, 2)).apply({1, 1});  // scale first
        const Vec2 s2 = (Mat3::scale(2, 2) * Mat3::translate(5, 0)).apply({1, 1});  // translate first
        CHECK("matrix order matters: (7,2) vs (12,2)", near(s1.x, 7) && near(s1.y, 2) && near(s2.x, 12) && near(s2.y, 2));
        CHECK("det(rotation) = 1 and det(scale(2,3)) = 6", near(Mat3::rotate(0.7).det(), 1) && near(Mat3::scale(2, 3).det(), 6));
        Shape c = makeCircle({10, 10}, 5, {0, 0, 0, 255}, false);
        transform(c, Mat3::about(Mat3::scale(2, 2), 10, 10));
        CHECK("scaling a circle by 2 doubles its radius", near(c.r, 10) && near(c.pts[0].x, 10) && near(c.pts[0].y, 10));
    }

    // ------------------------------------------------------------ framebuffer
    std::cout << "Framebuffer\n";
    {
        Framebuffer fb(4, 4);
        fb.clear({255, 255, 255, 255});
        fb.set(-1, 0, {1, 2, 3, 255});   // off-screen: must be ignored
        fb.set(4, 4, {1, 2, 3, 255});
        fb.set(2, 1, {9, 8, 7, 255});
        CHECK("set/get round trip", fb.get(2, 1) == Color({9, 8, 7, 255}));
        CHECK("off-screen writes are clipped", fb.get(0, 0) == Color({255, 255, 255, 255}));
    }

    std::cout << "\n" << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? 0 : 1;
}
