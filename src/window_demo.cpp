// window_demo.cpp - live window using OpenGL (through GLFW).
//
// All drawing is still done by OUR algorithms into a 64 x 64 Framebuffer (see viewer.hpp).
// OpenGL is used for one job only: show that finished pixel array in a window. Each frame we
// upload the pixels as a texture and draw it on a rectangle that fills the window.
//
//   keys:  arrows move the house      Q / E   rotate        + / -   scale        F   flip
//          SPACE start/stop animation 1 pencil  2 bucket    N   next colour      C   clear paint
//          R reset                    S save screenshot     ESC quit
//   mouse: left button draws (pencil) or flood-fills (bucket)
#include <cmath>
#include <cstdio>
#include <string>

#include <GLFW/glfw3.h>

#include "viewer.hpp"

using namespace gfx;

static Viewer* gViewer = nullptr;
static int gShots = 0;

static void onKey(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
    if (action == GLFW_RELEASE || gViewer == nullptr) return;
    const bool pressed = (action == GLFW_PRESS);  // GLFW_REPEAT (key held down) is used only for movement
    switch (key) {
        case GLFW_KEY_LEFT:  gViewer->command(Command::MoveLeft); break;
        case GLFW_KEY_RIGHT: gViewer->command(Command::MoveRight); break;
        case GLFW_KEY_UP:    gViewer->command(Command::MoveUp); break;
        case GLFW_KEY_DOWN:  gViewer->command(Command::MoveDown); break;
        case GLFW_KEY_Q:     gViewer->command(Command::RotateCCW); break;
        case GLFW_KEY_E:     gViewer->command(Command::RotateCW); break;
        case GLFW_KEY_EQUAL:
        case GLFW_KEY_KP_ADD:      gViewer->command(Command::ScaleUp); break;
        case GLFW_KEY_MINUS:
        case GLFW_KEY_KP_SUBTRACT: gViewer->command(Command::ScaleDown); break;
        default: break;
    }
    if (!pressed) return;
    switch (key) {
        case GLFW_KEY_F:      gViewer->command(Command::FlipH); break;
        case GLFW_KEY_SPACE:  gViewer->command(Command::ToggleAnimation); break;
        case GLFW_KEY_1:      gViewer->command(Command::ToolPencil); break;
        case GLFW_KEY_2:      gViewer->command(Command::ToolBucket); break;
        case GLFW_KEY_N:      gViewer->command(Command::NextColour); break;
        case GLFW_KEY_C:      gViewer->command(Command::ClearPaint); break;
        case GLFW_KEY_R:      gViewer->command(Command::Reset); break;
        case GLFW_KEY_S: {
            const std::string name = "screenshot_" + std::to_string(++gShots) + ".bmp";
            if (gViewer->saveScreenshot(name)) std::printf("Saved %s\n", name.c_str());
            break;
        }
        case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GLFW_TRUE); break;
        default: break;
    }
}

// Convert the mouse position (window coordinates) to a pixel of the 64 x 64 screen.
static void cursorPixel(GLFWwindow* window, int& px, int& py) {
    double x = 0, y = 0;
    glfwGetCursorPos(window, &x, &y);
    int w = 1, h = 1;
    glfwGetWindowSize(window, &w, &h);
    px = static_cast<int>(std::floor(x / w * Viewer::kSize));
    py = static_cast<int>(std::floor(y / h * Viewer::kSize));
    if (px < 0) px = 0;
    if (py < 0) py = 0;
    if (px > Viewer::kSize - 1) px = Viewer::kSize - 1;
    if (py > Viewer::kSize - 1) py = Viewer::kSize - 1;
}

int main() {
    if (!glfwInit()) {
        std::fprintf(stderr, "Could not start GLFW.\n");
        return 1;
    }
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);  // fixed size keeps the picture square
    GLFWwindow* window = glfwCreateWindow(640, 640, "PixelLab", nullptr, nullptr);
    if (!window) {
        std::fprintf(stderr, "Could not open a window (is OpenGL available?).\n");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // wait for the screen refresh (about 60 frames per second)

    Viewer viewer;
    gViewer = &viewer;
    glfwSetKeyCallback(window, onKey);

    // One texture holds our 64 x 64 picture. NEAREST keeps the big square pixels sharp.
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Viewer::kSize, Viewer::kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glEnable(GL_TEXTURE_2D);

    std::printf("PixelLab live window\n"
                "  arrows move | Q/E rotate | +/- scale | F flip | SPACE animate\n"
                "  1 pencil | 2 bucket | N colour | C clear paint | R reset | S screenshot | ESC quit\n"
                "  left mouse button draws or fills\n");

    bool wasDown = false;
    double last = glfwGetTime(), titleTimer = 0.0;
    int frames = 0;

    while (!glfwWindowShouldClose(window)) {
        const double now = glfwGetTime();
        const double dt = now - last;
        last = now;

        // mouse: pressed / held / released
        const bool down = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        int px = 0, py = 0;
        cursorPixel(window, px, py);
        if (down && !wasDown) viewer.mouseDown(px, py);
        else if (down && wasDown) viewer.mouseDrag(px, py);
        else if (!down && wasDown) viewer.mouseUp();
        wasDown = down;

        viewer.update(dt);   // advance the animation
        viewer.render();     // OUR algorithms draw the picture into the framebuffer

        // OpenGL part: upload the pixels and draw them as one rectangle filling the window.
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, Viewer::kSize, Viewer::kSize, GL_RGBA, GL_UNSIGNED_BYTE,
                        viewer.framebuffer().data());
        int fw = 0, fh = 0;
        glfwGetFramebufferSize(window, &fw, &fh);
        glViewport(0, 0, fw, fh);
        glClear(GL_COLOR_BUFFER_BIT);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, 1.0f);   // top-left
        glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 1.0f);    // top-right
        glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, -1.0f);   // bottom-right
        glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f, -1.0f);  // bottom-left
        glEnd();

        glfwSwapBuffers(window);
        glfwPollEvents();

        ++frames;
        titleTimer += dt;
        if (titleTimer >= 0.5) {
            const std::string title = viewer.status() + " | " + std::to_string(static_cast<int>(frames / titleTimer)) + " fps";
            glfwSetWindowTitle(window, title.c_str());
            frames = 0;
            titleTimer = 0.0;
        }
    }

    glDeleteTextures(1, &texture);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
