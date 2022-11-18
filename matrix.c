#include<stdio.h>
#include<stdlib.h>
#include<sys/mman.h>

#include "matrix.h"

matrix* allocate_matrix() { 
  size_t mapped, alignment, length;
  matrix* target;

  alignment = sizeof(matrix);
mapped = (size_t)mmap(NULL, alignment * 2, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

  if(mapped == 0) {
    perror("allocation error");
  }

  target = (matrix*)(((mapped - 1) & ~(alignment - 1)) + alignment);

  length = (size_t)target - mapped; 
  if(length) {
    munmap((void*)mapped, length);
  }

  length = (mapped + alignment * 2) - (size_t)(target + 1);
  if(length) {
    munmap((void*)(target + 1), length);
  }

  size_t x, y, bx, by;
  for_each_blocks(x, y) {
    for_each_element(bx, by) {
      target->blocks[y][x].element[by][bx] = 0.;
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
