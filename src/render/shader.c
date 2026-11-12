#include "shader.h"

LambertUniforms lambert_uniforms(vec3 sun_dir, vec3 albedo, float ka, float kd) {
  LambertUniforms u;
  u.light = normalize3(scale3(sun_dir, -1.0f));
  u.albedo = albedo;
  u.ka = ka;
  u.kd = kd;
  return u;
}

vec3 fs_lambert(const Varyings* in, const void* uniforms) {
  const LambertUniforms* u = uniforms;
  vec3 n = normalize3(in->normals);
  float ndotl = fmaxf(0.0f, dot3(n, u->light));
  return scale3(u->albedo, u->ka + u->kd * ndotl);
}

vec3 fs_normals(const Varyings* in, const void* uniforms) {
  (void)uniforms;
  vec3 n = normalize3(in->normals);
  return make3(n.x * 0.5f + 0.5f, n.y * 0.5f + 0.5f, n.z * 0.5f + 0.5f);
}
