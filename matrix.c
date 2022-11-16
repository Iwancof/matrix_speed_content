#include<stdio.h>
#include<stdlib.h>

#include "matrix.h"

normal_matrix allocate_normal_matrix(size_t width, size_t height) {
  normal_matrix mat;

  mat.width = width;
  mat.height = height;
  mat.element = (INNER_TYPE**)malloc(sizeof(INNER_TYPE*) * height);

  for(size_t i = 0;i < height;i++) {
    posix_memalign((void**)&mat.element[i], CACHE_SIZE, sizeof(INNER_TYPE) * width);
  }

  return mat;
}

transpose_matrix allocate_transpose_matrix(size_t width, size_t height) {
  transpose_matrix mat;

  mat.width = width;
  mat.height = height;
  mat.element = (INNER_TYPE**)malloc(sizeof(INNER_TYPE*) * width);

  for(size_t i = 0;i < width;i++) {
    posix_memalign((void**)&mat.element[i], CACHE_SIZE, sizeof(INNER_TYPE) * height);
  }

  return mat;
}

void show_normal_matrix(normal_matrix mat) {
  for(size_t i = 0;i < mat.height;i++) {
    for(size_t j = 0;j < mat.width;j++) {
      printf("%.10f ", mat.element[i][j]);
    }
    printf("\n");
  }
}

void show_transpose_matrix(transpose_matrix mat) {
  for(size_t i = 0;i < mat.height;i++) {
    for(size_t j = 0;j < mat.width;j++) {
      printf("%.10f ", mat.element[j][i]);
    }
    printf("\n");
  }
}

matrix from_normal(normal_matrix mat) {
  matrix mat;
  mat.normal;
}

