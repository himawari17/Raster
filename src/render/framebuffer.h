#ifndef _FRAMEBUFFER_H
#define _FRAMEBUFFER_H

#include "../math/vectors.h"
#define FB_DEPTH_CLEAR 1.0f

/* CPU FrameBuffer. Saves pixels in RAM instead of GPU */
typedef struct {
  int w;
  int h;
  float* c;
  float* z;
} FrameBuffer;

/* Create FrameBuffer of w * h size */
FrameBuffer FB_Create(int w, int h);

/* Every frame function. Clears buffer. Sets color for all pixels in buffer */
void FB_Clear(FrameBuffer* fb, vec3 c);

/* Sets color for a specific pixel */
void FB_DrawPixel(FrameBuffer* fb, int x, int y, vec3 c);

/* Saves current FrameBuffer state in a PNG file */
int FB_SavePng(FrameBuffer* fb, const char* out_name);

/* Save depth-buffer footprint */
int FB_SaveDepth(FrameBuffer* fb, const char* out, float f, float n);

void FB_Free(FrameBuffer* fb);
#endif
