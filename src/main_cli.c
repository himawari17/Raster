// Пример работы с утилитами шаблона без окна:
// загружает меш, печатает его размеры, загружает текстуру и сохраняет её
// с переставленными местами красным и синим каналами.

#include "render/context.h"
#include "render/framebuffer.h"
#include "render/raster.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const float PI = 3.14159265f;
static float deg2rad(float d) { return d * PI / 180.0f; }

int main(int argc, char** argv) {
  const int W = 800, H = 600;
  const char* mode = argc > 3 ? argv[3] : "color";
  FrameBuffer fb = FB_Create(W, H);
  FB_Clear(&fb, make3(0, 0, 0));
  const vec3 red = make3(1, 0, 0), green = make3(0, 1, 0),
             blue = make3(0, 0, 1), white = make3(1, 1, 1);
  draw_triangle(&fb, make3(100, 100, 0), make3(300, 500, 0), make3(500, 100, 0),
                red, green, blue);
  if (!FB_SavePng(&fb, "triangles.png")) {
    printf("failed to save\n");
    return 1;
  }
  printf("saved triangles.png\n");
  FB_Free(&fb);

  /* task 2 */
  const char* mesh_path = argc > 1 ? argv[1] : "resources/Bunny.obj";
  const char* out_path = argc > 2 ? argv[2] : "projection.png";
  mesh mesh = load_obj(mesh_path);
  if (mesh.num_triangles == 0) {
    fprintf(stderr, "FAILED TO LAOD MESH %s\n", mesh_path);
    return 1;
  }
  printf("%s: %d vertices, %d triangles\n", mesh_path, mesh.num_vertices, mesh.num_triangles);

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
  mat4 proj = perspective(deg2rad(60.0f), (float)W / (float)H, near, far);
  const vec3 model_position = make3(0.0f, 0.0f, 0.0f);
  /* Horizontal rotation */
  const float model_horizontal_angle = 0.0f; // yaw
  const float model_vertical_angle = 0.0f;   // pitch
  const mat4 rotation = mat4_mul(mat4_rotate_x(deg2rad(model_vertical_angle)),
                                 mat4_rotate_y(deg2rad(model_horizontal_angle)));
  mat4 user = mat4_mul(mat4_make_translate(model_position),
                       mat4_mul(rotation,
                                mat4_make_scale(make3(1, 1, 1))));
  mat4 model = mat4_mul(user, normalize_mesh(&mesh));

  fb = FB_Create(W, H);
  FB_Clear(&fb, make3(0.1f, 0.1f, 0.1f));
  RContext context;
  ContextInit(&context, &fb);
  ContextSetCamera(&context, view, proj, near);
  ContextDrawMesh(&context, &mesh, model);
  int ok = strcmp(mode, "depth") == 0
               ? FB_SaveDepth(&fb, out_path, far, near)
               : FB_SavePng(&fb, out_path);
  if (ok)
    printf("saved %s (%dx%d)\n", out_path, W, H);
  else
    fprintf(stderr, "failed to save %s\n", out_path);
  ContextFree(&context);
  FB_Free(&fb);
  free_mesh(&mesh);

  return ok ? 0 : 1;
}
