// shapes.hpp - the vector scene. Phase 3.
//
// Shapes are stored as GEOMETRY (points, radius), not pixels. That is what lets
// us move, rotate and scale them: multiply their points by a matrix, then
// re-rasterize them with our own algorithms.
//
//    points  --matrix-->  new points  --algorithms-->  pixels
//
//   Line     pts = {p0, p1}
//   Circle   pts = {centre}, r = radius
//   Polygon  pts = {p0, p1, p2, ...}   (a rectangle is a polygon with 4 points)
//   Bezier   pts = {P0, P1, P2, P3}    (cubic control points)
#pragma once
#include <array>
#include <cmath>
#include <vector>

#include "algorithms.hpp"
#include "curves.hpp"
#include "framebuffer.hpp"
#include "matrix.hpp"
#include "types.hpp"

namespace gfx {

enum class ShapeType { Line, Circle, Polygon, Bezier };

struct Shape {
    ShapeType type = ShapeType::Line;
    std::vector<Vec2> pts;
    double r = 0.0;  // radius (circles only)
    Color color{0, 0, 0, 255};
    bool fill = false;
};

inline Shape makeLine(Vec2 a, Vec2 b, Color c) { return Shape{ShapeType::Line, {a, b}, 0, c, false}; }
inline Shape makeCircle(Vec2 centre, double r, Color c, bool fill) { return Shape{ShapeType::Circle, {centre}, r, c, fill}; }
inline Shape makePolygon(std::vector<Vec2> pts, Color c, bool fill) { return Shape{ShapeType::Polygon, std::move(pts), 0, c, fill}; }
inline Shape makeRect(double x0, double y0, double x1, double y1, Color c, bool fill) {
    return makePolygon({{x0, y0}, {x1, y0}, {x1, y1}, {x0, y1}}, c, fill);
}
inline Shape makeBezier(Vec2 p0, Vec2 p1, Vec2 p2, Vec2 p3, Color c) { return Shape{ShapeType::Bezier, {p0, p1, p2, p3}, 0, c, false}; }

// Average of the points: used as the pivot for rotate / scale.
inline Vec2 centroid(const Shape& s) {
    Vec2 sum{0, 0};
    for (const Vec2& p : s.pts) { sum.x += p.x; sum.y += p.y; }
    return Vec2{sum.x / s.pts.size(), sum.y / s.pts.size()};
}

// Apply a 3x3 matrix to every point of the shape (in place).
inline void transform(Shape& s, const Mat3& M) {
    for (Vec2& p : s.pts) p = M.apply(p);
    if (s.type == ShapeType::Circle) {
        // A circle stays a circle only under uniform scaling, so scale the radius
        // by sqrt(|determinant|), the average linear scale factor.
        s.r *= std::sqrt(std::fabs(M.det()));
    }
}

// Rasterize one shape into a framebuffer using our own algorithms.
inline void draw(Framebuffer& fb, const Shape& s) {
    auto plot = [&](int x, int y) { fb.set(x, y, s.color); };
    auto hline = [&](int x0, int x1, int y) { for (int x = x0; x <= x1; ++x) fb.set(x, y, s.color); };

    switch (s.type) {
        case ShapeType::Line:
            bresenham(roundi(s.pts[0].x), roundi(s.pts[0].y), roundi(s.pts[1].x), roundi(s.pts[1].y), plot);
            break;
        case ShapeType::Circle: {
            const int cx = roundi(s.pts[0].x), cy = roundi(s.pts[0].y);
            const int r = std::max(0, roundi(s.r));
            if (s.fill) fillCircle(cx, cy, r, hline);
            else midpointCircle(cx, cy, r, plot);
            break;
        }
        case ShapeType::Polygon:
            if (s.fill) scanlineFill(s.pts, hline);
            for (std::size_t i = 0; i < s.pts.size(); ++i) {
                const Vec2& a = s.pts[i];
                const Vec2& b = s.pts[(i + 1) % s.pts.size()];
                bresenham(roundi(a.x), roundi(a.y), roundi(b.x), roundi(b.y), plot);
            }
            break;
        case ShapeType::Bezier:
            drawBezier({s.pts[0], s.pts[1], s.pts[2], s.pts[3]}, plot);
            break;
    }
}

}  // namespace gfx
