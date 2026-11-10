#include "context.h"
#include "raster.h"
#include <stdlib.h>

static void ensure_clip_capacity(RContext* rc, int n) {
  if (n <= rc->clip_cap)
    return;
  free(rc->clip);
  rc->clip = malloc(sizeof(vec4) * n);
  rc->clip_cap = n;
}
static vec3 debug_color(int t) {
  unsigned h = (unsigned)t * 2654435761u;
  return make3(((h >> 0) & 255) / 255.0f,
               ((h >> 8) & 255) / 255.0f,
               ((h >> 16) & 255) / 255.0f);
}

int ContextInit(RContext* rc, FrameBuffer* fb) {
  if (!fb) {
    return EXIT_FAILURE;
  }
  rc->fb = fb;
  rc->view = mat4_make();
  rc->proj = mat4_make();
  rc->vp = mat4_make();
  rc->clip = NULL;
  rc->clip_cap = 0;
  return EXIT_SUCCESS;
}

void ContextSetCamera(RContext* rc, mat4 view, mat4 proj, float near) {
  rc->view = view;
  rc->proj = proj;
  rc->vp = mat4_mul(proj, view);
  rc->near = near;
}

void ContextDrawMesh(RContext* rc, const mesh* mesh, mat4 model) {
  float W = rc->fb->w;
  float H = rc->fb->h;
  mat4 mvp = mat4_mul(rc->vp, model);
  ensure_clip_capacity(rc, mesh->num_vertices);
  /* vert shader */
  for (size_t i = 0; i < mesh->num_vertices; i++) {
    rc->clip[i] = mat4_mul_vec4(mvp, point(mesh->verts[i]));
  }

  for (int t = 0; t < mesh->num_triangles; t++) {
    vec4 c[3];
    for (int k = 0; k < 3; k++)
      c[k] = rc->clip[mesh->indices[3 * t + k]];
    if (c[0].w < rc->near || c[1].w < rc->near || c[2].w < rc->near)
      continue;
    vec2 s[3];
    float z_ndc[3], inv_w[3];
    for (int k = 0; k < 3; k++) {
      float iw = 1.0f / c[k].w;
      s[k].x = (c[k].x * iw + 1.0f) * 0.5f * W;
      s[k].y = (c[k].y * iw + 1.0f) * 0.5f * H;
      z_ndc[k] = c[k].z * iw;
      inv_w[k] = iw;
    }
    (void)z_ndc;
    (void)inv_w;
    /* frag? */
    vec3 col = debug_color(t);
    draw_triangle(rc->fb, s[0], s[1], s[2], col, col, col);
  }
}
int ContextFree(RContext* rc) {
  free(rc->clip);
  rc->clip = NULL;
  rc->clip_cap = 0;
  return EXIT_SUCCESS;
}
