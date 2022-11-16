#include<stdio.h>
#include<stdlib.h>

#define INNER_TYPE double
#define CACHE_SIZE 0x100

typedef struct {
  size_t width, height;
  INNER_TYPE **element;
} normal_matrix;

// CACHE1: 10 20 30 40 ...
//         11 21 31 41 ...
//         12 22 32 42 ...

typedef struct {
  size_t width, height;
  INNER_TYPE **element;
} transpose_matrix;

/// C1
/// 10 20 30 40 ...
/// 11 21 31 41 ...
/// 12 22 32 42 ...

enum matrix_type {
  normal;
  transpose;
}

typedef struct {
  enum {
    normal,
    transpose,
  } tag;
  union {
    normal_matrix normal;
    transpose_matrix transpose;
  } data;
} matrix;

normal_matrix allocate_normal_matrix(size_t width, size_t height);
transpose_matrix allocate_transpose_matrix(size_t width, size_t height);
void show_normal_matrix(normal_matrix mat);
void show_transpose_matrix(transpose_matrix mat);

static inline INNER_TYPE* at_normal(normal_matrix mat, size_t x, size_t y) {
  return &mat.element[y][x];
}
static inline INNER_TYPE* at_transpose(transpose_matrix mat, size_t x, size_t y) {
  return &mat.element[x][y];
}
