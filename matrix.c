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
  // TODO: RIGHT_TRANSPOSE
  for (INDEX_TYPE y = 0; y < BLOCK_SIZE; y++) {
    for (INDEX_TYPE x = 0; x < BLOCK_SIZE; x++) {
      printf("%3.3f ", blk->element[y][x]);
    }
    puts("");
  }
}

void left_pre_block(block *blk) {
  IGNORE_UNUSED(blk);
#ifdef RIGHT_TRANSPOSE
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
#ifdef RIGHT_TRANSPOSE
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

void block_add(const block *const restrict from, block *const restrict dest) {
  for (INDEX_TYPE x = 0; x < BLOCK_SIZE; x += 4) {
    for (INDEX_TYPE y = 0; y < BLOCK_SIZE; y++) {
      SIMD_TYPE fragment, dest_fragment;
      fragment = _mm256_load_pd(&from->element[y][x]);
      dest_fragment = _mm256_load_pd(&dest->element[y][x]);

      fragment = _mm256_add_pd(fragment, dest_fragment);
      _mm256_store_pd(&dest->element[y][x], fragment);
    }
  }
}

void block_add_2(const block *const restrict left,
                 const block *const restrict right,
                 block *const restrict dest) {
#ifdef RIGHT_TRANSPOSE
#ifdef USE_SIMD
  // right and dest is transposed.
  for (INDEX_TYPE x = 0; x < BLOCK_SIZE; x += 4) {
    for (INDEX_TYPE y = 0; y < BLOCK_SIZE; y++) {
      SIMD_TYPE left_fragment, right_fragment;
      left_fragment[0] = left->element[x + 0][y];
      left_fragment[1] = left->element[x + 1][y];
      left_fragment[2] = left->element[x + 2][y];
      left_fragment[3] = left->element[x + 3][y];

      right_fragment = _mm256_load_pd(&right->element[y][x]);

      left_fragment = _mm256_add_pd(left_fragment, right_fragment);

      _mm256_store_pd(&dest->element[y][x], left_fragment);
    }
  }

#else // USE_SIMD
#error if you want it, write it yourself.
#endif // USE_SIMD

#else // RIGHT_TRANSPOSE
#error if you want it, write it yourself.
#endif // RIGHT_TRANSPOSE
}

#ifdef BLOCK_MULT_FUNC

inline void BLOCK_MULT(const block *const restrict left,
                       const block *const restrict right,
                       block *const restrict dest) {
  DOC(TODO : replace fast algorithm)
  DOC(__builtin_prefetch(&(left)->element, 0);)
  DOC(__builtin_prefetch(&(right)->element, 0);)
  DOC(__builtin_prefetch(&(dest)->element, 1);)

#ifdef RIGHT_TRANSPOSE

#ifdef USE_SIMD

#ifndef UNROLLED_SIMD
#error Set UNROLLED_SIMD to UNROLL_DISABLED, UNROLL_NORMAL, UNROLL_HARD.
#endif // UNROLLED_SIMD

#if UNROLLED_SIMD == UNROLL_DISABLED
  for (INDEX_TYPE left_y = 0; left_y < BLOCK_SIZE; left_y++) {
    for (INDEX_TYPE right_x = 0; right_x < BLOCK_SIZE; right_x++) {
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
#elif UNROLLED_SIMD == UNROLL_NORMAL
  for (INDEX_TYPE left_y = 0; left_y < BLOCK_SIZE; left_y++) {
    for (INDEX_TYPE right_x = 0; right_x < BLOCK_SIZE; right_x++) {
      SIMD_TYPE left_fragments[SIMD_PER_BLOCK], right_fragments[SIMD_PER_BLOCK];

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

      dest->element[right_x][left_y] +=
          left_fragments[0][0] + left_fragments[0][1] + left_fragments[0][2] +
          left_fragments[0][3];
    }
  }
#elif UNROLLED_SIMD == UNROLL_HARD

#if BLOCK_SIZE != 0x10
#error UNROLL_HARD needs being BLOCK_SIZE == 0x10
#endif // SIMD_PER_BLOCK

  SIMD_TYPE left_fragment0;
  SIMD_TYPE left_fragment1;
  SIMD_TYPE left_fragment2;
  SIMD_TYPE left_fragment3;

  SIMD_TYPE right_fragment0;
  SIMD_TYPE right_fragment1;
  SIMD_TYPE right_fragment2;
  SIMD_TYPE right_fragment3;

  SIMD_TYPE sum0;
  SIMD_TYPE sum1;
  SIMD_TYPE sum2;
  SIMD_TYPE sum3;

  SIMD_TYPE tmp0;
  SIMD_TYPE tmp1;
  SIMD_TYPE tmp2;
  SIMD_TYPE tmp3;

  // consume 16 registers(ymm)

  LOAD_LEFT_FRAGMENT(left_fragment, left, 0);
  // LOAD_RIGHT_FRAGMENT(right_fragment, right, 0, 0);

  for (INDEX_TYPE left_up_down_counter = 0; left_up_down_counter < 4;
       left_up_down_counter++) {
    INDEX_TYPE left_y = 0;

    LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 0);
    // LOAD_LEFT_FRAGMENT(left_fragment, left, left_y);
    INIT_SUM(sum, left_fragment, right_fragment, 0);

    LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 4);
    MULT_TO_TMP(tmp, left_fragment, right_fragment, 1);
    SUBMIT_TO_SUM(sum, tmp);

    LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 8);
    MULT_TO_TMP(tmp, left_fragment, right_fragment, 2);
    SUBMIT_TO_SUM(sum, tmp);

    LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 12);
    MULT_TO_TMP(tmp, left_fragment, right_fragment, 3);
    SUBMIT_TO_SUM(sum, tmp);

    WRITEBACK_TO_DEST(dest, left_y, left_up_down_counter * 4, sum);

    left_y += 1;

    LOAD_LEFT_FRAGMENT(left_fragment, left, left_y);
    INIT_SUM(sum, left_fragment, right_fragment, 3);

    LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 8);
    MULT_TO_TMP(tmp, left_fragment, right_fragment, 2);
    SUBMIT_TO_SUM(sum, tmp);

    LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 4);
    MULT_TO_TMP(tmp, left_fragment, right_fragment, 1);
    SUBMIT_TO_SUM(sum, tmp);

    LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 0);
    MULT_TO_TMP(tmp, left_fragment, right_fragment, 0);
    SUBMIT_TO_SUM(sum, tmp);

    WRITEBACK_TO_DEST(dest, left_y, left_up_down_counter * 4, sum);

    left_y += 1;

    do {
      LOAD_LEFT_FRAGMENT(left_fragment, left, left_y);
      INIT_SUM(sum, left_fragment, right_fragment, 0);

      LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 4);
      MULT_TO_TMP(tmp, left_fragment, right_fragment, 1);
      SUBMIT_TO_SUM(sum, tmp);

      LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 8);
      MULT_TO_TMP(tmp, left_fragment, right_fragment, 2);
      SUBMIT_TO_SUM(sum, tmp);

      LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 12);
      MULT_TO_TMP(tmp, left_fragment, right_fragment, 3);
      SUBMIT_TO_SUM(sum, tmp);

      WRITEBACK_TO_DEST(dest, left_y, left_up_down_counter * 4, sum);

      left_y += 1;

      LOAD_LEFT_FRAGMENT(left_fragment, left, left_y);
      INIT_SUM(sum, left_fragment, right_fragment, 3);

      LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 8);
      MULT_TO_TMP(tmp, left_fragment, right_fragment, 2);
      SUBMIT_TO_SUM(sum, tmp);

      LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 4);
      MULT_TO_TMP(tmp, left_fragment, right_fragment, 1);
      SUBMIT_TO_SUM(sum, tmp);

      LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 0);
      MULT_TO_TMP(tmp, left_fragment, right_fragment, 0);
      SUBMIT_TO_SUM(sum, tmp);

      WRITEBACK_TO_DEST(dest, left_y, left_up_down_counter * 4, sum);

      left_y += 1;
    } while (left_y < 16);

    left_up_down_counter += 1;
    left_y -= 1;

    LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 0);
    // LOAD_LEFT_FRAGMENT(left_fragment, left, left_y);
    INIT_SUM(sum, left_fragment, right_fragment, 0);

    LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 4);
    MULT_TO_TMP(tmp, left_fragment, right_fragment, 1);
    SUBMIT_TO_SUM(sum, tmp);

    LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 8);
    MULT_TO_TMP(tmp, left_fragment, right_fragment, 2);
    SUBMIT_TO_SUM(sum, tmp);

    LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 12);
    MULT_TO_TMP(tmp, left_fragment, right_fragment, 3);
    SUBMIT_TO_SUM(sum, tmp);

    WRITEBACK_TO_DEST(dest, left_y, left_up_down_counter * 4, sum);

    left_y -= 1;

    LOAD_LEFT_FRAGMENT(left_fragment, left, left_y);
    INIT_SUM(sum, left_fragment, right_fragment, 3);

    LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 8);
    MULT_TO_TMP(tmp, left_fragment, right_fragment, 2);
    SUBMIT_TO_SUM(sum, tmp);

    LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 4);
    MULT_TO_TMP(tmp, left_fragment, right_fragment, 1);
    SUBMIT_TO_SUM(sum, tmp);

    LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 0);
    MULT_TO_TMP(tmp, left_fragment, right_fragment, 0);
    SUBMIT_TO_SUM(sum, tmp);

    WRITEBACK_TO_DEST(dest, left_y, left_up_down_counter * 4, sum);

    do {
      left_y -= 1;

      LOAD_LEFT_FRAGMENT(left_fragment, left, left_y);
      INIT_SUM(sum, left_fragment, right_fragment, 0);

      LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 4);
      MULT_TO_TMP(tmp, left_fragment, right_fragment, 1);
      SUBMIT_TO_SUM(sum, tmp);

      LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 8);
      MULT_TO_TMP(tmp, left_fragment, right_fragment, 2);
      SUBMIT_TO_SUM(sum, tmp);

      LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 12);
      MULT_TO_TMP(tmp, left_fragment, right_fragment, 3);
      SUBMIT_TO_SUM(sum, tmp);

      WRITEBACK_TO_DEST(dest, left_y, left_up_down_counter * 4, sum);

      left_y -= 1;

      LOAD_LEFT_FRAGMENT(left_fragment, left, left_y);
      INIT_SUM(sum, left_fragment, right_fragment, 3);

      LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 8);
      MULT_TO_TMP(tmp, left_fragment, right_fragment, 2);
      SUBMIT_TO_SUM(sum, tmp);

      LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 4);
      MULT_TO_TMP(tmp, left_fragment, right_fragment, 1);
      SUBMIT_TO_SUM(sum, tmp);

      LOAD_RIGHT_FRAGMENT(right_fragment, right, 4 * left_up_down_counter, 0);
      MULT_TO_TMP(tmp, left_fragment, right_fragment, 0);
      SUBMIT_TO_SUM(sum, tmp);

      WRITEBACK_TO_DEST(dest, left_y, left_up_down_counter * 4, sum);
    } while (0 < left_y);
  }

#else // UNROLLED_SIMD
#error Set UNROLLED_SIMD to UNROLL_DISABLED, UNROLL_NORMAL, UNROLL_HARD.
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
#else  // RIGHT_TRANSPOSE
#ifdef USE_SIMD
#error "Using simd without RIGHT_TRANSPOSE is not supported now."
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
#endif // RIGHT_TRANSPOSE
}
#endif // BLOCK_MULT_FUNC

void matrix_mult_per_block(const matrix *const restrict left,
                           const matrix *const restrict right,
                           matrix *const restrict dest,
                           matrix *const restrict thread_memo[PARALLEL]) {
#ifdef RIGHT_TRANSPOSE

#ifdef USE_REDUCTION
  INDEX_TYPE cache_block_x, cache_block_y, move;

#pragma omp parallel for private(cache_block_y, move) num_threads(PARALLEL)    \
    schedule(static)
  for (cache_block_x = 0; cache_block_x < SUPER_SIZE; cache_block_x++) {
    const int thread_index = omp_get_thread_num();
    matrix *const thread_dest = thread_memo[thread_index];
    for (cache_block_y = 0; cache_block_y < SUPER_SIZE;
         cache_block_y++) { // for write cache
      for (move = 0; move < SUPER_SIZE; move++) {
        BLOCK_MULT(&left->blocks[cache_block_y][move],
                   &right->blocks[cache_block_x][move],
                   &thread_dest->blocks[cache_block_x][cache_block_y]);
      }
    }
  }

#pragma omp parallel for private(cache_block_x)              \
    num_threads(PARALLEL) schedule(static)
  for (cache_block_y = 0; cache_block_y < SUPER_SIZE; cache_block_y++) {
    for (cache_block_x = 0; cache_block_x < SUPER_SIZE; cache_block_x++) {
      block *const dest_block = &dest->blocks[cache_block_y][cache_block_x];
      for (int i = 0; i < PARALLEL; i++) {
        const block *const from =
            &thread_memo[i]->blocks[cache_block_y][cache_block_x];
        block_add(from, dest_block);
      }
    }
  }
#else // USE_REDUCTION
  INDEX_TYPE cache_block_x, cache_block_y, move;

  IGNORE_UNUSED(thread_memo);

#pragma omp parallel for private(cache_block_y, move) num_threads(PARALLEL)    \
    schedule(static)
  for (cache_block_x = 0; cache_block_x < SUPER_SIZE; cache_block_x++) {
    for (cache_block_y = 0; cache_block_y < SUPER_SIZE;
         cache_block_y++) { // for write cache
      for (move = 0; move < SUPER_SIZE; move++) {
        BLOCK_MULT(&left->blocks[cache_block_y][move],
                   &right->blocks[cache_block_x][move],
                   &dest->blocks[cache_block_x][cache_block_y]);
      }
    }
  }
#endif // USE_REDUCTION

#else // RIGHT_TRANSPOSE

#pragma omp parallel for private(cache_block_y, move) num_threads(PARALLEL)    \
    schedule(static)
  for (cache_block_x = 0; cache_block_x < SUPER_SIZE; cache_block_x++) {
    int thread_index = omp_get_thread_num();
    matrix *thread_dest = thread_memo[thread_index];
    for (cache_block_y = 0; cache_block_y < SUPER_SIZE;
         cache_block_y++) { // for write cache
      for (move = 0; move < SUPER_SIZE; move++) {
        BLOCK_MULT(&left->blocks[cache_block_y][cache_block_x],
                   &right->blocks[cache_block_x][move],
                   &thread_dest->blocks[cache_block_y][move]);
      }
    }
  }

#pragma omp parallel for private(cache_block_x, block_x, block_y)              \
    num_threads(PARALLEL) schedule(static)
  for (cache_block_x = 0; cache_block_x < SUPER_SIZE; cache_block_x++) {
    for (cache_block_y = 0; cache_block_y < SUPER_SIZE; cache_block_y++) {
      block *dest_block = &dest->blocks[cache_block_y][cache_block_x];
      for (int i = 0; i < PARALLEL; i++) {
        block *from = &thread_memo[i]->blocks[cache_block_y][cache_block_x];
        for (block_x = 0; block_x < BLOCK_SIZE; block_x++) {
          for (block_y = 0; block_y < BLOCK_SIZE; block_y++) {
            dest_block->element[block_y][block_x] +=
                from->element[block_y][block_x];
          }
        }
      }
    }
  }
#endif
}

void matrix_mult_vanilla(matrix *left, matrix *right, matrix *dest) {
  // TODO: RIGHT_TRANSPOSE
  for (INDEX_TYPE x = 0; x < SUPER_SIZE; x++) {
    for (INDEX_TYPE y = 0; y < SUPER_SIZE; y++) {
      for (INDEX_TYPE i = 0; i < SUPER_SIZE; i++) {
        BLOCK_MULT(&left->blocks[y][i], &right->blocks[i][x],
                   &dest->blocks[y][x]);
      }
    }
  }
}
