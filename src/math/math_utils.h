// Math helper
#ifndef _MATH_UTILS_H
#define _MATH_UTILS_H
#include <math.h>

#define PI 3.14159265f
static inline float min3f(float a, float b, float c) {
  return fminf(a, fminf(b, c));
}

static inline float max3f(float a, float b, float c) {
  return fmaxf(a, fmaxf(b, c));
}

static inline int clampi(int a, int b, int c) {
  return a < b ? b : (a > c ? c : a);
}

static inline float deg2rad(float d) { return d * PI / 180.0f; }
#endif
