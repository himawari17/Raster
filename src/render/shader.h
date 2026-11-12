#ifndef _SHADER_H
#define _SHADER_H

#include "shader_utils.h"

typedef struct {
  vec3 light;
  vec3 albedo;
  float ka;
  float kd;
} LambertUniforms;

vec3 fs_lambert(const Varyings* in, const void* uniforms);
vec3 fs_normals(const Varyings* in, const void* uniforms);

LambertUniforms lambert_uniforms(vec3 sun_dir, vec3 albedo, float ka, float kd);
#endif
