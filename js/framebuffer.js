/*
 * framebuffer.js - our own tiny "screen" in memory.
 *
 * Instead of calling canvas.lineTo() / arc() / fill() (which hide how drawing
 * works), PixelLab owns an array of pixels and sets each one itself.
 * The finished array is copied to the <canvas> once per frame with
 * putImageData. This is a raster operation: the whole picture is just
 * width * height * 4 numbers (R, G, B, A).
 */
(function (root) {
  const PL = (root.PL = root.PL || {});

  function hexToRgba(hex) {
    const h = hex.replace("#", "");
    return [
      parseInt(h.slice(0, 2), 16),
      parseInt(h.slice(2, 4), 16),
      parseInt(h.slice(4, 6), 16),
      255,
    ];
  }

  class Framebuffer {
    constructor(w, h) {
      this.w = w;
      this.h = h;
      this.data = new Uint8ClampedArray(w * h * 4);
    }

    inBounds(x, y) {
      return x >= 0 && y >= 0 && x < this.w && y < this.h;
    }

    // Pixel (x, y) starts at index (y * width + x) * 4 in the flat array.
    set(x, y, rgba) {
      if (!this.inBounds(x, y)) return; // clipping: ignore off-screen pixels
      const i = (y * this.w + x) * 4;
      this.data[i] = rgba[0];
      this.data[i + 1] = rgba[1];
      this.data[i + 2] = rgba[2];
      this.data[i + 3] = rgba[3];
    }

    get(x, y) {
      if (!this.inBounds(x, y)) return [0, 0, 0, 0];
      const i = (y * this.w + x) * 4;
      const d = this.data;
      return [d[i], d[i + 1], d[i + 2], d[i + 3]];
    }

    // A single integer for the colour at (x, y); handy for comparing pixels.
    key(x, y) {
      const i = (y * this.w + x) * 4;
      const d = this.data;
      return ((d[i] << 16) | (d[i + 1] << 8) | d[i + 2]) >>> 0;
    }

    clear(rgba) {
      const c = rgba || [0, 0, 0, 0];
      for (let i = 0; i < this.data.length; i += 4) {
        this.data[i] = c[0];
        this.data[i + 1] = c[1];
        this.data[i + 2] = c[2];
        this.data[i + 3] = c[3];
      }
    }

    copyFrom(other) {
      this.data.set(other.data);
    }

    clone() {
      const f = new Framebuffer(this.w, this.h);
      f.copyFrom(this);
      return f;
    }

    // Draw `layer` on top of this buffer: opaque pixels replace what is below.
    overlay(layer) {
      const s = layer.data, d = this.data;
      for (let i = 0; i < s.length; i += 4) {
        if (s[i + 3] > 0) {
          d[i] = s[i]; d[i + 1] = s[i + 1]; d[i + 2] = s[i + 2]; d[i + 3] = 255;
        }
      }
    }
  }

  PL.Framebuffer = Framebuffer;
  PL.hexToRgba = hexToRgba;
})(typeof window !== "undefined" ? window : globalThis);
