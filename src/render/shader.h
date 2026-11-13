#ifndef _SHADER_H
#define _SHADER_H

#include "shader_utils.h"
#include "texture.h"

typedef struct {
  vec3 light;
  vec3 albedo;
  float ka;
  float kd;
  Texture* texture;
} LambertUniforms;

vec3 fs_lambert(const Varyings* in, const void* uniforms);
vec3 fs_normals(const Varyings* in, const void* uniforms);
vec3 fs_lambert_tex(const Varyings* in, const void* uniforms);
vec3 fs_uv(const Varyings* in, const void* uniforms);

LambertUniforms lambert_uniforms(vec3 sun_dir, vec3 albedo, float ka, float kd, Texture* texture);
#endif
