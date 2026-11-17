// viewer_tests.cpp - tests the interactive logic (commands, mouse, painting, animation) with no window.
// Build and run:  g++ -std=c++17 -I src tests/viewer_tests.cpp -o run_viewer_tests && ./run_viewer_tests
#include <cmath>
#include <iostream>

#include "viewer.hpp"

using namespace gfx;

static int passed = 0, failed = 0;
#define CHECK(name, cond)                                                   \
    do {                                                                    \
        if (cond) { ++passed; std::cout << "  ok    " << name << "\n"; }     \
        else { ++failed; std::cout << "  FAIL  " << name << "\n"; }         \
    } while (0)

static bool near(double a, double b, double eps = 1e-6) { return std::fabs(a - b) < eps; }
static bool sameColour(Color a, Color b) { return a.r == b.r && a.g == b.g && a.b == b.b; }

int main() {
    std::cout << "Viewer (interactive logic, no window)\n";
    Viewer v;
    v.render(false);
    const Color sky = hexToColor("#87ceeb");
    CHECK("sky is drawn at the top-left corner", sameColour(v.framebuffer().get(2, 2), sky));
    CHECK("grass is drawn at the bottom-left", sameColour(v.framebuffer().get(2, 60), hexToColor("#2e9e4f")));

    // --- transformations of the house group
    const Vec2 c0 = v.houseCentre();
    v.command(Command::MoveRight);
    CHECK("move right shifts the house centre by +2 in x", near(v.houseCentre().x, c0.x + 2) && near(v.houseCentre().y, c0.y));
    v.command(Command::MoveLeft);
    v.command(Command::MoveDown);
    CHECK("move down shifts the house centre by +2 in y", near(v.houseCentre().y, c0.y + 2));
    v.command(Command::MoveUp);
    v.command(Command::RotateCW);
    v.command(Command::RotateCCW);
    v.command(Command::ScaleUp);
    v.command(Command::ScaleDown);
    v.command(Command::FlipH);
    v.command(Command::FlipH);
    CHECK("rotate / scale / flip about the centre keep the centre fixed",
          near(v.houseCentre().x, c0.x, 1e-9) && near(v.houseCentre().y, c0.y, 1e-9));

    // --- animation
    CHECK("animation starts switched off", !v.animating());
    v.update(1.0);
    CHECK("time does not advance while animation is off", near(v.time(), 0.0));
    v.command(Command::ToggleAnimation);
    v.update(0.5);
    CHECK("time advances while animation is on", v.animating() && near(v.time(), 0.5));
    v.render(false);
    Viewer still;
    still.render(false);
    int differing = 0;
    for (int y = 0; y < 64; ++y)
        for (int x = 0; x < 64; ++x)
            if (v.framebuffer().key(x, y) != still.framebuffer().key(x, y)) ++differing;
    CHECK("animating changes the picture (the sun moves)", differing > 0);

    // --- pencil (Bresenham between mouse samples)
    Viewer p;
    p.mouseDown(5, 5);
    p.mouseDrag(25, 5);
    p.mouseUp();
    p.render(false);
    bool lineOk = true;
    for (int x = 5; x <= 25; ++x) lineOk = lineOk && sameColour(p.framebuffer().get(x, 5), p.colour());
    CHECK("pencil drag draws an unbroken horizontal line", lineOk);
    CHECK("paint layer is not empty after drawing", !p.paintIsEmpty());
    p.command(Command::ClearPaint);
    CHECK("clear paint empties the paint layer", p.paintIsEmpty());

    // --- bucket (flood fill)
    Viewer b;
    b.command(Command::ToolBucket);
    b.command(Command::NextColour);  // colour 2 (blue)
    b.mouseDown(2, 2);
    b.render(false);
    CHECK("bucket fills the sky with the chosen colour", sameColour(b.framebuffer().get(2, 2), b.colour()));
    CHECK("bucket fill stops at the grass", sameColour(b.framebuffer().get(2, 60), hexToColor("#2e9e4f")));
    CHECK("bucket fill stops at the house wall", sameColour(b.framebuffer().get(18, 38), hexToColor("#8a5a2b")) ||
                                                 sameColour(b.framebuffer().get(18, 42), hexToColor("#8a5a2b")));

    // --- reset
    b.command(Command::Reset);
    CHECK("reset clears the paint layer", b.paintIsEmpty());

    // --- screenshot
    Viewer s;
    s.render(true);
    CHECK("screenshot saves a .bmp file", s.saveScreenshot("viewer_test_shot.bmp"));

    std::cout << "\n" << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? 0 : 1;
}
