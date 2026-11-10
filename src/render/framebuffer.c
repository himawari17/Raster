#include "framebuffer.h"
#include "../image_utils.h"
#include <stdlib.h>

FrameBuffer FB_Create(int w, int h) {
  FrameBuffer fb;
  fb.w = w;
  fb.h = h;
  fb.c = calloc(3 * w * h, sizeof(float));
  return fb;
};

void FB_Clear(FrameBuffer* fb, vec3 c) {
  for (int x = 0; x < fb->w; x++) {
    for (int y = 0; y < fb->h; y++) {
      FB_DrawPixel(fb, x, y, c);
    }
  }
}

void FB_DrawPixel(FrameBuffer* fb, int x, int y, vec3 c) {
  float* pixel = fb->c + 3 * (y * fb->w + x);
  pixel[0] = c.x;
  pixel[1] = c.y;
  pixel[2] = c.z;
}

int FB_SavePng(FrameBuffer* fb, const char* out_name) {
  return save_image_f32_png_rgb(fb->c, out_name, fb->w, fb->h, 3, 1.0f);
}

void FB_Free(FrameBuffer* fb) {
  free(fb->c);
  fb->c = NULL;
}
