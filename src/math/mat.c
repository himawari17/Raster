#include "mat.h"
#include <stdlib.h>

mat4 mat4_make(void) {
  mat4 m = {0};
  for (size_t i = 0; i < 4; i++) {
    m.m[i][i] = 1.0f;
  }

  /* 1 0 0 0
   * 0 1 0 0
   * 0 0 1 0
   * 0 0 0 1 */
  return m;
}

mat4 mat4_mul(mat4 a, mat4 b) {
  mat4 out;
  for (size_t i = 0; i < 4; i++) {
    for (size_t j = 0; j < 4; j++) {
      float sum = 0.0f;
      for (size_t k = 0; k < 4; k++) {
        sum += a.m[i][k] * b.m[k][j];
      }
      out.m[i][j] = sum;
    }
  }
  return out;
}

vec4 mat4_mul_vec4(mat4 m, vec4 v) {
  vec4 out;
  for (size_t i = 0; i < 4; i++) {
    out.M[i] = m.m[i][0] * v.x + m.m[i][1] * v.y +
               m.m[i][2] * v.z + m.m[i][3] * v.w;
  }
  return out;
}

mat4 mat4_transpose(mat4 a) {
  mat4 out;
  for (size_t i = 0; i < 4; i++) {
    for (size_t j = 0; j < 4; j++) {
      out.m[i][j] = a.m[j][i];
    }
  }
  return out;
}

int mat4_inverse(mat4 a, mat4* out) {
  float aug[4][8]; /* [A | I] */
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++) {
      aug[i][j] = a.m[i][j];
      aug[i][j + 4] = (i == j) ? 1.0f : 0.0f;
    }

  for (int c = 0; c < 4; c++) {
    int piv = c;
    float best = fabsf(aug[c][c]);
    for (int r = c + 1; r < 4; r++)
      if (fabsf(aug[r][c]) > best) {
        best = fabsf(aug[r][c]);
        piv = r;
      }
    if (best < 1e-8f)
      return 0;
    if (piv != c)
      for (int j = 0; j < 8; j++) {
        float t = aug[c][j];
        aug[c][j] = aug[piv][j];
        aug[piv][j] = t;
      }

    float inv = 1.0f / aug[c][c];
    for (int j = 0; j < 8; j++)
      aug[c][j] *= inv;

    for (int r = 0; r < 4; r++) {
      if (r == c)
        continue;
      float f = aug[r][c];
      if (f == 0.0f)
        continue;
      for (int j = 0; j < 8; j++)
        aug[r][j] -= f * aug[c][j];
    }
  }

  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++)
      out->m[i][j] = aug[i][j + 4];
  return 1;
}

mat3 mat3_from_mat4(mat4 a) {
  mat3 out;
  for (size_t i = 0; i < 3; i++) {
    for (size_t j = 0; j < 3; j++) {
      out.m[i][j] = a.m[i][j];
    }
  }
  return out;
}
vec3 mat3_mul_vec3(mat3 m, vec3 v) {
  return make3(m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z,
               m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z,
               m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z);
}

mat4 mat4_make_translate(vec3 t) {
  mat4 out = mat4_make();
  out.m[0][3] = t.x;
  out.m[1][3] = t.y;
  out.m[2][3] = t.z;
  return out;
}
mat4 mat4_make_scale(vec3 s) {
  mat4 out = mat4_make();
  out.m[0][0] = s.x;
  out.m[1][1] = s.y;
  out.m[2][2] = s.z;
  return out;
}

mat4 mat4_rotate_x(float a) {
  mat4 out = mat4_make();
  float c = cosf(a);
  float s = sinf(a);
  out.m[1][1] = c;
  out.m[1][2] = -s;
  out.m[2][1] = s;
  out.m[2][2] = c;
  return out;
}
mat4 mat4_rotate_y(float a) {
  mat4 out = mat4_make();
  float c = cosf(a);
  float s = sinf(a);
  out.m[0][0] = c;
  out.m[0][2] = s;
  out.m[2][0] = -s;
  out.m[2][2] = c;
  return out;
}
mat4 mat4_rotate_z(float a) {
  mat4 out = mat4_make();
  float c = cosf(a);
  float s = sinf(a);
  out.m[0][0] = c;
  out.m[0][1] = -s;
  out.m[1][0] = s;
  out.m[1][1] = c;
  return out;
}

mat4 look_at(vec3 eye, vec3 target, vec3 up) {
  vec3 zc = normalize3(sub3(eye, target));
  vec3 xc = cross3(up, zc);

  if (length3(xc) < 1e-6f) {
    vec3 alt = fabsf(zc.y) < 0.9f ? make3(0, 1, 0) : make3(1, 0, 0);
    xc = cross3(alt, zc);
  }

  xc = normalize3(xc);
  vec3 yc = cross3(zc, xc);
  mat4 v = mat4_make();

  v.m[0][0] = xc.x;
  v.m[0][1] = xc.y;
  v.m[0][2] = xc.z;
  v.m[0][3] = -dot3(xc, eye);
  v.m[1][0] = yc.x;
  v.m[1][1] = yc.y;
  v.m[1][2] = yc.z;
  v.m[1][3] = -dot3(yc, eye);
  v.m[2][0] = zc.x;
  v.m[2][1] = zc.y;
  v.m[2][2] = zc.z;
  v.m[2][3] = -dot3(zc, eye);
  return v;
}
mat4 perspective(float fovy, float aspect, float near, float far) {
  float c = 1.0f / tanf(fovy * 0.5f);
  mat4 out = {0};
  out.m[0][0] = c / aspect;
  out.m[1][1] = c;
  out.m[2][2] = (far + near) / (near - far);
  out.m[2][3] = 2.0f * far * near / (near - far);
  out.m[3][2] = -1.0f;
  return out;
}

mat3 mat3_normal_matrix(mat4 model) {
  mat4 inv;
  if (!mat4_inverse(model, &inv)) {
    return mat3_from_mat4(model);
  }
  return mat3_from_mat4(mat4_transpose(inv));
}
