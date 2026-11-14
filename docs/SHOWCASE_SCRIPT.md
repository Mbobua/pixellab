# Showcase Video Script (solo, about 10 minutes)

The brief asks for one recorded showcase with **camera on** and the **shared screen visible**
during demonstrations. You are a group of one, so you present every section yourself.
Say in the first sentence: "I am working alone, so I will present all roles."

## Before recording

- [ ] Build and run once so the `output/` folder is full (`sh build.sh`).
- [ ] Open these windows ready to switch between: VS Code (the `src/` folder), a terminal in
      the project folder, and the `output/` folder (or `docs/screenshots/`) with images.
- [ ] Open `output/anim/` and be ready to flip through the frames (hold the right arrow key).
- [ ] Camera and microphone on, face visible. Record the screen and camera together
      (Zoom, Teams or Google Meet "record" works: start a meeting alone and record it).
- [ ] Upload to YouTube as **Unlisted**, copy the link.
- [ ] GitHub repository is public and up to date (check the link in a private window).

---

## 1. Proposed project vs actual project (about 1.5 min)

**Show:** slides 1, 4 and 5 of your proposal deck, then the repository on GitHub.

**Say:**
- "My proposal was PixelLab: a 64x64 raster drawing and animation studio where every pixel comes
  from my own algorithms."
- "What stayed the same: the name, the 64x64 pixel idea, and all the algorithms: Bresenham,
  midpoint circle, scanline fill, flood fill, Bezier curves and 2D matrix transformations."
- "What changed: I first wrote it in JavaScript with a mouse-driven interface. The course
  requires C++, so I rewrote the whole project in pure C++17."
- "What was removed: the browser interface, mouse tools, undo and live animation, because they
  needed a web page. I replaced them with programs that save pictures as .bmp files, which show
  the graphics output directly."
- "What I added: two demo programs that produce one picture per technique, an animation frame
  sequence, and automated tests."

## 2. Responsibilities (about 1.5 min)

**Show:** the README contributors table, the GitHub commit history.

**Say:**
- "I hold every role: algorithms, interface and output, animation, testing, documentation and integration."
- "Components and techniques I worked on: the framebuffer, all five core algorithms, Bezier curves,
  the matrix class, the shape model and the animation frames."
- "Challenges and how I handled them" (these are true for this project; add your own):
  1. *Porting JavaScript to C++.* JavaScript passes functions around freely. In C++ I used
     templates with lambdas, so each algorithm takes a `plot` or `hline` callback and stays independent
     of the framebuffer.
  2. *Output without a graphics library.* I wrote a BMP file writer: a 54-byte header, rows padded
     to a multiple of 4 bytes, pixels stored bottom row first in B, G, R order.
  3. *A test failed.* My filled-circle area test counted 4096 pixels because the framebuffer started
     opaque instead of transparent. I fixed the default.
  4. *Rotation looked messy at 64x64.* A 14-pixel house rotated 20 degrees looked blobby, so I
     rendered that picture at 128x128. This shows that resolution matters for raster graphics.

## 3. Project components (about 2 min)

**Show:** `src/shapes.hpp`, then the images `p3_6_scene.bmp` and `p3_2_transforms.bmp`.

For each component say: what it is, its purpose, how it is implemented, and what it connects to.

| Component | Purpose | Implementation | Relates to |
|---|---|---|---|
| Framebuffer | The screen as a pixel array | `Framebuffer`: a vector of colours, `set` clips off-screen pixels | Everything draws into it |
| Line | A straight segment | `Shape` with 2 points | Drawn by Bresenham |
| Circle | A round shape | 1 point + radius | Drawn by midpoint circle |
| Polygon / rectangle | Walls, roofs, grass | List of points | Outline by Bresenham, fill by scanline |
| Bezier curve | Smooth path or river | 4 control points | Sampled, joined with Bresenham lines |
| Shape group | The house = wall + roof + door | `vector<Shape>` | Transformed together with one matrix |

Explain the data flow: **points, then matrix, then algorithm, then pixels, then .bmp file.**
Open `p3_6_scene.bmp` and point out each component in the final scene.

## 4. Techniques and algorithms (about 4 min, the main part)

For **each** technique answer the brief's five questions: *what, how, which component, what it achieves,
demonstrate.* Run the program (`./phase2_demo`, `./phase3_demo`) in the terminal and open the picture.

### Bresenham line (`p2_1_lines.bmp`)
- **What:** draws a line using only integers.
- **How:** walk one pixel at a time along the long axis; an error term tracks how far the true line is
  from the pixel centre; when it passes half a pixel, step sideways too.
- **Component:** lines, polygon outlines, curve segments.
- **Achieves:** straight lines in every direction (the picture uses 16 directions to cover all 8 octants).

### Midpoint circle (`p2_2_circles.bmp`)
- **How:** compute one eighth of the circle and mirror each pixel 8 ways. A decision value `d` tests whether
  the midpoint between two candidate pixels is inside or outside the circle. `d = 1 - r`, then
  `d += 2x + 3` or `d += 2(x - y) + 5`.
- **Component:** circle outline and filled disc (the same loop filling horizontal spans).

### Scanline polygon fill (`p2_3_polygon_fill.bmp`)
- **How:** for each row, find where it crosses the polygon edges, sort the crossings, fill between pairs
  (inside, outside, inside...). This is the even-odd rule. Sample at `y + 0.5` so shared vertices are not counted twice.
- **Show:** the triangle, the concave L shape, and the star whose centre stays empty. That is the even-odd rule.

### Flood fill (`p2_4_flood_fill.bmp`)
- **How:** from a seed pixel, spread to up, down, left and right neighbours with the same colour. It uses an
  explicit stack, not recursion, so large areas cannot overflow the call stack.
- **Show:** the ring and the box inside it are filled with different colours; the fill stops at the outline.

### Cubic Bezier (`p3_1_bezier.bmp`)
- **How (de Casteljau):** interpolate between neighbouring control points, 4 to 3 to 2 to 1 points; the last
  point lies on the curve. The curve starts at P0, ends at P3 and is only pulled towards P1 and P2.
- **Show:** the grey control polygon and black control points drawn on the picture.

### 2D transformations with matrices (`p3_2_transforms.bmp`, `p3_4_order.bmp`)
- **How:** a point `(x, y)` becomes `[x, y, 1]`; translate, rotate and scale are all 3x3 matrices, so they combine
  into one matrix by multiplication.
- **Show the matrices** printed by `phase3_demo` in the terminal:
  translate `[1 0 tx; 0 1 ty; 0 0 1]`, rotate `[cos -sin 0; sin cos 0; 0 0 1]`, scale `[sx 0 0; 0 sy 0; 0 0 1]`.
- **Show:** four houses: translated, rotated 20 degrees, scaled 1.4 times, flipped.
- **Order matters** (`p3_4_order.bmp`): the blue square is `T * R` (rotate first, then move); the red one is
  `R * T` (move first, then rotate). Same two matrices, different result. The terminal prints both matrices.

### Rotation about a pivot (`p3_3_pivot.bmp`)
- **How:** rotation matrices rotate about the origin, so we use `T(c) * R * T(-c)`: move the pivot to the
  origin, rotate, move back.
- **Show:** left side rotates about the screen origin and swings away; right side rotates about its own centre and
  spins in place.

### Animation (`p3_5_animation.bmp`, `output/anim/`)
- **How:** each frame applies a different angle through a matrix: a square spins about its own centre, a circle
  orbits the screen centre, a circle pulses (scale about its centre) and a clock hand rotates.
- **Show:** the 8-frame filmstrip, then flip through `output/anim/frame_000.bmp` to `frame_035.bmp`.

### Testing
- Run `./run_tests` and show **20 passed, 0 failed**. Mention a few: Bresenham pixels stay within 0.5 px of the
  true line, circle pixels stay within 0.75 px of the radius, the Bezier midpoint matches the Bernstein formula, and
  scale-then-translate differs from translate-then-scale.

## 5. Recommendations (about 1 min)

**Say:**
- **Future additions:** a live window using SDL2 or OpenGL for real-time animation and mouse tools; thick lines;
  anti-aliasing (Xiaolin Wu's line algorithm); Bezier control-point dragging; 3D wireframe with perspective projection.
- **What I would remove or change:** the 64x64 limit is too small for some rotations, so a resolution setting is a good idea.
- **Lessons learnt:** keep algorithms independent of the output device (callbacks); write tests against known
  mathematical results; check the course requirements (language) at the start; commit to GitHub regularly.

## Closing

"Thank you. The source code is in my GitHub repository, linked in the submission."

---

## Submission checklist

- [ ] Unlisted YouTube URL
- [ ] GitHub URL (public, latest commit includes the C++ code and screenshots)
- [ ] Camera visible throughout; screen share visible during demonstrations
- [ ] You can explain every algorithm without reading the code (practise once out loud)
