#include "raster.h"
#include "../math/math_utils.h"
#include <stdlib.h>

static inline float edge(vec3 a, vec3 b, vec2 p) {
  return (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
}

/* Checks if the triangle is out of screen */
static inline int is_on_screen(FrameBuffer* fb, vec3 p0, vec3 p1, vec3 p2,
                               float minx, float miny, float maxx, float maxy) {
  if (maxx < 0 || maxy < 0 || minx > fb->w || miny > fb->h)
    return 0;
  return 1;
}

void draw_triangle(FrameBuffer* fb, vec3 p0, vec3 p1, vec3 p2,
                   vec3 c0, vec3 c1, vec3 c2) {
  float S = edge(p0, p1, make2(p2.x, p2.y));
  if (fabsf(S) < 1e-8f)
    return;

  float minx = min3f(p0.x, p1.x, p2.x), maxx = max3f(p0.x, p1.x, p2.x);
  float miny = min3f(p0.y, p1.y, p2.y), maxy = max3f(p0.y, p1.y, p2.y);
  if (!is_on_screen(fb, p0, p1, p2, minx, miny, maxx, maxy)) {
    return;
  }

  /* Bounding box process */
  int x0 = clampi((int)floorf(minx), 0, fb->w - 1);
  int y0 = clampi((int)floorf(miny), 0, fb->h - 1);
  int x1 = clampi((int)ceilf(maxx), 0, fb->w - 1);
  int y1 = clampi((int)ceilf(maxy), 0, fb->h - 1);

  float inverted_S = 1.0 / S;

  for (size_t x = x0; x <= x1; x++) {
    for (size_t y = y0; y <= y1; y++) {
      vec2 centre = make2(x + 0.5, y + 0.5);
      float e0 = edge(p1, p2, centre) * inverted_S;
      float e1 = edge(p2, p0, centre) * inverted_S;
      float e2 = edge(p0, p1, centre) * inverted_S;
      if (e0 < 0.0 || e1 < 0.0 || e2 < 0.0) {
        continue;
      }

      /* test depth */
      float z = e0 * p0.z + e1 * p1.z + e2 * p2.z;
      int idx = y * fb->w + x;
      if (z >= fb->z[idx]) {
        continue;
      }
      fb->z[idx] = z;
      const vec3 color = add3(add3(scale3(c0, e0), scale3(c1, e1)), scale3(c2, e2));
      FB_DrawPixel(fb, x, y, color);
    }
  }
}
