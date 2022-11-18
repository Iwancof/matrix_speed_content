#include<stdio.h>
#include<stdlib.h>
#include<sys/mman.h>

#include "matrix.h"

matrix* allocate_matrix() { 
  size_t mapped, alignment;
  matrix* target;

  alignment = sizeof(matrix);
mapped = (size_t)mmap(NULL, alignment * 2, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

  if(mapped == 0) {
    perror("allocation error");
  }

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

block* allocate_block() {
  block* ret = NULL;

  if(posix_memalign((void**)&ret, BLOCK_SIZE * BLOCK_SIZE, sizeof(block))) {
    perror("allocation error");
  }

  return ret;

}

void show_matrix(matrix *mat) {
  for(size_t i = 0; i < MATRIX_SIZE;i++) {
    for(size_t j = 0;j < MATRIX_SIZE;j++) {
      size_t block_h_index = i / BLOCK_SIZE;
      size_t block_w_index = j / BLOCK_SIZE;
      size_t in_block_h = i % BLOCK_SIZE;
      size_t in_block_w = j % BLOCK_SIZE;

      printf("%2.2f ", mat->blocks[block_h_index][block_w_index].element[in_block_h][in_block_w]);
    }
    printf("\n");
  }
}

void show_block(block *blk) {
  for(INDEX_TYPE y = 0;y < BLOCK_SIZE;y++) {
    for(INDEX_TYPE x = 0; x < BLOCK_SIZE;x++) {
      printf("%3.3f ", blk->element[y][x]);
    }
    puts("");
  }
}
