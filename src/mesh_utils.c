#include "mesh_utils.h"

#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include "external/tinyobj_loader_c.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN64
#define atoll(S) _atoi64(S)
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

void free_mesh(mesh* m) {
  free(m->verts);
  free(m->normals);
  free(m->tcs);
  free(m->indices);
}

static char* mmap_file(size_t* len, const char* filename) {
#ifdef _WIN64
  HANDLE file =
      CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                  FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

  if (file == INVALID_HANDLE_VALUE) { /* E.g. Model may not have materials. */
    return NULL;
  }

  HANDLE fileMapping = CreateFileMapping(file, NULL, PAGE_READONLY, 0, 0, NULL);
  assert(fileMapping != INVALID_HANDLE_VALUE);

  LPVOID fileMapView = MapViewOfFile(fileMapping, FILE_MAP_READ, 0, 0, 0);
  char* fileMapViewChar = (char*)fileMapView;
  assert(fileMapView != NULL);

  DWORD file_size = GetFileSize(file, NULL);
  (*len) = (size_t)file_size;

  return fileMapViewChar;
#else

  struct stat sb;
  char* p;
  int fd;

  fd = open(filename, O_RDONLY);
  if (fd == -1) {
    perror("open");
    return NULL;
  }

  if (fstat(fd, &sb) == -1) {
    perror("fstat");
    return NULL;
  }

  if (!S_ISREG(sb.st_mode)) {
    fprintf(stderr, "%s is not a file\n", filename);
    return NULL;
  }

  p = (char*)mmap(0, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);

  if (p == MAP_FAILED) {
    perror("mmap");
    return NULL;
  }

  if (close(fd) == -1) {
    perror("close");
    return NULL;
  }

  (*len) = sb.st_size;

  return p;

#endif
}

static void get_file_data(void* ctx, const char* filename, const int is_mtl,
                          const char* obj_filename, char** data, size_t* len) {
  // NOTE: If you allocate the buffer with malloc(),
  // You can define your own memory management struct and pass it through `ctx`
  // to store the pointer and free memories at clean up stage(when you quit an
  // app)
  // This example uses mmap(), so no free() required.
  (void)ctx;

  if (!filename) {
    fprintf(stderr, "null filename\n");
    (*data) = NULL;
    (*len) = 0;
    return;
  }

  size_t data_len = 0;

  *data = mmap_file(&data_len, filename);
  (*len) = data_len;
}

typedef struct
{
  int v_idx, vt_idx, vn_idx;
  int id; // -1 - empty
} vertex_slot;

static uint32_t hash_vertex(int v_idx, int vt_idx, int vn_idx) {
  uint32_t h = (uint32_t)v_idx * 73856093u;
  h ^= (uint32_t)vt_idx * 19349663u;
  h ^= (uint32_t)vn_idx * 83492791u;
  return h;
}

static int valid_index(int idx, unsigned int count) {
  return idx >= 0 && (unsigned int)idx < count;
}

mesh load_obj(const char* filename) {
  mesh m;
  m.num_triangles = 0;
  m.num_vertices = 0;
  m.verts = NULL;
  m.normals = NULL;
  m.tcs = NULL;
  m.indices = NULL;

  tinyobj_attrib_t attrib;
  tinyobj_shape_t* shapes = NULL;
  size_t num_shapes;
  tinyobj_material_t* materials = NULL;
  size_t num_materials;

  unsigned int flags = TINYOBJ_FLAG_TRIANGULATE;
  int ret =
      tinyobj_parse_obj(&attrib, &shapes, &num_shapes, &materials,
                        &num_materials, filename, get_file_data, NULL, flags);
  if (ret != TINYOBJ_SUCCESS)
    return m;

  const int num_corners = attrib.num_faces;

  int has_normals = attrib.num_normals > 0;
  int has_tcs = attrib.num_texcoords > 0;
  int valid = num_corners % 3 == 0;
  for (int i = 0; i < num_corners && valid; i++) {
    valid = valid_index(attrib.faces[i].v_idx, attrib.num_vertices);
    has_normals = has_normals && valid_index(attrib.faces[i].vn_idx, attrib.num_normals);
    has_tcs = has_tcs && valid_index(attrib.faces[i].vt_idx, attrib.num_texcoords);
  }

  if (valid) {
    m.verts = malloc(num_corners * sizeof(vec3));
    m.normals = has_normals ? malloc(num_corners * sizeof(vec3)) : NULL;
    m.tcs = has_tcs ? malloc(num_corners * sizeof(vec2)) : NULL;
    m.indices = malloc(num_corners * sizeof(int));
    m.num_triangles = num_corners / 3;

    uint32_t table_size = 1;
    while (table_size < 2 * (uint32_t)num_corners)
      table_size *= 2;
    vertex_slot* table = malloc(table_size * sizeof(vertex_slot));
    for (uint32_t i = 0; i < table_size; i++)
      table[i].id = -1;

    for (int i = 0; i < num_corners; i++) {
      const int v_idx = attrib.faces[i].v_idx;
      const int vt_idx = has_tcs ? attrib.faces[i].vt_idx : -1;
      const int vn_idx = has_normals ? attrib.faces[i].vn_idx : -1;

      uint32_t slot = hash_vertex(v_idx, vt_idx, vn_idx) & (table_size - 1);
      while (table[slot].id >= 0 &&
             !(table[slot].v_idx == v_idx && table[slot].vt_idx == vt_idx && table[slot].vn_idx == vn_idx))
        slot = (slot + 1) & (table_size - 1);

      if (table[slot].id < 0) {
        const int id = m.num_vertices++;
        table[slot].v_idx = v_idx;
        table[slot].vt_idx = vt_idx;
        table[slot].vn_idx = vn_idx;
        table[slot].id = id;

        m.verts[id] = make3(attrib.vertices[3 * v_idx + 0], attrib.vertices[3 * v_idx + 1], attrib.vertices[3 * v_idx + 2]);
        if (has_normals)
          m.normals[id] = make3(attrib.normals[3 * vn_idx + 0], attrib.normals[3 * vn_idx + 1], attrib.normals[3 * vn_idx + 2]);
        if (has_tcs)
          m.tcs[id] = make2(attrib.texcoords[2 * vt_idx + 0], attrib.texcoords[2 * vt_idx + 1]);
      }

      m.indices[i] = table[slot].id;
    }

    free(table);

    m.verts = realloc(m.verts, m.num_vertices * sizeof(vec3));
    if (m.normals)
      m.normals = realloc(m.normals, m.num_vertices * sizeof(vec3));
    if (m.tcs)
      m.tcs = realloc(m.tcs, m.num_vertices * sizeof(vec2));
  } else {
    fprintf(stderr, "load_obj: %s has invalid face indices\n", filename);
  }

  tinyobj_attrib_free(&attrib);
  tinyobj_shapes_free(shapes, num_shapes);
  tinyobj_materials_free(materials, num_materials);
  return m;
}

mat4 normalize_mesh(const mesh* m) {
  vec3 mn = m->verts[0];
  vec3 mx = m->verts[0];
  for (int i = 1; i < m->num_vertices; i++) {
    vec3 p = m->verts[i];
    mn = make3(fminf(mn.x, p.x), fminf(mn.y, p.y), fminf(mn.z, p.z));
    mx = make3(fmaxf(mx.x, p.x), fmaxf(mx.y, p.y), fmaxf(mx.z, p.z));
  }
  vec3 c = scale3(add3(mn, mx), 0.5f);
  vec3 size = sub3(mx, mn);
  float ext = fmaxf(size.x, fmaxf(size.y, size.z));
  float s = ext > 1e-12f ? 1.0f / ext : 1.0f;

  return mat4_mul(mat4_make_scale(make3(s, s, s)),
                  mat4_make_translate(scale3(c, -1.0f)));
}

void compute_normals(mesh* m) {
  free(m->normals);
  m->normals = calloc(m->num_vertices, sizeof(vec3));

  for (int t = 0; t < m->num_triangles; t++) {
    int i0 = m->indices[3 * t + 0];
    int i1 = m->indices[3 * t + 1];
    int i2 = m->indices[3 * t + 2];
    vec3 e1 = sub3(m->verts[i1], m->verts[i0]);
    vec3 e2 = sub3(m->verts[i2], m->verts[i0]);
    vec3 fn = cross3(e1, e2);

    m->normals[i0] = add3(m->normals[i0], fn);
    m->normals[i1] = add3(m->normals[i1], fn);
    m->normals[i2] = add3(m->normals[i2], fn);
  }

  for (int i = 0; i < m->num_vertices; i++) {
    vec3 n = normalize3(m->normals[i]);
    if (n.x == 0.0f && n.y == 0.0f && n.z == 0.0f)
      n = make3(0.0f, 1.0f, 0.0f);
    m->normals[i] = n;
  }
}
