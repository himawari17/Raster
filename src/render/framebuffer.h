#ifndef _FRAMEBUFFER_H
#define _FRAMEBUFFER_H

#include "../math/vectors.h"

/* CPU FrameBuffer. Saves pixels in RAM instead of GPU */
typedef struct {
  int w;
  int h;
  float* c;
} FrameBuffer;

/* Create FrameBuffer of w * h size */
FrameBuffer FB_Create(int w, int h);

/* Every frame function. Clears buffer. Sets color for all pixels in buffer */
void FB_Clear(FrameBuffer* fb, vec3 c);

/* Sets color for a specific pixel */
void FB_DrawPixel(FrameBuffer* fb, int x, int y, vec3 c);

/* Saves current FrameBuffer state in a PNG file */
int FB_SavePng(FrameBuffer* fb, const char* out_name);

void FB_Free(FrameBuffer* fb);
#endif
