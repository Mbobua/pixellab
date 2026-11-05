# PixelLab - Raster Graphics Studio (work in progress)

PixelLab is a 64x64 pixel drawing and animation studio. It does **not** use the browser's
drawing functions (`lineTo`, `arc`, `fill`): every pixel is set by graphics algorithms written
from scratch in this repository.

## Status: Phase 2 - Core Graphics (done so far)

| Technique | File | Status |
|---|---|---|
| Framebuffer (own pixel array, clipping, layers) | `js/framebuffer.js` | Done |
| Bresenham line algorithm | `js/algorithms.js` | Done |
| Midpoint circle algorithm (outline + filled) | `js/algorithms.js` | Done |
| Scanline polygon fill (even-odd rule) | `js/algorithms.js` | Done |
| Flood fill (4-connected, iterative stack) | `js/algorithms.js` | Done |
| Algorithm tests | `tests/test.js` | Done |
| Drawing tools and interface | - | Planned |
| Bezier curves | - | Planned |
| 2D transformations (3x3 matrices) | - | Planned |
| Animation | - | Planned |

## How to run

1. Download or clone this repository.
2. Open `idex.html` in a browser (click inside a shape to flood-fill it).
3. Optional tests (needs Node.js): `node tests/test.js`

## Technologies

HTML5 canvas (only to display the finished pixel array), plain JavaScript, Node.js `assert` for tests, Git/GitHub.

## Contributors

| Name | GitHub | Contribution |
|---|---|---|
| William Githinji | Mbobua | Solo project: all design, algorithms, testing and documentation |
