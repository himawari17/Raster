#ifndef _CONTEXT_H
#define _CONTEXT_H

#include "../math/mat.h"
#include "../mesh_utils.h"
#include "framebuffer.h"

typedef struct {
  FrameBuffer* fb;
  mat4 view;
  mat4 proj;
  mat4 vp;
  float near;
  vec4* clip;
  int clip_cap;
} RContext;

int ContextInit(RContext* rc, FrameBuffer* fb);
void ContextSetCamera(RContext* rc, mat4 view, mat4 proj, float near);
void ContextDrawMesh(RContext* rc, const mesh* mesh, mat4 model);
int ContextFree(RContext* rc);

#endif
