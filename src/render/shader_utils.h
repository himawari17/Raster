#ifndef _SHADER_UTILS_H
#define _SHADER_UTILS_H

#include "../math/vectors.h"

typedef struct {
  vec3 normals;
  vec3 color;
  vec2 uv;
} Varyings;

/* Shader function pointer template*/
typedef vec3 (*FragShader)(const Varyings* in, const void* uniforms);

#endif
