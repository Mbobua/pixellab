// curves.hpp - cubic Bezier curves (de Casteljau). Phase 3 (advanced graphics).
//
// Given 4 control points P0..P3 and t in [0, 1], repeatedly interpolate between
// neighbouring points:
//     level 1:  Q0 = lerp(P0,P1,t)  Q1 = lerp(P1,P2,t)  Q2 = lerp(P2,P3,t)
//     level 2:  R0 = lerp(Q0,Q1,t)  R1 = lerp(Q1,Q2,t)
//     level 3:  B  = lerp(R0,R1,t)   <- the point on the curve
// The curve starts at P0, ends at P3, and is only "pulled toward" P1 and P2.
// Sampling many t values and joining them with Bresenham lines draws the curve.
#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

#include "algorithms.hpp"
#include "types.hpp"

namespace gfx {

inline Vec2 lerp(Vec2 a, Vec2 b, double t) {
    return Vec2{a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t};
}

inline Vec2 bezierPoint(const std::array<Vec2, 4>& p, double t) {
    const Vec2 q0 = lerp(p[0], p[1], t), q1 = lerp(p[1], p[2], t), q2 = lerp(p[2], p[3], t);
    const Vec2 r0 = lerp(q0, q1, t), r1 = lerp(q1, q2, t);
    return lerp(r0, r1, t);
}

// Sample the curve at enough points to look smooth. The length of the control
// polygon is an upper bound of the curve length, so it sets the sample count.
inline std::vector<Vec2> bezierPolyline(const std::array<Vec2, 4>& p) {
    double len = 0;
    for (int i = 0; i < 3; ++i) len += std::hypot(p[i + 1].x - p[i].x, p[i + 1].y - p[i].y);
    const int steps = std::max(8, std::min(200, static_cast<int>(std::ceil(len))));
    std::vector<Vec2> out;
    out.reserve(steps + 1);
    for (int i = 0; i <= steps; ++i) out.push_back(bezierPoint(p, static_cast<double>(i) / steps));
    return out;
}

template <class Plot>
void drawBezier(const std::array<Vec2, 4>& p, Plot plot) {
    const std::vector<Vec2> pts = bezierPolyline(p);
    for (std::size_t i = 0; i + 1 < pts.size(); ++i)
        bresenham(roundi(pts[i].x), roundi(pts[i].y), roundi(pts[i + 1].x), roundi(pts[i + 1].y), plot);
}

}  // namespace gfx
