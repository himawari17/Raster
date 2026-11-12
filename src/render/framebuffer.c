#include "framebuffer.h"
#include "../image_utils.h"
#include <stdlib.h>

static float linear_depth(float z_ndc, float n, float f) {
  return 2.0f * f * n / ((f + n) - z_ndc * (f - n));
}
FrameBuffer FB_Create(int w, int h) {
  FrameBuffer fb;
  fb.w = w;
  fb.h = h;
  fb.c = calloc(3 * w * h, sizeof(float));
  fb.z = malloc(sizeof(float) * w * h);
  return fb;
};

void FB_Clear(FrameBuffer* fb, vec3 c) {
  for (int y = 0; y < fb->h; y++) {
    for (int x = 0; x < fb->w; x++) {
      FB_DrawPixel(fb, x, y, c);
    }
  }
  for (size_t i = 0; i < fb->h * fb->w; i++) {
    fb->z[i] = FB_DEPTH_CLEAR;
  }
}

void FB_DrawPixel(FrameBuffer* fb, int x, int y, vec3 c) {
  float* pixel = fb->c + 3 * (y * fb->w + x);
  pixel[0] = c.x;
  pixel[1] = c.y;
  pixel[2] = c.z;
}

int FB_SavePng(FrameBuffer* fb, const char* out_name, float gamma) {
  return save_image_f32_png_rgb(fb->c, out_name, fb->w, fb->h, 3, gamma);
}

void FB_Free(FrameBuffer* fb) {
  free(fb->c);
  free(fb->z);
  fb->c = NULL;
  fb->z = NULL;
}

int FB_SaveDepth(FrameBuffer* fb, const char* out, float f, float n) {
  const int N = fb->w * fb->h;
  float dmin = INFINITY, dmax = -INFINITY;
  for (int i = 0; i < N; i++) {
    if (fb->z[i] >= FB_DEPTH_CLEAR) {
      continue;
    }
    float d = linear_depth(fb->z[i], n, f);
    if (d < dmin)
      dmin = d;
    if (d > dmax)
      dmax = d;
  }
  float range = dmax - dmin;
  if (range < 1e-12f) {
    range = 1.0f;
  }

  float* img = malloc(sizeof(float) * 3 * N);
  for (int i = 0; i < N; i++) {
    float v = 0.0f;
    if (fb->z[i] < FB_DEPTH_CLEAR) {
      float t = (linear_depth(fb->z[i], n, f) - dmin) / range;
      v = 0.15f + 0.85f * t;
    }
    img[3 * i + 0] = img[3 * i + 1] = img[3 * i + 2] = v;
  }
  int ok = save_image_f32_png_rgb(img, out, fb->w, fb->h, 3, 1.0f);
  free(img);
  return ok;
}
