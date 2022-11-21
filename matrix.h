#include <immintrin.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#include "settings"

#define DOC(ignore)

#define for_each_blocks(x, y)                                                  \
  for (x = 0; x < SUPER_SIZE; x++)                                             \
    for (y = 0; y < SUPER_SIZE; y++)

#define for_each_element(x, y)                                                 \
  for (x = 0; x < BLOCK_SIZE; x++)                                             \
    for (y = 0; y < BLOCK_SIZE; y++)

#define LOAD_LEFT_FRAGMENT(frag, left, y)                                      \
  do {                                                                         \
    frag##0 = _mm256_load_pd(&left->element[y][0]);                            \
    frag##1 = _mm256_load_pd(&left->element[y][4]);                            \
    frag##2 = _mm256_load_pd(&left->element[y][8]);                            \
    frag##3 = _mm256_load_pd(&left->element[y][12]);                           \
  } while (0)

#define LOAD_RIGHT_FRAGMENT(frag, right, base_x, y)                            \
  do {                                                                         \
    frag##0 = _mm256_load_pd(&right->element[(base_x) + 0][y]);                \
    frag##1 = _mm256_load_pd(&right->element[(base_x) + 1][y]);                \
    frag##2 = _mm256_load_pd(&right->element[(base_x) + 2][y]);                \
    frag##3 = _mm256_load_pd(&right->element[(base_x) + 3][y]);                \
  } while (0)

#define INIT_SUM(sum, lf, rf, li)                                              \
  do {                                                                         \
    sum##0 = _mm256_mul_pd(lf##li, rf##0);                                     \
    sum##1 = _mm256_mul_pd(lf##li, rf##1);                                     \
    sum##2 = _mm256_mul_pd(lf##li, rf##2);                                     \
    sum##3 = _mm256_mul_pd(lf##li, rf##3);                                     \
  } while (0)

#define MULT_TO_TMP(tmp, lf, rf, li)                                           \
  do {                                                                         \
    tmp##0 = _mm256_mul_pd(lf##li, rf##0);                                     \
    tmp##1 = _mm256_mul_pd(lf##li, rf##1);                                     \
    tmp##2 = _mm256_mul_pd(lf##li, rf##2);                                     \
    tmp##3 = _mm256_mul_pd(lf##li, rf##3);                                     \
  } while (0)

#define SUBMIT_TO_SUM(sum, tmp)                                                \
  do {                                                                         \
    sum##0 = _mm256_add_pd(sum##0, tmp##0);                                    \
    sum##1 = _mm256_add_pd(sum##1, tmp##1);                                    \
    sum##2 = _mm256_add_pd(sum##2, tmp##2);                                    \
    sum##3 = _mm256_add_pd(sum##3, tmp##3);                                    \
  } while (0)

#define WRITEBACK_TO_DEST(dest, left_y, left_base_x, sum)                      \
  do {                                                                         \
    dest->element[left_y][(left_base_x) + 0] =                                 \
        sum##0 [0] + sum##0 [1] + sum##0 [2] + sum##0 [3];                     \
    dest->element[left_y][(left_base_x) + 1] =                                 \
        sum##1 [0] + sum##1 [1] + sum##1 [2] + sum##1 [3];                     \
    dest->element[left_y][(left_base_x) + 2] =                                 \
        sum##2 [0] + sum##2 [1] + sum##2 [2] + sum##2 [3];                     \
    dest->element[left_y][(left_base_x) + 3] =                                 \
        sum##3 [0] + sum##3 [1] + sum##3 [2] + sum##3 [3];                     \
  } while (0)

typedef struct {
  INNER_TYPE element[BLOCK_SIZE][BLOCK_SIZE];
} block;

typedef struct {
  block blocks[SUPER_SIZE][SUPER_SIZE]; // TODO: Replace with L2 cache.
} matrix;

matrix *map_matrix();
void unmap_matrix(matrix *mat);
block *allocate_block();
void free_block(block *blk);

/// ASSUME: mat has been initialized.
void show_matrix(matrix *mat);

/// ASSUME: blk has been initialized.
void show_block(block *blk);

/// ASSUME: blk has been initialized.
void left_pre_block(block *blk);

/// ASSUME: mat has been initialized.
void left_pre_matrix(matrix *mat);

/// ASSUME: left, right and dest has been initialized.
/// NOTE: This function will not initialize dest.
///       The result will be added into dest.

#ifdef BLOCK_MULT_FUNC

void BLOCK_MULT(const block *const restrict left,
                const block *const restrict right, block *const restrict dest);

#else // BLOCK_MULT_FUNC

#ifdef LEFT_TRANSPOSE

#define BLOCK_MULT(left, right, dest)                                          \
  do {                                                                         \
    DOC(TODO : replace fast algorithm)                                         \
    DOC(__builtin_prefetch(&(left)->element, 0))                               \
    DOC(__builtin_prefetch(&(right)->element, 0))                              \
    DOC(__builtin_prefetch(&(dest)->element, 1))                               \
    for (INDEX_TYPE height = 0; height < BLOCK_SIZE; height++) {               \
      for (INDEX_TYPE index = 0; index < BLOCK_SIZE; index++) {                \
        for (INDEX_TYPE width = 0; width < BLOCK_SIZE; width++) {              \
          (dest)->element[width][height] +=                                    \
              (left)->element[height][index] * (right)->element[index][width]; \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  } while (0)

#else

#define BLOCK_MULT(left, right, dest)                                          \
  do {                                                                         \
    DOC(TODO : replace fast algorithm)                                         \
    DOC(__builtin_prefetch(&(left)->element, 0))                               \
    DOC(__builtin_prefetch(&(right)->element, 0))                              \
    DOC(__builtin_prefetch(&(dest)->element, 1))                               \
    for (INDEX_TYPE height = 0; height < BLOCK_SIZE; height++) {               \
      for (INDEX_TYPE width = 0; width < BLOCK_SIZE; width++) {                \
        for (INDEX_TYPE index = 0; index < BLOCK_SIZE; index++) {              \
          (dest)->element[height][width] +=                                    \
              (left)->element[height][index] * (right)->element[width][index]; \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  } while (0)

#endif // LEFT_TRANSPOSE

#endif // BLOCK_MULT_FUNC

/// ASSUME: left, right, dest has been allocated.
/// ASSUME: dest has been initialized.

static inline void matrix_mult_per_block(
    const matrix *const restrict left, const matrix *const restrict right,
    matrix *const restrict dest, matrix *const restrict thread_memo[PARALLEL]) {
  INDEX_TYPE cache_block_x, cache_block_y, move, block_x, block_y;

#ifdef LEFT_TRANSPOSE

#pragma omp parallel for private(cache_block_x, move) num_threads(PARALLEL)    \
    schedule(static)
  for (cache_block_y = 0; cache_block_y < SUPER_SIZE;
       cache_block_y++) { // for write cache
    const int thread_index = omp_get_thread_num();
    matrix *const thread_dest = thread_memo[thread_index];
    for (cache_block_x = 0; cache_block_x < SUPER_SIZE; cache_block_x++) {
      for (move = 0; move < SUPER_SIZE; move++) {
        BLOCK_MULT(&left->blocks[cache_block_y][move],
                   &right->blocks[cache_block_x][move],
                   &thread_dest->blocks[cache_block_x][cache_block_y]);
      }
    }
  }

  puts("reducting");

#pragma omp parallel for private(cache_block_x, block_x, block_y)              \
    num_threads(PARALLEL) schedule(static)
  for (cache_block_y = 0; cache_block_y < SUPER_SIZE; cache_block_y++) {
    for (cache_block_x = 0; cache_block_x < SUPER_SIZE; cache_block_x++) {
      block *const dest_block = &dest->blocks[cache_block_y][cache_block_x];
      for (int i = 0; i < PARALLEL; i++) {
        const block *const from =
            &thread_memo[i]->blocks[cache_block_y][cache_block_x];
        for (block_x = 0; block_x < BLOCK_SIZE; block_x++) {
          for (block_y = 0; block_y < BLOCK_SIZE; block_y++) {
            dest_block->element[block_y][block_x] +=
                from->element[block_y][block_x];
          }
        }
      }
    }
  }

#else // LEFT_TRANSPOSE

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

static inline void matrix_mult_vanilla(matrix *left, matrix *right,
                                       matrix *dest) {
  // TODO: LEFT_TRANSPOSE
  for (INDEX_TYPE x = 0; x < SUPER_SIZE; x++) {
    for (INDEX_TYPE y = 0; y < SUPER_SIZE; y++) {
      for (INDEX_TYPE i = 0; i < SUPER_SIZE; i++) {
        BLOCK_MULT(&left->blocks[y][i], &right->blocks[i][x],
                   &dest->blocks[y][x]);
      }
    }
  }
}
