// algorithms.hpp - the classic raster graphics algorithms, written from scratch.
//
// None of these call a graphics library. Each one decides WHICH pixels to light
// and reports them through a callback (a lambda):
//     plot(x, y)         - light one pixel
//     hline(x0, x1, y)   - light a horizontal run of pixels (used for fills)
#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <utility>
#include <vector>

#include "types.hpp"

namespace gfx {

// ---------------------------------------------------------------------------
// 1. Bresenham line algorithm
//
// Walk along the line one pixel at a time. Keep an "error" value that measures
// how far the true line is from the pixel centre. When the error passes half a
// pixel, step sideways too. Only integer additions are needed: no division,
// no rounding, no floating point. Works in every direction (all 8 octants).
// ---------------------------------------------------------------------------
template <class Plot>
void bresenham(int x0, int y0, int x1, int y1, Plot plot) {
    const int dx = std::abs(x1 - x0);
    const int dy = -std::abs(y1 - y0);
    const int sx = x0 < x1 ? 1 : -1;
    const int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;  // error term
    for (;;) {
        plot(x0, y0);
        if (x0 == x1 && y0 == y1) break;
        const int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }  // step in x
        if (e2 <= dx) { err += dx; y0 += sy; }  // step in y
    }
}

// ---------------------------------------------------------------------------
// 2. Midpoint circle algorithm
//
// A circle is 8-way symmetric, so we only compute the arc from the top (0, r)
// to the 45 degree point and mirror each pixel into 8 places. At each step we
// test the MIDPOINT between the two candidate pixels with the circle equation
// x^2 + y^2 - r^2 (the decision parameter d):
//     d < 0  -> midpoint inside the circle  -> choose (x+1, y)
//     d >= 0 -> midpoint outside the circle -> choose (x+1, y-1)
// ---------------------------------------------------------------------------
template <class Plot>
void midpointCircle(int cx, int cy, int r, Plot plot) {
    int x = 0, y = r;
    int d = 1 - r;  // initial decision parameter
    while (x <= y) {
        plot(cx + x, cy + y); plot(cx - x, cy + y);
        plot(cx + x, cy - y); plot(cx - x, cy - y);
        plot(cx + y, cy + x); plot(cx - y, cy + x);
        plot(cx + y, cy - x); plot(cx - y, cy - x);
        if (d < 0) {
            d += 2 * x + 3;
        } else {
            d += 2 * (x - y) + 5;
            --y;
        }
        ++x;
    }
}

// Same loop, but fills horizontal spans instead of plotting 8 dots: a solid disc.
template <class HLine>
void fillCircle(int cx, int cy, int r, HLine hline) {
    int x = 0, y = r, d = 1 - r;
    while (x <= y) {
        hline(cx - x, cx + x, cy + y);
        hline(cx - x, cx + x, cy - y);
        hline(cx - y, cx + y, cy + x);
        hline(cx - y, cx + y, cy - x);
        if (d < 0) d += 2 * x + 3;
        else { d += 2 * (x - y) + 5; --y; }
        ++x;
    }
}

// ---------------------------------------------------------------------------
// 3. Scanline polygon fill (even-odd rule)
//
// For every horizontal scanline y:
//   1. find where the scanline crosses each polygon edge,
//   2. sort the crossings from left to right,
//   3. fill between crossing 1-2, 3-4, 5-6 ... (inside / outside alternates).
// We sample at the pixel centre (y + 0.5) so a vertex shared by two edges is
// not counted twice. Horizontal edges are skipped.
// ---------------------------------------------------------------------------
template <class HLine>
void scanlineFill(const std::vector<Vec2>& pts, HLine hline) {
    if (pts.size() < 3) return;
    double minY = pts[0].y, maxY = pts[0].y;
    for (const Vec2& p : pts) {
        minY = std::min(minY, p.y);
        maxY = std::max(maxY, p.y);
    }
    const int yStart = static_cast<int>(std::floor(minY));
    const int yEnd = static_cast<int>(std::ceil(maxY));
    std::vector<double> xs;
    for (int y = yStart; y <= yEnd; ++y) {
        const double yc = y + 0.5;
        xs.clear();
        for (std::size_t i = 0; i < pts.size(); ++i) {
            const Vec2& a = pts[i];
            const Vec2& b = pts[(i + 1) % pts.size()];
            const double lo = std::min(a.y, b.y), hi = std::max(a.y, b.y);
            if (lo == hi) continue;           // horizontal edge: skip
            if (yc < lo || yc >= hi) continue;  // scanline misses this edge
            const double t = (yc - a.y) / (b.y - a.y);
            xs.push_back(a.x + t * (b.x - a.x));  // x where the edge crosses
        }
        std::sort(xs.begin(), xs.end());
        for (std::size_t k = 0; k + 1 < xs.size(); k += 2) {
            // pixel x is inside when its centre (x + 0.5) lies in [xs[k], xs[k+1])
            const int xa = static_cast<int>(std::ceil(xs[k] - 0.5));
            const int xb = static_cast<int>(std::ceil(xs[k + 1] - 0.5)) - 1;
            if (xb >= xa) hline(xa, xb, y);
        }
    }
}

// ---------------------------------------------------------------------------
// 4. Flood fill (4-connected, explicit stack)
//
// Start at a seed pixel, remember its colour, and spread to the up / down /
// left / right neighbours that have the SAME colour. getKey(x, y) returns a
// pixel's colour as a number; plot(x, y) paints a pixel. An explicit stack is
// used instead of recursion so big areas cannot overflow the call stack.
// Returns the number of pixels filled.
// ---------------------------------------------------------------------------
template <class GetKey, class Plot>
int floodFill(int w, int h, GetKey getKey, int sx, int sy, Plot plot) {
    if (sx < 0 || sy < 0 || sx >= w || sy >= h) return 0;
    const auto target = getKey(sx, sy);
    std::vector<std::uint8_t> visited(static_cast<std::size_t>(w) * h, 0);
    std::vector<std::pair<int, int>> stack;
    stack.emplace_back(sx, sy);
    int count = 0;
    while (!stack.empty()) {
        const auto [x, y] = stack.back();
        stack.pop_back();
        if (x < 0 || y < 0 || x >= w || y >= h) continue;
        const std::size_t idx = static_cast<std::size_t>(y) * w + x;
        if (visited[idx]) continue;
        if (getKey(x, y) != target) continue;
        visited[idx] = 1;
        plot(x, y);
        ++count;
        stack.emplace_back(x + 1, y);
        stack.emplace_back(x - 1, y);
        stack.emplace_back(x, y + 1);
        stack.emplace_back(x, y - 1);
    }
    return count;
}

}  // namespace gfx
