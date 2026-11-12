#include "raster.h"
#include "../math/math_utils.h"
#include <stdlib.h>

/* Basic pixel shader.*/
static vec3 fs_vertex_color(const Varyings* in, const void* uniforms) {
  (void)uniforms;
  return in->color;
}
static Varyings interpolate(const Varyings v[3], float b0, float b1, float b2) {
  Varyings out = {0};
  out.normals = add3(add3(scale3(v[0].normals, b0),
                          scale3(v[1].normals, b1)),
                     scale3(v[2].normals, b2));
  out.color = add3(add3(scale3(v[0].color, b0), scale3(v[1].color, b1)),
                   scale3(v[2].color, b2));
  return out;
}

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
  Varyings v[3] = {0};
  v[0].color = c0;
  v[1].color = c1;
  v[2].color = c2;
  draw_triangle_shaded(fb, p0, p1, p2, v, fs_vertex_color, NULL);
}
void draw_triangle_shaded(FrameBuffer* fb, vec3 p0, vec3 p1, vec3 p2,
                          const Varyings v[3],
                          FragShader shader, const void* uniforms) {
  float S = edge(p0, p1, make2(p2.x, p2.y));
  if (fabsf(S) < 1e-8f)
    return;

  float minx = min3f(p0.x, p1.x, p2.x), maxx = max3f(p0.x, p1.x, p2.x);
  float miny = min3f(p0.y, p1.y, p2.y), maxy = max3f(p0.y, p1.y, p2.y);
  if (!is_on_screen(fb, p0, p1, p2, minx, miny, maxx, maxy))
    return;

  int x0 = clampi((int)floorf(minx), 0, fb->w - 1);
  int y0 = clampi((int)floorf(miny), 0, fb->h - 1);
  int x1 = clampi((int)ceilf(maxx), 0, fb->w - 1);
  int y1 = clampi((int)ceilf(maxy), 0, fb->h - 1);
  float inv_S = 1.0f / S;

  for (int y = y0; y <= y1; y++) {
    for (int x = x0; x <= x1; x++) {
      vec2 c = make2(x + 0.5f, y + 0.5f);
      float b0 = edge(p1, p2, c) * inv_S;
      float b1 = edge(p2, p0, c) * inv_S;
      float b2 = edge(p0, p1, c) * inv_S;
      if (b0 < 0.0f || b1 < 0.0f || b2 < 0.0f)
        continue;

      float z = b0 * p0.z + b1 * p1.z + b2 * p2.z;
      int idx = y * fb->w + x;
      if (z >= fb->z[idx]) {
        continue;
      }
      fb->z[idx] = z;

      Varyings in = interpolate(v, b0, b1, b2);
      vec3 color = shader(&in, uniforms);
      FB_DrawPixel(fb, x, y, color);
    }
  }
}
