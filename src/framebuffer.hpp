// framebuffer.hpp - our own "screen" in memory.
//
// Instead of using a graphics library to draw, PixelLab owns an array of pixels
// and sets each one itself. The finished array is written to a .bmp image file,
// so the program needs nothing except the C++ standard library.
//
// This is a raster operation: the whole picture is just width * height colours.
#pragma once
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

#include "types.hpp"

namespace gfx {

class Framebuffer {
public:
    Framebuffer(int width, int height) : w_(width), h_(height), px_(static_cast<std::size_t>(width) * height, Color{0, 0, 0, 0}) {}  // starts fully transparent

    int width() const { return w_; }
    int height() const { return h_; }

    bool inBounds(int x, int y) const { return x >= 0 && y >= 0 && x < w_ && y < h_; }

    // Pixel (x, y) lives at index y * width + x in the flat array.
    // Off-screen pixels are ignored: this is "clipping".
    void set(int x, int y, Color c) {
        if (!inBounds(x, y)) return;
        px_[static_cast<std::size_t>(y) * w_ + x] = c;
    }

    Color get(int x, int y) const {
        if (!inBounds(x, y)) return Color{0, 0, 0, 0};
        return px_[static_cast<std::size_t>(y) * w_ + x];
    }

    // One integer per colour (ignores alpha); handy for comparing pixels.
    std::uint32_t key(int x, int y) const {
        Color c = get(x, y);
        return (static_cast<std::uint32_t>(c.r) << 16) | (static_cast<std::uint32_t>(c.g) << 8) | c.b;
    }

    void clear(Color c) {
        for (auto& p : px_) p = c;
    }

    // Copy another framebuffer into this one with its top-left corner at (ox, oy).
    void blit(const Framebuffer& src, int ox, int oy) {
        for (int y = 0; y < src.h_; ++y)
            for (int x = 0; x < src.w_; ++x) set(ox + x, oy + y, src.get(x, y));
    }

    // Write a 24-bit .bmp file. `scale` enlarges every pixel into a scale x scale
    // block (nearest neighbour) so a 64x64 picture is easy to see.
    // Transparent pixels (alpha 0) are written as white.
    bool saveBMP(const std::string& path, int scale = 1) const {
        const int outW = w_ * scale, outH = h_ * scale;
        const int rowBytes = (outW * 3 + 3) & ~3;  // rows are padded to a multiple of 4 bytes
        const std::uint32_t dataSize = static_cast<std::uint32_t>(rowBytes) * outH;
        const std::uint32_t fileSize = 54 + dataSize;

        std::ofstream f(path, std::ios::binary);
        if (!f) return false;

        auto u16 = [&](std::uint16_t v) { f.put(char(v & 0xFF)); f.put(char((v >> 8) & 0xFF)); };
        auto u32 = [&](std::uint32_t v) {
            for (int i = 0; i < 4; ++i) f.put(char((v >> (8 * i)) & 0xFF));
        };

        // BITMAPFILEHEADER (14 bytes)
        f.put('B'); f.put('M');
        u32(fileSize); u16(0); u16(0); u32(54);
        // BITMAPINFOHEADER (40 bytes)
        u32(40); u32(static_cast<std::uint32_t>(outW)); u32(static_cast<std::uint32_t>(outH));
        u16(1); u16(24); u32(0); u32(dataSize); u32(2835); u32(2835); u32(0); u32(0);

        std::vector<char> row(static_cast<std::size_t>(rowBytes), 0);
        for (int oy = outH - 1; oy >= 0; --oy) {  // BMP stores the bottom row first
            const int y = oy / scale;
            for (int ox = 0; ox < outW; ++ox) {
                Color c = get(ox / scale, y);
                if (c.a == 0) c = Color{255, 255, 255, 255};
                row[static_cast<std::size_t>(ox) * 3 + 0] = static_cast<char>(c.b);  // BMP order is B, G, R
                row[static_cast<std::size_t>(ox) * 3 + 1] = static_cast<char>(c.g);
                row[static_cast<std::size_t>(ox) * 3 + 2] = static_cast<char>(c.r);
            }
            f.write(row.data(), rowBytes);
        }
        return static_cast<bool>(f);
    }

private:
    int w_, h_;
    std::vector<Color> px_;
};

}  // namespace gfx
