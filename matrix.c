#include<stdio.h>
#include<stdlib.h>
#include<sys/mman.h>

#include "matrix.h"

matrix* allocate_matrix() { 
  size_t mapped, alignment;
  matrix* target;

  alignment = sizeof(matrix);

  mapped = (size_t)mmap(NULL, alignment * 2, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

  target = (matrix*)(((mapped - 1) & ~(alignment - 1)) + alignment);
  // TODO: munmap unused chunks

  for(size_t x = 0; x < SUPER_SIZE;x++) {
    for(size_t y = 0; y < SUPER_SIZE;y++) {
      for(size_t bx = 0; bx < BLOCK_SIZE;bx++) {
        for(size_t by = 0; by < BLOCK_SIZE;by++) {
          target->blocks[x][y].element[bx][by] = 0.;
        }
      }
    }
  }

  return target;

}

void show_matrix(matrix *mat) {
  for(size_t i = 0; i < MATRIX_SIZE;i++) {
    for(size_t j = 0;j < MATRIX_SIZE;j++) {
      size_t block_h_index = i / BLOCK_SIZE;
      size_t block_w_index = j / BLOCK_SIZE;
      size_t in_block_h = i % BLOCK_SIZE;
      size_t in_block_w = i % BLOCK_SIZE;

      printf("%.10f ", mat->blocks[block_h_index][block_w_index].element[in_block_h][in_block_w]);
    }
    printf("\n");
  }
}

