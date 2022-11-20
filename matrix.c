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
  // TODO: LEFT_TRANSPOSE
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

inline void BLOCK_MULT(const block *const restrict left,
                       const block *const restrict right,
                       block *const restrict dest) {
  DOC(TODO : replace fast algorithm)
  DOC(__builtin_prefetch(&(left)->element, 0);)
  DOC(__builtin_prefetch(&(right)->element, 0);)
  DOC(__builtin_prefetch(&(dest)->element, 1);)

#ifdef LEFT_TRANSPOSE

#ifdef USE_SIMD
  // SIMD_TYPE calc_memo[BLOCK_SIZE][BLOCK_SIZE / SIMD_WIDTH];

  IGNORE_UNUSED(left);
  IGNORE_UNUSED(right);
  IGNORE_UNUSED(dest);

  for (INDEX_TYPE left_y = 0; left_y < BLOCK_SIZE; left_y++) {
    for (INDEX_TYPE right_x = 0; right_x < BLOCK_SIZE; right_x++) {
#ifdef UNROLLED_SIMD
      __m256d left_fragments[SIMD_PER_BLOCK], right_fragments[SIMD_PER_BLOCK];

      left_fragments[0] = _mm256_load_pd(&left->element[left_y][0]);
      left_fragments[1] = _mm256_load_pd(&left->element[left_y][4]);
      left_fragments[2] = _mm256_load_pd(&left->element[left_y][8]);
      left_fragments[3] = _mm256_load_pd(&left->element[left_y][12]);
      right_fragments[0] = _mm256_load_pd(&right->element[right_x][0]);
      right_fragments[1] = _mm256_load_pd(&right->element[right_x][4]);
      right_fragments[2] = _mm256_load_pd(&right->element[right_x][8]);
      right_fragments[3] = _mm256_load_pd(&right->element[right_x][12]);

      left_fragments[0] = _mm256_mul_pd(left_fragments[0], right_fragments[0]);
      left_fragments[1] = _mm256_mul_pd(left_fragments[1], right_fragments[1]);
      left_fragments[2] = _mm256_mul_pd(left_fragments[2], right_fragments[2]);
      left_fragments[3] = _mm256_mul_pd(left_fragments[3], right_fragments[3]);

      left_fragments[0] = _mm256_add_pd(left_fragments[0], left_fragments[1]);
      left_fragments[0] = _mm256_add_pd(left_fragments[0], left_fragments[2]);
      left_fragments[0] = _mm256_add_pd(left_fragments[0], left_fragments[3]);

      dest->element[right_x][left_y] += left_fragments[0][0] + left_fragments[0][1] + left_fragments[0][2] + left_fragments[0][3];
#else  // UNROLLED_SIMD
      SIMD_TYPE sum = {0};
      for (INDEX_TYPE move = 0; move < BLOCK_SIZE; move += SIMD_PER_BLOCK) {
        SIMD_TYPE left_fragment = _mm256_load_pd(&left->element[left_y][move]);
        SIMD_TYPE right_fragment =
            _mm256_load_pd(&right->element[right_x][move]);

        left_fragment = _mm256_mul_pd(left_fragment, right_fragment);
        sum = _mm256_add_pd(sum, left_fragment);
      }

      dest->element[right_x][left_y] += sum[0] + sum[1] + sum[2] + sum[3];
    }
  }
#endif // UNROLLED_SIMD
#else  // USE_SIMD
  for (INDEX_TYPE width = 0; width < BLOCK_SIZE; width++) {
    for (INDEX_TYPE index = 0; index < BLOCK_SIZE; index++) {
      for (INDEX_TYPE height = 0; height < BLOCK_SIZE; height++) {
        dest->element[width][height] +=
            left->element[height][index] * right->element[width][index];
      }
    }
  }
#endif // USE_SIMD
#else  // LEFT_TRANSPOSE
#ifdef USE_SIMD
#error "Using simd without LEFT_TRANSPOSE is not supported now."
#else  // USE_SIMD
  for (INDEX_TYPE height = 0; height < BLOCK_SIZE; height++) {
    for (INDEX_TYPE width = 0; width < BLOCK_SIZE; width++) {
      for (INDEX_TYPE index = 0; index < BLOCK_SIZE; index++) {
        dest->element[height][width] +=
            left->element[height][index] * right->element[index][width];
      }
    }
  }
#endif // USE_SIMD
#endif // LEFT_TRANSPOSE
    }

#endif // BLOCK_MULT_FUNC
