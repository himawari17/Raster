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

static void ensure_norm_capacity(RContext* rc, int n) {
  if (n <= rc->norm_cap) {
    return;
  }
  free(rc->norms);
  rc->norms = malloc(sizeof(vec3) * n);
  rc->norm_cap = n;
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
  rc->clip_back = 1;
  rc->norms = NULL;
  rc->norm_cap = 0;
  rc->uniforms = NULL;
  rc->shader = NULL;
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
  ensure_norm_capacity(rc, mesh->num_vertices);
  mat3 nmat = mat3_normal_matrix(model);
  /* Vertex shader */
  for (size_t i = 0; i < mesh->num_vertices; i++) {
    rc->clip[i] = mat4_mul_vec4(mvp, point(mesh->verts[i]));
    if (mesh->normals) {
      rc->norms[i] = normalize3(mat3_mul_vec3(nmat, mesh->normals[i]));
    }
  }

  for (int t = 0; t < mesh->num_triangles; t++) {
    vec4 c[3];
    for (int k = 0; k < 3; k++)
      c[k] = rc->clip[mesh->indices[3 * t + k]];
    if (c[0].w < rc->near || c[1].w < rc->near || c[2].w < rc->near)
      continue;
    vec3 s[3];
    float z_ndc[3], inv_w[3];
    for (int k = 0; k < 3; k++) {
      float iw = 1.0f / c[k].w;
      s[k].x = (c[k].x * iw + 1.0f) * 0.5f * W;
      s[k].y = (c[k].y * iw + 1.0f) * 0.5f * H;
      z_ndc[k] = c[k].z * iw;
      s[k].z = z_ndc[k];
      inv_w[k] = iw;
    }
    inv_w[0] = inv_w[1] = inv_w[2] = 1.0f;

    /* back face culling */
    float area = (s[1].x - s[0].x) * (s[2].y - s[0].y) -
                 (s[1].y - s[0].y) * (s[2].x - s[0].x);
    if (rc->clip_back && area <= 0.0f)
      continue;

    int use_shader = rc->shader != NULL && mesh->normals != NULL;
    if (use_shader) {
      Varyings v[3] = {0};
      for (int k = 0; k < 3; k++) {
        int idx = mesh->indices[3 * t + k];
        v[k].normals = rc->norms[mesh->indices[3 * t + k]];
        if (mesh->tcs) {
          v[k].uv = mesh->tcs[idx];
        } else {
          v[k].uv = make2(0.0f, 0.0f);
        }
      }
      draw_triangle_shaded(rc->fb, s[0], s[1], s[2], inv_w, v, rc->shader, rc->uniforms);
    } else {
      vec3 col = debug_color(t);
      draw_triangle(rc->fb, s[0], s[1], s[2], col, col, col);
    }
  }
}
int ContextFree(RContext* rc) {
  free(rc->clip);
  free(rc->norms);
  rc->clip = NULL;
  rc->clip_cap = 0;
  rc->norms = NULL;
  rc->norm_cap = 0;
  return EXIT_SUCCESS;
}

void ContextUseShader(RContext* rc, FragShader shader, const void* uniforms) {
  rc->shader = shader;
  rc->uniforms = uniforms;
}
