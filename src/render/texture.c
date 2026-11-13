#include "texture.h"
#include "../image_utils.h"
#include <stdlib.h>

/* Brings index i into [0, n-1] */
static inline int wrap_index(int i, int n, TextureWrap mode) {
  if (mode == WRAP_CLAMP) {
    if (i < 0) {
      return 0;
    } else if (i >= n) {
      return n - 1;
    } else {
      return i;
    }
  }
  int r = i % n;
  return r < 0 ? r + n : r;
}

static inline vec3 texel(const Texture* t, int i, int j) {
  i = wrap_index(i, t->w, t->wrap);
  j = wrap_index(j, t->h, t->wrap);
  const float* p = t->data + 3 * (j * t->w + i);
  return make3(p[0], p[1], p[2]);
}

static vec3 filter_nearest(const Texture* t, float x, float y) {
  return texel(t, (int)floorf(x + 0.5f), (int)floorf(y + 0.5f));
}

static vec3 filter_bilinear(const Texture* t, float x, float y) {
  float fx = floorf(x), fy = floorf(y);
  int i = (int)fx, j = (int)fy;
  float a = x - fx;
  float b = y - fy;

  vec3 c00 = texel(t, i, j);
  vec3 c10 = texel(t, i + 1, j);
  vec3 c01 = texel(t, i, j + 1);
  vec3 c11 = texel(t, i + 1, j + 1);
  vec3 bottom = add3(scale3(c00, 1.0f - a), scale3(c10, a));
  vec3 top = add3(scale3(c01, 1.0f - a), scale3(c11, a));
  return add3(scale3(bottom, 1.0f - b), scale3(top, b));
}

Texture* TextureLoad(const char* path, float gamma) {
  int w = 0;
  int h = 0;
  float* raw_data = load_image_f32_rgb(path, &w, &h, gamma);
  if (!raw_data) {
    return NULL;
  }
  Texture* texture = malloc(sizeof(Texture));
  if (!texture) {
    free(raw_data);
    return NULL;
  }
  texture->w = w;
  texture->h = h;
  texture->data = raw_data;
  texture->filter = FILTER_BILINEAR;
  texture->wrap = WRAP_REPEAT;
  return texture;
}
void FreeTexture(Texture* texture) {
  if (!texture) {
    return;
  }
  free(texture->data);
  free(texture);
}
vec3 TextureSample(const Texture* texture, vec2 uv) {
  float x = uv.x * texture->w - 0.5f;
  float y = uv.y * texture->h - 0.5f;
  if (texture->filter == FILTER_BILINEAR) {
    return filter_bilinear(texture, x, y);
  }
  return filter_nearest(texture, x, y);
}
