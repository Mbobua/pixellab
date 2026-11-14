// types.hpp - small value types shared by every module.
#pragma once
#include <cmath>
#include <cstdint>
#include <string>

namespace gfx {

// A 2D point or vector with floating-point coordinates.
struct Vec2 {
    double x = 0.0;
    double y = 0.0;
};

// An RGBA colour, 8 bits per channel (0-255).
struct Color {
    std::uint8_t r = 0, g = 0, b = 0, a = 255;
};

inline bool operator==(const Color& p, const Color& q) {
    return p.r == q.r && p.g == q.g && p.b == q.b && p.a == q.a;
}

// Convert "#rrggbb" (or "rrggbb") to a Color.
inline Color hexToColor(const std::string& hex) {
    std::size_t i = (!hex.empty() && hex[0] == '#') ? 1 : 0;
    auto byteAt = [&](std::size_t pos) {
        return static_cast<std::uint8_t>(std::stoi(hex.substr(pos, 2), nullptr, 16));
    };
    return Color{byteAt(i), byteAt(i + 2), byteAt(i + 4), 255};
}

constexpr double kPi = 3.14159265358979323846;
inline double toRadians(double deg) { return deg * kPi / 180.0; }
inline double toDegrees(double rad) { return rad * 180.0 / kPi; }

// Round to the nearest integer; halves go up.
inline int roundi(double v) { return static_cast<int>(std::floor(v + 0.5)); }

}  // namespace gfx
