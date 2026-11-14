// matrix.hpp - 3x3 homogeneous matrices for 2D transformations. Phase 3.
//
// A 2D point (x, y) is written as the column vector [x, y, 1]^T. The extra "1"
// lets translation be a matrix multiplication too, so every transformation
// (translate, rotate, scale) can be combined into ONE matrix.
//
// Stored row-major:   | m0 m1 m2 |     | a  b  tx |
//                     | m3 m4 m5 |  =  | c  d  ty |
//                     | m6 m7 m8 |     | 0  0  1  |
//
// Order matters: A * B means "apply B first, then A".
#pragma once
#include <array>
#include <cmath>

#include "types.hpp"

namespace gfx {

struct Mat3 {
    std::array<double, 9> m{1, 0, 0, 0, 1, 0, 0, 0, 1};  // identity by default

    static Mat3 identity() { return Mat3{}; }

    static Mat3 translate(double tx, double ty) { return Mat3{{1, 0, tx, 0, 1, ty, 0, 0, 1}}; }

    // Angle in radians. With y pointing DOWN on screen, a positive angle looks clockwise.
    static Mat3 rotate(double rad) {
        const double c = std::cos(rad), s = std::sin(rad);
        return Mat3{{c, -s, 0, s, c, 0, 0, 0, 1}};
    }

    static Mat3 scale(double sx, double sy) { return Mat3{{sx, 0, 0, 0, sy, 0, 0, 0, 1}}; }

    // Make any transform act around a pivot instead of the origin:
    //   T(pivot) * M * T(-pivot)  = "move pivot to origin, transform, move back".
    static Mat3 about(const Mat3& M, double px, double py) {
        return translate(px, py) * M * translate(-px, -py);
    }

    // Determinant of the 2x2 linear part. |det| is the area scale factor.
    double det() const { return m[0] * m[4] - m[1] * m[3]; }

    // Matrix product (this * B): B is applied first.
    Mat3 operator*(const Mat3& B) const {
        Mat3 R;
        for (int r = 0; r < 3; ++r)
            for (int c = 0; c < 3; ++c) {
                double sum = 0;
                for (int k = 0; k < 3; ++k) sum += m[r * 3 + k] * B.m[k * 3 + c];
                R.m[r * 3 + c] = sum;
            }
        return R;
    }

    // Apply to a point: [x', y', 1]^T = M * [x, y, 1]^T
    Vec2 apply(Vec2 p) const {
        return Vec2{m[0] * p.x + m[1] * p.y + m[2], m[3] * p.x + m[4] * p.y + m[5]};
    }
};

}  // namespace gfx
