#ifndef _RASTER_H
#define _RASTER_H

#include "framebuffer.h"
#include "shader_utils.h"

/* Draws a triangle with p0,p1,p2 coords and c0,c1,c2 colors */
void draw_triangle(FrameBuffer* fb, vec3 p0, vec3 p1, vec3 p2,
                   vec3 c0, vec3 c1, vec3 c2);
void draw_triangle_shaded(FrameBuffer* fb, vec3 p0, vec3 p1, vec3 p2,
                          const Varyings v[3],
                          FragShader shader, const void* uniforms);
#endif
