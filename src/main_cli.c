// Пример работы с утилитами шаблона без окна:
// загружает меш, печатает его размеры, загружает текстуру и сохраняет её
// с переставленными местами красным и синим каналами.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "math/math_utils.h"
#include "render/context.h"
#include "render/framebuffer.h"
#include "render/raster.h"
#include "render/shader.h"

static void render(RContext* rc, const mesh* m, mat4 model,
                   FragShader shader, const void* uniforms) {
  FB_Clear(rc->fb, make3(0.0f, 0.0f, 0.0f));
  ContextUseShader(rc, shader, uniforms);
  ContextDrawMesh(rc, m, model);
}

static int report(int ok, const char* name) {
  if (ok)
    printf("saved %s\n", name);
  else
    fprintf(stderr, "failed to save %s\n", name);
  return ok;
}
int main(int argc, char** argv) {

  const int W = 800, H = 600;

  const char* mesh_path = argc > 1 ? argv[1] : "resources/Bunny.obj";
  int ok = 1;
  /* default colors */
  const vec3 red = make3(1, 0, 0);
  const vec3 green = make3(0, 1, 0);
  const vec3 blue = make3(0, 0, 1);

  /* Task1 */
  FrameBuffer fb = FB_Create(W, H);
  FB_Clear(&fb, make3(0.0f, 0.0f, 0.0f));
  draw_triangle(&fb, make3(100, 100, 0), make3(300, 500, 0), make3(500, 100, 0),
                red, green, blue);
  ok &= report(FB_SavePng(&fb, "triangle.png", 1.0f), "triangle.png");

  /* task 2-5 */
  mesh mesh = load_obj(mesh_path);
  if (mesh.num_triangles == 0) {
    fprintf(stderr, "FAILED TO LOAD MESH %s\n", mesh_path);
    FB_Free(&fb);
    return 1;
  }
  if (!mesh.normals) {
    printf("no normals in file -> computing\n");
    compute_normals(&mesh);
  }
  printf("%s: %d vertices, %d triangles\n", mesh_path, mesh.num_vertices,
         mesh.num_triangles);

  //
  // Setup model
  //
  const float near = 0.01f;
  const float far = 10.0f;
  const vec3 up = make3(0.0f, 1.0f, 0.0f);
  /* look from eye to target*/
  const vec3 camera_position = make3(0.0f, 0.3f, 1.0f);
  /*         					letf/right  up    down*/
  const vec3 camera_rotation = make3(0.0f, 0.0f, 0.0f);
  /* Camera view */
  mat4 view = look_at(camera_position, // eye
                      camera_rotation, // target
                      up);             // up

  float fov = deg2rad(60.0f);

  mat4 proj = perspective(fov, (float)W / (float)H, near, far);

  const vec3 model_position = make3(0.0f, 0.0f, 0.0f);
  const float model_horizontal_angle = 0.0f; // yaw
  const float model_vertical_angle = 0.0f;   // pitch

  const mat4 rotation = mat4_mul(mat4_rotate_x(deg2rad(model_vertical_angle)),
                                 mat4_rotate_y(deg2rad(model_horizontal_angle)));

  mat4 user = mat4_mul(mat4_make_translate(model_position),
                       mat4_mul(rotation, mat4_make_scale(make3(1, 1, 1))));
  mat4 model = mat4_mul(user, normalize_mesh(&mesh));
  RContext context;
  ContextInit(&context, &fb);
  ContextSetCamera(&context, view, proj, near);
  render(&context, &mesh, model, NULL, NULL);

  /* Task 2 */
  ok &= report(FB_SavePng(&fb, "projection.png", 1.0f), "projection.png");

  /* Task 3 */
  ok &= report(FB_SaveDepth(&fb, "depth.png", far, near), "depth.png");

  /* Task 4 Normals*/
  render(&context, &mesh, model, fs_normals, NULL);
  ok &= report(FB_SavePng(&fb, "normals.png", 1.0f), "normals.png");

  LambertUniforms lambert = lambert_uniforms(make3(-1.0f, -2.0f, -2.0f), /* sun */
                                             make3(1.0f, 1.0f, 1.0f),    /* albedo */
                                             0.1f, 0.9f);                /* ka, kd */

  /* Task 4 Lambert */
  render(&context, &mesh, model, fs_lambert, &lambert);
  ok &= report(FB_SavePng(&fb, "lambert.png", 2.2f), "lambert.png");

  ContextFree(&context);
  FB_Free(&fb);
  free_mesh(&mesh);
  return ok ? 0 : 1;
}
