#include "shader.h"

LambertUniforms lambert_uniforms(vec3 sun_dir, vec3 albedo, float ka, float kd, Texture* texture) {
  LambertUniforms u;
  u.light = normalize3(scale3(sun_dir, -1.0f));
  u.albedo = albedo;
  u.ka = ka;
  u.kd = kd;
  u.texture = texture;
  return u;
}

vec3 fs_lambert(const Varyings* in, const void* uniforms) {
  const LambertUniforms* u = uniforms;
  vec3 n = normalize3(in->normals);
  float ndotl = fmaxf(0.0f, dot3(n, u->light));
  return scale3(u->albedo, u->ka + u->kd * ndotl);
}

vec3 fs_uv(const Varyings* in, const void* uniforms) {
  (void)uniforms;
  return make3(in->uv.x - floorf(in->uv.x), in->uv.y - floorf(in->uv.y), 0.0f);
}
vec3 fs_lambert_tex(const Varyings* in, const void* uniforms) {
  const LambertUniforms* u = uniforms;
  vec3 albedo = u->albedo;
  if (u->texture)
    albedo = mul3(albedo, TextureSample(u->texture, in->uv));

  vec3 n = normalize3(in->normals);
  float ndotl = fmaxf(0.0f, dot3(n, u->light));
  return scale3(albedo, u->ka + u->kd * ndotl);
}

vec3 fs_normals(const Varyings* in, const void* uniforms) {
  (void)uniforms;
  vec3 n = normalize3(in->normals);
  return make3(n.x * 0.5f + 0.5f, n.y * 0.5f + 0.5f, n.z * 0.5f + 0.5f);
}
