#include<stdio.h>
#include<stdlib.h>

#include "matrix.h"

block* allocate_block() {
  block* ret;
  posix_memalign((void**)&ret, CACHE_SIZE, sizeof(block));

  return ret;
}

matrix allocate_matrix(size_t width, size_t height) {
  if(width % BLOCK_SIZE) {
    fprintf(stderr, "Requred width is invalid\n");
    exit(-1);
  }
  if(height % BLOCK_SIZE) {
    fprintf(stderr, "Requred height is invalid\n");
    exit(-1);
  }

  size_t block_width = width / BLOCK_SIZE;
  size_t block_height = height / BLOCK_SIZE;

  matrix matrix;

  matrix.width = width;
  matrix.height = height;

  matrix.blocks = (block***)malloc(block_height * sizeof(block**));
  for(size_t i = 0;i < block_height;i++) {
    matrix.blocks[i] = (block**)malloc(block_width * sizeof(block*));
    for(size_t j = 0;j < block_width;j++) {
      block* tmp = matrix.blocks[i][j] = allocate_block();
      printf("%p\n", tmp);
    }
  }

  return matrix;
}

void show_matrix(matrix mat) {
  for(size_t i = 0; i < mat.height;i++) {
    for(size_t j = 0;j < mat.width;j++) {
      size_t block_h_index = i / BLOCK_SIZE;
      size_t block_w_index = j / BLOCK_SIZE;
      size_t in_block_h = i % BLOCK_SIZE;
      size_t in_block_w = i % BLOCK_SIZE;

      printf("%.10f ", mat.blocks[block_h_index][block_w_index]->element[in_block_h][in_block_w]);
    }
    printf("\n");
  }
}

