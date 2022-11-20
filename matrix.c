#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include <x86intrin.h>

#include "matrix.h"

#define IGNORE_UNUSED(x) (void)(x)

// TODO: assuming `sizeof(matrix)` is page aligned is not good.
matrix *map_matrix() {
  size_t mapped, alignment, length;
  matrix *target;

  alignment = sizeof(matrix);
  mapped = (size_t)mmap(NULL, alignment * 2, PROT_READ | PROT_WRITE,
                        MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

  if (mapped == 0) {
    perror("allocation error");
  }

  target = (matrix *)(((mapped - 1) & ~(alignment - 1)) + alignment);

  length = (size_t)target - mapped;
  if (length) {
    int ret = munmap((void *)mapped, length);
    if (ret) {
      perror("munmap error 1");
    }
  }

  length = (mapped + alignment * 2) - (size_t)(target + 1);
  if (length) {
    int ret = munmap((void *)(target + 1), length);
    if (ret) {
      perror("munmap error 2");
    }
  }

  // printf("allocated %p\n", target);

  size_t x, y, bx, by;
  for_each_blocks(x, y) {
    for_each_element(bx, by) { target->blocks[y][x].element[by][bx] = 0.; }
  }

  return target;
}

void unmap_matrix(matrix *mat) { munmap((void *)mat, sizeof(matrix)); }

block *allocate_block() {
  block *ret = NULL;
  INDEX_TYPE x, y;

  if (posix_memalign((void **)&ret, BLOCK_SIZE * BLOCK_SIZE, sizeof(block))) {
    perror("allocation error");
  }

  for_each_element(x, y) { ret->element[y][x] = 0.; }

  return ret;
}

void free_block(block *blk) { free(blk); }

void show_matrix(matrix *mat) {
  for (size_t i = 0; i < MATRIX_SIZE; i++) {
    for (size_t j = 0; j < MATRIX_SIZE; j++) {
      size_t block_h_index = i / BLOCK_SIZE;
      size_t block_w_index = j / BLOCK_SIZE;
      size_t in_block_h = i % BLOCK_SIZE;
      size_t in_block_w = j % BLOCK_SIZE;

      printf("%2.2f ", mat->blocks[block_h_index][block_w_index]
                           .element[in_block_h][in_block_w]);
    }
    printf("\n");
  }
}

void show_block(block *blk) {
  for (INDEX_TYPE y = 0; y < BLOCK_SIZE; y++) {
    for (INDEX_TYPE x = 0; x < BLOCK_SIZE; x++) {
      printf("%3.3f ", blk->element[y][x]);
    }
    puts("");
  }
}

void left_pre_block(block *blk) {
  IGNORE_UNUSED(blk);
#ifdef LEFT_TRANSPOSE
  for (INDEX_TYPE x = 1; x < BLOCK_SIZE; x++) { // BLOCK_SIZE is not zero.
    for (INDEX_TYPE y = 0; y < x; y++) {
      double tmp = blk->element[y][x];
      blk->element[y][x] = blk->element[x][y];
      blk->element[x][y] = tmp;
    }
  }
#endif
}

void left_pre_matrix(matrix *mat) {
  IGNORE_UNUSED(mat);
#ifdef LEFT_TRANSPOSE
  INDEX_TYPE x, y;
  for_each_blocks(x, y) { left_pre_block(&mat->blocks[y][x]); }

  block *tmp = allocate_block();
  for (x = 1; x < SUPER_SIZE; x++) {
    for (y = 0; y < x; y++) {
      memcpy(tmp, &mat->blocks[y][x], sizeof(block));
      memcpy(&mat->blocks[y][x], &mat->blocks[x][y], sizeof(block));
      memcpy(&mat->blocks[x][y], tmp, sizeof(block));
    }
  }

  free_block(tmp);
#endif
}

#ifdef BLOCK_MULT_FUNC

void BLOCK_MULT(block *left, block *right, block *dest) {
  DOC(TODO : replace fast algorithm)
  DOC(__builtin_prefetch(&(left)->element, 0))
  DOC(__builtin_prefetch(&(right)->element, 0))
  DOC(__builtin_prefetch(&(dest)->element, 1))

#ifdef LEFT_TRANSPOSE
  for (INDEX_TYPE width = 0; width < BLOCK_SIZE; width++) {
    for (INDEX_TYPE index = 0; index < BLOCK_SIZE; index++) {
      for (INDEX_TYPE height = 0; height < BLOCK_SIZE; height++) {
        dest->element[width][height] +=
            left->element[height][index] * right->element[width][index];
      }
    }
  }
#else  // LEFT_TRANSPOSE
  for (INDEX_TYPE height = 0; height < BLOCK_SIZE; height++) {
    for (INDEX_TYPE width = 0; width < BLOCK_SIZE; width++) {
      for (INDEX_TYPE index = 0; index < BLOCK_SIZE; index++) {
        dest->element[height][width] +=
            left->element[height][index] * right->element[index][width];
      }
    }
  }
#endif // LEFT_TRANSPOSE
}

/*
void BLOCK_MULT(block *left, block *right, block *dest) {
  DOC(TODO : replace fast algorithm)
  DOC(__builtin_prefetch(&(left)->element, 0))
  DOC(__builtin_prefetch(&(right)->element, 0))
  DOC(__builtin_prefetch(&(dest)->element, 1))

  puts("");

  for(INDEX_TYPE left_y = 0;left_y < BLOCK_SIZE;) {
    INDEX_TYPE right_x = 0;

    SIMD_TYPE left_fragment = _mm256_load_pd(&left->element[left_y][0]);

    for(;right_x < BLOCK_SIZE;right_x++) {
    }

    left_y += 1;
    right_x -= 1;

    for(;0 <= right_x;right_x--) {
    }

    left_y += 1;
  }
}
*/

#endif
