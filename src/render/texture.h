#ifndef _TEXTURE_H
#define _TEXTURE_H
#include "../math/vectors.h"

typedef enum {
  WRAP_CLAMP,
  WRAP_REPEAT,
} TextureWrap;

typedef enum {
  FILTER_NEAREST,
  FILTER_BILINEAR,
} TextureFilter;

typedef struct {
  int w;
  int h;
  float* data;
  TextureWrap wrap;
  TextureFilter filter;
} Texture;

Texture* TextureLoad(const char* path, float gamma);
void FreeTexture(Texture* texture);
vec3 TextureSample(const Texture* texture, vec2 uv);

#endif
