/*
 * algorithms.js - the classic raster graphics algorithms, written from scratch.
 *
 * None of these use canvas drawing functions. Each one decides WHICH pixels to
 * light up and reports them through a callback:
 *     plot(x, y)        - light one pixel
 *     hline(x0, x1, y)  - light a horizontal run of pixels (used for fills)
 */
(function (root) {
  const PL = (root.PL = root.PL || {});

  /* ------------------------------------------------------------------ *
   * 1. Bresenham line algorithm
   *
   * Idea: walk along the line one pixel at a time. Keep an "error" value that
   * measures how far the true line is from the pixel centre. When the error
   * grows past half a pixel, step sideways too. Only integer additions are
   * needed - no division, no rounding, no floating point.
   *
   * This version handles every octant using the sign of dx and dy.
   * ------------------------------------------------------------------ */
  function bresenham(x0, y0, x1, y1, plot) {
    const dx = Math.abs(x1 - x0);
    const dy = -Math.abs(y1 - y0);
    const sx = x0 < x1 ? 1 : -1;
    const sy = y0 < y1 ? 1 : -1;
    let err = dx + dy; // error term: dx - |dy|
    while (true) {
      plot(x0, y0);
      if (x0 === x1 && y0 === y1) break;
      const e2 = 2 * err;
      if (e2 >= dy) { err += dy; x0 += sx; } // step in x
      if (e2 <= dx) { err += dx; y0 += sy; } // step in y
    }
  }

  /* ------------------------------------------------------------------ *
   * 2. Midpoint circle algorithm
   *
   * A circle is 8-way symmetric, so we only compute the arc from the top
   * (0, r) to the 45-degree point, then mirror each pixel into 8 places.
   * At each step we test the MIDPOINT between the two candidate pixels with
   * the circle equation x^2 + y^2 - r^2 (a "decision parameter" d):
   *     d < 0  -> midpoint inside circle -> choose pixel (x+1, y)
   *     d >= 0 -> midpoint outside       -> choose pixel (x+1, y-1)
   * ------------------------------------------------------------------ */
  function midpointCircle(cx, cy, r, plot) {
    let x = 0, y = r;
    let d = 1 - r; // initial decision parameter
    while (x <= y) {
      plot(cx + x, cy + y); plot(cx - x, cy + y);
      plot(cx + x, cy - y); plot(cx - x, cy - y);
      plot(cx + y, cy + x); plot(cx - y, cy + x);
      plot(cx + y, cy - x); plot(cx - y, cy - x);
      if (d < 0) {
        d += 2 * x + 3;
      } else {
        d += 2 * (x - y) + 5;
        y--;
      }
      x++;
    }
  }

  // Same loop, but instead of plotting 8 dots it fills horizontal spans, which
  // gives a solid disc.
  function fillCircle(cx, cy, r, hline) {
    let x = 0, y = r, d = 1 - r;
    while (x <= y) {
      hline(cx - x, cx + x, cy + y);
      hline(cx - x, cx + x, cy - y);
      hline(cx - y, cx + y, cy + x);
      hline(cx - y, cx + y, cy - x);
      if (d < 0) d += 2 * x + 3;
      else { d += 2 * (x - y) + 5; y--; }
      x++;
    }
  }

  /* ------------------------------------------------------------------ *
   * 3. Scanline polygon fill (even-odd rule)
   *
   * For every horizontal scanline y:
   *   1. find where the scanline crosses each polygon edge,
   *   2. sort the crossings left to right,
   *   3. fill between crossing 1-2, 3-4, 5-6 ... (inside/outside alternates).
   * We sample at the pixel centre (y + 0.5) so shared vertices are not
   * counted twice.
   * ------------------------------------------------------------------ */
  function scanlineFill(pts, hline) {
    if (pts.length < 3) return;
    let minY = Infinity, maxY = -Infinity;
    for (const p of pts) {
      minY = Math.min(minY, p[1]);
      maxY = Math.max(maxY, p[1]);
    }
    const yStart = Math.floor(minY), yEnd = Math.ceil(maxY);
    for (let y = yStart; y <= yEnd; y++) {
      const yc = y + 0.5;
      const xs = [];
      for (let i = 0; i < pts.length; i++) {
        const a = pts[i], b = pts[(i + 1) % pts.length];
        const lo = Math.min(a[1], b[1]), hi = Math.max(a[1], b[1]);
        if (lo === hi) continue;              // horizontal edge: skip
        if (yc < lo || yc >= hi) continue;    // scanline misses this edge
        const t = (yc - a[1]) / (b[1] - a[1]);
        xs.push(a[0] + t * (b[0] - a[0]));    // x where the edge crosses
      }
      xs.sort((p, q) => p - q);
      for (let k = 0; k + 1 < xs.length; k += 2) {
        // pixel x is inside when its centre (x + 0.5) lies in [xs[k], xs[k+1])
        const xa = Math.ceil(xs[k] - 0.5);
        const xb = Math.ceil(xs[k + 1] - 0.5) - 1;
        if (xb >= xa) hline(xa, xb, y);
      }
    }
  }

  /* ------------------------------------------------------------------ *
   * 4. Flood fill (4-connected, iterative stack)
   *
   * Start at a seed pixel, remember its colour, and spread to the up / down /
   * left / right neighbours that have the SAME colour. `getKey(x, y)` returns
   * the colour of a pixel as a number; `plot` paints a pixel.
   * An explicit stack is used instead of recursion so big areas cannot
   * overflow the call stack.
   * ------------------------------------------------------------------ */
  function floodFill(w, h, getKey, sx, sy, plot) {
    if (sx < 0 || sy < 0 || sx >= w || sy >= h) return 0;
    const target = getKey(sx, sy);
    const visited = new Uint8Array(w * h);
    const stack = [[sx, sy]];
    let count = 0;
    while (stack.length) {
      const [x, y] = stack.pop();
      if (x < 0 || y < 0 || x >= w || y >= h) continue;
      const idx = y * w + x;
      if (visited[idx]) continue;
      if (getKey(x, y) !== target) continue;
      visited[idx] = 1;
      plot(x, y);
      count++;
      stack.push([x + 1, y], [x - 1, y], [x, y + 1], [x, y - 1]);
    }
    return count;
  }

  PL.Algo = { bresenham, midpointCircle, fillCircle, scanlineFill, floodFill };
})(typeof window !== "undefined" ? window : globalThis);
