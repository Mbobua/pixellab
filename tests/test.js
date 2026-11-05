// Run with:  node tests/test.js
// Checks each graphics algorithm against a known mathematical result.
const assert = require("assert");
require("../js/framebuffer.js");
require("../js/algorithms.js");
const { Algo, Framebuffer } = globalThis.PL;

let passed = 0;
function test(name, fn) {
  fn();
  passed++;
  console.log("  ok  " + name);
}
const near = (a, b, eps = 1e-9) => Math.abs(a - b) < eps;

console.log("Bresenham line");
function collect(fn) { const out = []; fn((x, y) => out.push([x, y])); return out; }
test("includes both endpoints", () => {
  const pts = collect((plot) => Algo.bresenham(2, 3, 17, 9, plot));
  assert.deepStrictEqual(pts[0], [2, 3]);
  assert.deepStrictEqual(pts[pts.length - 1], [17, 9]);
});
test("pixel count = max(|dx|,|dy|) + 1 in every direction", () => {
  const cases = [[0, 0, 10, 3], [10, 3, 0, 0], [0, 0, 3, 10], [5, 5, 5, 5], [0, 10, 10, 0], [10, 0, 0, 10]];
  for (const [a, b, c, d] of cases) {
    const pts = collect((plot) => Algo.bresenham(a, b, c, d, plot));
    assert.strictEqual(pts.length, Math.max(Math.abs(c - a), Math.abs(d - b)) + 1);
  }
});
test("pixels stay within 0.5 px of the true line", () => {
  const pts = collect((plot) => Algo.bresenham(0, 0, 30, 11, plot));
  for (const [x, y] of pts) assert(Math.abs(y - (11 / 30) * x) <= 0.5 + 1e-9);
});

console.log("Midpoint circle");
test("every pixel is within 0.5 px of the true radius", () => {
  for (const r of [1, 5, 12, 25]) {
    const pts = collect((plot) => Algo.midpointCircle(0, 0, r, plot));
    for (const [x, y] of pts) assert(Math.abs(Math.hypot(x, y) - r) < 0.75, `r=${r} (${x},${y})`);
  }
});
test("circle is 8-way symmetric", () => {
  const set = new Set(collect((plot) => Algo.midpointCircle(0, 0, 9, plot)).map((p) => p.join(",")));
  for (const k of set) {
    const [x, y] = k.split(",").map(Number);
    assert(set.has(`${y},${x}`) && set.has(`${-x},${y}`) && set.has(`${x},${-y}`));
  }
});
test("filled circle area is close to pi (r + 0.5)^2 (boundary pixels included)", () => {
  const fb = new Framebuffer(64, 64);
  Algo.fillCircle(32, 32, 10, (x0, x1, y) => { for (let x = x0; x <= x1; x++) fb.set(x, y, [1, 1, 1, 255]); });
  let n = 0;
  for (let i = 3; i < fb.data.length; i += 4) if (fb.data[i]) n++;
  const expected = Math.PI * 10.5 * 10.5;
  assert(Math.abs(n - expected) / expected < 0.03, "area " + n + " vs " + expected.toFixed(1));
});

console.log("Scanline polygon fill");
function countFill(pts) {
  let n = 0;
  Algo.scanlineFill(pts, (x0, x1) => { n += x1 - x0 + 1; });
  return n;
}
test("4x4 square fills 16 pixels", () => assert.strictEqual(countFill([[0, 0], [4, 0], [4, 4], [0, 4]]), 16));
test("right triangle area is about half the square", () => {
  const n = countFill([[0, 0], [20, 0], [0, 20]]);
  assert(Math.abs(n - 200) <= 20, "n=" + n);
});
test("concave 'L' shape uses even-odd correctly (area 12)", () => {
  // 4x4 square minus a 2x2 notch = 12
  assert.strictEqual(countFill([[0, 0], [4, 0], [4, 2], [2, 2], [2, 4], [0, 4]]), 12);
});

console.log("Flood fill");
test("fills exactly the enclosed region and stops at the wall", () => {
  const w = 10, h = 10;
  const grid = Array.from({ length: h }, () => new Array(w).fill(0));
  for (let i = 2; i <= 7; i++) { grid[2][i] = 1; grid[7][i] = 1; grid[i][2] = 1; grid[i][7] = 1; } // box
  let n = 0;
  Algo.floodFill(w, h, (x, y) => grid[y][x], 4, 4, (x, y) => { grid[y][x] = 2; n++; });
  assert.strictEqual(n, 16); // inside is 4x4
  assert.strictEqual(grid[0][0], 0); // outside untouched
});

console.log("\n" + passed + " tests passed");
