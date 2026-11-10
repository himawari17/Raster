// testing code build by AI
//
//
#include "math/mat.h"
#include <stdio.h>

static int fails = 0;
static const float PI = 3.14159265f;

static void check(int ok, const char* what) {
  printf("[%s] %s\n", ok ? " OK " : "FAIL", what);
  if (!ok)
    fails++;
}

static int feq(float a, float b) { return fabsf(a - b) < 1e-4f; }

static int v3eq(vec3 a, vec3 b) {
  return feq(a.x, b.x) && feq(a.y, b.y) && feq(a.z, b.z);
}

static int v4eq(vec4 a, vec4 b) {
  return feq(a.x, b.x) && feq(a.y, b.y) && feq(a.z, b.z) && feq(a.w, b.w);
}

static int is_identity(mat4 a) {
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++)
      if (!feq(a.m[i][j], i == j ? 1.0f : 0.0f))
        return 0;
  return 1;
}

static void test_vectors(void) {
  vec3 x = make3(1, 0, 0), y = make3(0, 1, 0), z = make3(0, 0, 1);
  check(v3eq(cross3(x, y), z), "x * y = z");
  check(v3eq(cross3(y, z), x), "y * z = x");
  check(v3eq(cross3(z, x), y), "z * x = y");

  vec3 a = make3(1, 2, 3), b = make3(-4, 0.5f, 2);
  vec3 c = cross3(a, b);
  check(feq(dot3(c, a), 0) && feq(dot3(c, b), 0), "cross перпендикулярен обоим");

  check(feq(length3(normalize3(a)), 1.0f), "normalize даёт длину 1");
  check(v3eq(normalize3(make3(0, 0, 0)), make3(0, 0, 0)), "normalize(0) = 0");
}

static void test_matrices(void) {
  /* умножение на вектор: точка и направление */
  mat4 T = mat4_make_translate(make3(5, 6, 7));
  check(v4eq(mat4_mul_vec4(T, point(make3(1, 2, 3))), make4(6, 8, 10, 1)),
        "T сдвигает точку");
  check(v4eq(mat4_mul_vec4(T, dir(make3(1, 2, 3))), make4(1, 2, 3, 0)),
        "T не трогает направление");

  /* повороты */
  check(v4eq(mat4_mul_vec4(mat4_rotate_y(PI / 2), dir(make3(0, 0, 1))),
             make4(1, 0, 0, 0)),
        "Ry(90): z -> x");
  check(v4eq(mat4_mul_vec4(mat4_rotate_z(PI / 2), dir(make3(1, 0, 0))),
             make4(0, 1, 0, 0)),
        "Rz(90): x -> y");
  check(v4eq(mat4_mul_vec4(mat4_rotate_x(PI / 2), dir(make3(0, 1, 0))),
             make4(0, 0, 1, 0)),
        "Rx(90): y -> z");

  /* порядок умножения: T*S сначала масштабирует, потом сдвигает */
  mat4 TS = mat4_mul(T, mat4_make_scale(make3(2, 2, 2)));
  check(v4eq(mat4_mul_vec4(TS, point(make3(1, 1, 1))), make4(7, 8, 9, 1)),
        "T*S: сначала S, потом T");

  /* обращение */
  mat4 M = mat4_mul(T, mat4_mul(mat4_rotate_y(0.7f),
                                mat4_mul(mat4_rotate_x(-1.1f), mat4_make_scale(make3(2, 0.5f, 3)))));
  mat4 Mi;
  check(mat4_inverse(M, &Mi), "TRS обратима");
  check(is_identity(mat4_mul(M, Mi)), "M * M^-1 = I");
  mat4 zero = {0};
  check(!mat4_inverse(zero, &Mi), "нулевая матрица необратима");

  /* транспонирование */
  check(feq(mat4_transpose(T).m[3][0], 5), "transpose переносит столбец в строку");
}

static void test_camera(void) {
  vec3 eye = make3(2, 1.5f, 3), target = make3(0, 0.2f, 0);
  mat4 V = look_at(eye, target, make3(0, 1, 0));

  check(v4eq(mat4_mul_vec4(V, point(eye)), make4(0, 0, 0, 1)), "V * eye = 0");
  float d = length3(sub3(target, eye));
  check(v4eq(mat4_mul_vec4(V, point(target)), make4(0, 0, -d, 1)),
        "V * target лежит на оси -z");

  mat4 Vi;
  mat4_inverse(V, &Vi);
  check(v4eq(mat4_mul_vec4(Vi, make4(0, 0, 0, 1)), point(eye)),
        "V^-1 возвращает камеру в eye");

  /* строки V - оси камеры, они должны быть ортонормированы */
  vec3 xc = make3(V.m[0][0], V.m[0][1], V.m[0][2]);
  vec3 yc = make3(V.m[1][0], V.m[1][1], V.m[1][2]);
  vec3 zc = make3(V.m[2][0], V.m[2][1], V.m[2][2]);
  check(feq(dot3(xc, yc), 0) && feq(dot3(yc, zc), 0) && feq(dot3(xc, zc), 0),
        "оси камеры перпендикулярны");
  check(feq(length3(xc), 1) && feq(length3(yc), 1) && feq(length3(zc), 1),
        "оси камеры единичные");

  mat4 Vdown = look_at(make3(0, 5, 0), make3(0, 0, 0), make3(0, 1, 0));
  check(!isnan(Vdown.m[0][0]), "look_at строго вниз без NaN");
}

static void test_projection(void) {
  const float n = 0.01f, f = 10.0f;
  mat4 P = perspective(60.0f * PI / 180.0f, 800.0f / 600.0f, n, f);

  vec4 cn = mat4_mul_vec4(P, make4(0, 0, -n, 1));
  vec4 cf = mat4_mul_vec4(P, make4(0, 0, -f, 1));
  check(feq(cn.z / cn.w, -1), "near -> z_ndc = -1");
  check(feq(cf.z / cf.w, 1), "far  -> z_ndc = +1");
  check(feq(cn.w, n), "w_clip = расстояние до точки");

  /* полный конвейер: камера в (0,0,3), точка (1,0,0) -> пиксель (573.2, 300) */
  mat4 V = look_at(make3(0, 0, 3), make3(0, 0, 0), make3(0, 1, 0));
  vec4 c = mat4_mul_vec4(mat4_mul(P, V), point(make3(1, 0, 0)));
  float xs = (c.x / c.w + 1) * 0.5f * 800;
  float ys = (c.y / c.w + 1) * 0.5f * 600;
  printf("       (1,0,0) -> (%.1f, %.1f)\n", xs, ys);
  check(fabsf(xs - 573.2f) < 0.1f && fabsf(ys - 300) < 0.1f, "точка попала в (573.2, 300)");

  vec4 o = mat4_mul_vec4(mat4_mul(P, V), point(make3(0, 0, 0)));
  check(feq((o.x / o.w + 1) * 0.5f * 800, 400) && feq((o.y / o.w + 1) * 0.5f * 600, 300),
        "точка на оси взгляда -> центр кадра");
}

int main(void) {
  test_vectors();
  test_matrices();
  test_camera();
  test_projection();
  printf(fails ? "\n%d FAILED\n" : "\nвсе тесты прошли\n", fails);
  return fails ? 1 : 0;
}
