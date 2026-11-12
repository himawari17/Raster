#ifndef VECTORS_H
#define VECTORS_H

#include <math.h>
// Минимальный набор векторных типов, нужный утилитам шаблона.
// Остальную математику (операции над векторами, матрицы) вы пишете сами.

typedef struct
{
  union {
    struct
    {
      float x;
      float y;
    };
    float M[2];
  };
} vec2;

typedef struct
{
  union {
    struct
    {
      float x;
      float y;
      float z;
    };
    float M[3];
  };
} vec3;

typedef struct {
  union {
    struct {
      float x;
      float y;
      float z;
      float w;
    };
    float M[4];
  };
} vec4;

static inline vec2 make2(float x, float y) {
  vec2 v;
  v.x = x;
  v.y = y;
  return v;
}

static inline vec3 make3(float x, float y, float z) {
  vec3 v;
  v.x = x;
  v.y = y;
  v.z = z;
  return v;
}

static inline vec4 make4(float x, float y, float z, float w) {
  vec4 v;
  v.x = x;
  v.y = y;
  v.z = z;
  v.w = w;
  return v;
}

static inline vec2 sub2(vec2 a, vec2 b) {
  return make2(a.x - b.x, a.y - b.y);
}
static inline vec3 sub3(vec3 a, vec3 b) {
  return make3(a.x - b.x, a.y - b.y, a.z - b.z);
}

static inline vec2 add2(vec2 a, vec2 b) {
  return make2(a.x + b.x, a.y + b.y);
}

static inline vec3 add3(vec3 a, vec3 b) {
  return make3(a.x + b.x, a.y + b.y, a.z + b.z);
}

static inline float cross2(vec2 a, vec2 b) {
  return a.x * b.y - a.y * b.x;
}

static inline vec3 cross3(vec3 a, vec3 b) {
  return make3(a.y * b.z - a.z * b.y,
               a.z * b.x - a.x * b.z,
               a.x * b.y - a.y * b.x);
}

static inline vec3 mul3(vec3 a, vec3 b) {
  return make3(a.x * b.x, a.y * b.y, a.z * b.z);
}

static inline vec3 scale3(vec3 a, float c) {
  return make3(a.x * c, a.y * c, a.z * c);
}

static inline float dot3(vec3 a, vec3 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline float length3(vec3 a) {
  return sqrtf(dot3(a, a));
}

static inline vec3 normalize3(vec3 a) {
  float lenght = length3(a);
  if (lenght <= 0.0) {
    return make3(0.0, 0.0, 0.0);
  }
  return scale3(a, 1.0f / lenght);
}

/* vec4 operations */

static inline vec4 point(vec3 a) {
  return make4(a.x, a.y, a.z, 1.0f);
}
static inline vec4 dir(vec3 a) {
  return make4(a.x, a.y, a.z, 0.0f);
}
static inline vec3 tovec3(vec4 a) {
  return make3(a.x, a.y, a.z);
}

#endif
