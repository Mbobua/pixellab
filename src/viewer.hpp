// viewer.hpp - the interactive PixelLab scene, with NO window or OpenGL code in it.
//
// This class holds everything the live window needs: the scene, the user's commands
// (move / rotate / scale / animate / pencil / bucket fill) and the rendering. It draws into a
// Framebuffer using our own algorithms. window_demo.cpp only has to copy that framebuffer to
// the screen with OpenGL. Keeping the two apart means the logic can be tested without a window.
#pragma once
#include <array>
#include <string>
#include <vector>

#include "algorithms.hpp"
#include "framebuffer.hpp"
#include "matrix.hpp"
#include "shapes.hpp"
#include "types.hpp"

namespace gfx {

enum class Command {
    MoveLeft, MoveRight, MoveUp, MoveDown,
    RotateCW, RotateCCW, ScaleUp, ScaleDown, FlipH,
    ToggleAnimation, ToolPencil, ToolBucket, NextColour, ClearPaint, Reset
};

enum class Tool { Pencil, Bucket };

class Viewer {
public:
    static constexpr int kSize = 64;  // the screen is 64 x 64 pixels

    Viewer() : fb_(kSize, kSize), paint_(kSize, kSize) {
        const Color wall = hexToColor("#8a5a2b"), roof = hexToColor("#d9381e"), door = hexToColor("#3b2a18");
        // The house is a GROUP of shapes in its own local coordinates; one matrix places all of it.
        house_.push_back(makeRect(0, 8, 14, 22, wall, true));
        house_.push_back(makePolygon({{-2, 8}, {7, 0}, {16, 8}}, roof, true));
        house_.push_back(makeRect(5, 14, 9, 22, door, true));
        palette_ = {hexToColor("#d9381e"), hexToColor("#1e6fd9"), hexToColor("#2e9e4f"),
                    hexToColor("#f2a900"), hexToColor("#7b3fb8"), Color{0, 0, 0, 255}, Color{255, 255, 255, 255}};
        reset();
    }

    // ------------------------------------------------------------ commands
    void command(Command c) {
        switch (c) {
            case Command::MoveLeft:  houseM_ = Mat3::translate(-2, 0) * houseM_; break;
            case Command::MoveRight: houseM_ = Mat3::translate(2, 0) * houseM_; break;
            case Command::MoveUp:    houseM_ = Mat3::translate(0, -2) * houseM_; break;
            case Command::MoveDown:  houseM_ = Mat3::translate(0, 2) * houseM_; break;
            case Command::RotateCW:  aboutHouse(Mat3::rotate(toRadians(15))); break;
            case Command::RotateCCW: aboutHouse(Mat3::rotate(toRadians(-15))); break;
            case Command::ScaleUp:   aboutHouse(Mat3::scale(1.2, 1.2)); break;
            case Command::ScaleDown:  aboutHouse(Mat3::scale(1 / 1.2, 1 / 1.2)); break;
            case Command::FlipH:     aboutHouse(Mat3::scale(-1, 1)); break;
            case Command::ToggleAnimation: animating_ = !animating_; break;
            case Command::ToolPencil: tool_ = Tool::Pencil; break;
            case Command::ToolBucket: tool_ = Tool::Bucket; break;
            case Command::NextColour: colourIdx_ = (colourIdx_ + 1) % static_cast<int>(palette_.size()); break;
            case Command::ClearPaint: paint_.clear(Color{0, 0, 0, 0}); break;
            case Command::Reset: reset(); break;
        }
    }

    // Time passes only while the animation is switched on.
    void update(double dt) {
        if (animating_) animT_ += dt;
    }

    // ------------------------------------------------------------ mouse (pixel coordinates)
    void mouseDown(int px, int py) {
        if (!paint_.inBounds(px, py)) return;
        if (tool_ == Tool::Pencil) {
            paint_.set(px, py, colour());
            lastX_ = px; lastY_ = py;
            dragging_ = true;
        } else {
            bucketFill(px, py);
        }
    }
    void mouseDrag(int px, int py) {
        if (!dragging_ || tool_ != Tool::Pencil) return;
        const Color c = colour();
        bresenham(lastX_, lastY_, px, py, [&](int x, int y) { paint_.set(x, y, c); });
        lastX_ = px; lastY_ = py;
    }
    void mouseUp() { dragging_ = false; }

    // ------------------------------------------------------------ drawing
    // Draw the whole scene into the framebuffer using our own algorithms.
    void render(bool showMarker = true) {
        fb_.clear(Color{255, 255, 255, 255});
        draw(fb_, makeRect(0, 0, 63, 42, hexToColor("#87ceeb"), true));          // sky

        // Sun: rotates about a point on the ground, so it sets behind the grass and comes round again.
        Shape sun = makeCircle({52, 10}, 6, hexToColor("#f2a900"), true);
        transform(sun, Mat3::about(Mat3::rotate(toRadians(animT_ * 20.0)), 32, 42));
        draw(fb_, sun);

        draw(fb_, makeRect(0, 42, 63, 63, hexToColor("#2e9e4f"), true));         // grass
        for (const Shape& base : house_) {                                       // house group
            Shape s = base;
            transform(s, houseM_);
            draw(fb_, s);
        }
        draw(fb_, makeBezier({0, 58}, {20, 46}, {40, 62}, {63, 52}, hexToColor("#1e6fd9")));  // path
        draw(fb_, makeLine({48, 26}, {48, 50}, Color{0, 0, 0, 255}));                         // pole

        // Ring on the pole: pulses (scales about its own centre) while animating.
        Shape ring = makeCircle({48, 22}, 4, hexToColor("#7b3fb8"), false);
        const double k = 1.0 + 0.4 * std::sin(animT_ * 3.0);
        transform(ring, Mat3::about(Mat3::scale(k, k), 48, 22));
        draw(fb_, ring);

        fb_.overlay(paint_);  // the user's pencil strokes and bucket fills go on top

        if (showMarker) {     // small blue cross at the house's pivot (its centre)
            const Vec2 c = houseCentre();
            const Color m{0, 120, 255, 255};
            const int cx = roundi(c.x), cy = roundi(c.y);
            fb_.set(cx, cy, m); fb_.set(cx - 1, cy, m); fb_.set(cx + 1, cy, m);
            fb_.set(cx, cy - 1, m); fb_.set(cx, cy + 1, m);
        }
    }

    // ------------------------------------------------------------ information
    const Framebuffer& framebuffer() const { return fb_; }
    Vec2 houseCentre() const { return houseM_.apply({7, 11}); }  // (7, 11) is the house's local centre
    bool animating() const { return animating_; }
    double time() const { return animT_; }
    Color colour() const { return palette_[colourIdx_]; }
    bool paintIsEmpty() const {
        for (int y = 0; y < kSize; ++y)
            for (int x = 0; x < kSize; ++x) if (paint_.get(x, y).a > 0) return false;
        return true;
    }
    std::string status() const {
        return std::string("PixelLab | tool: ") + (tool_ == Tool::Pencil ? "pencil" : "bucket") +
               " | colour " + std::to_string(colourIdx_ + 1) + " | animation " + (animating_ ? "ON" : "off");
    }
    bool saveScreenshot(const std::string& path) const { return fb_.saveBMP(path, 10); }

private:
    void reset() {
        paint_.clear(Color{0, 0, 0, 0});
        houseM_ = Mat3::translate(10, 26) * Mat3::scale(1.2, 1.2);  // place and enlarge the house
        animating_ = false;
        animT_ = 0.0;
        tool_ = Tool::Pencil;
        colourIdx_ = 0;
        dragging_ = false;
    }

    // Apply a transformation about the house's own centre:  T(c) * M * T(-c)  then the existing matrix.
    void aboutHouse(const Mat3& M) {
        const Vec2 c = houseCentre();
        houseM_ = Mat3::about(M, c.x, c.y) * houseM_;
    }

    // Bucket tool: flood fill on the picture as the user sees it, writing into the paint layer.
    void bucketFill(int px, int py) {
        render(false);
        const Color c = colour();
        const std::uint32_t newKey = (static_cast<std::uint32_t>(c.r) << 16) | (static_cast<std::uint32_t>(c.g) << 8) | c.b;
        if (fb_.key(px, py) == newKey) return;  // already this colour: nothing to do
        floodFill(kSize, kSize, [&](int x, int y) { return fb_.key(x, y); }, px, py,
                  [&](int x, int y) { paint_.set(x, y, c); });
    }

    Framebuffer fb_;
    Framebuffer paint_;
    std::vector<Shape> house_;
    std::vector<Color> palette_;
    Mat3 houseM_;
    bool animating_ = false;
    double animT_ = 0.0;
    Tool tool_ = Tool::Pencil;
    int colourIdx_ = 0;
    bool dragging_ = false;
    int lastX_ = 0, lastY_ = 0;
};

}  // namespace gfx
