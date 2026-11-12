#ifndef _MAT_H
#define _MAT_H

/* mat3 and mat4 functions*/

#include "vectors.h"

typedef struct {
  float m[3][3];
} mat3;

typedef struct {
  float m[4][4];
} mat4;

mat4 mat4_make(void);
mat4 mat4_mul(mat4 a, mat4 b);
vec4 mat4_mul_vec4(mat4 m, vec4 v);
mat4 mat4_transpose(mat4 a);
/* trys to invert mat A. inverted mat is OUT. returs 1 if A invertable
 * gauss jordan method*/
int mat4_inverse(mat4 a, mat4* out);

mat3 mat3_from_mat4(mat4 a);
vec3 mat3_mul_vec3(mat3 m, vec3 v);

/* model mats */
mat4 mat4_make_translate(vec3 t);
mat4 mat4_make_scale(vec3 s);

/* creates rotate mat!!! */
mat4 mat4_rotate_x(float a);
/* creates rotate mat!!! */
mat4 mat4_rotate_y(float a);
/* creates rotate mat!!! */
mat4 mat4_rotate_z(float a);

mat4 look_at(vec3 eye, vec3 target, vec3 up);
mat4 perspective(float fovy, float aspect, float near, float far);

/* Create normal matrix */
mat3 mat3_normal_matrix(mat4 model);

#endif
