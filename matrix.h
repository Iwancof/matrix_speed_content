#include <stdio.h>
#include <stdlib.h>

#define INNER_TYPE double
#define CACHE_SIZE 0x100

#define BLOCK_SIZE 0x10
// #define MATRIX_SIZE 0x20000000
#define MATRIX_SIZE 0x400
#define SUPER_SIZE (MATRIX_SIZE / BLOCK_SIZE)

#define DOC(ignore)

#define for_each_blocks(x, y)                                                  \
  for (x = 0; x < SUPER_SIZE; x++)                                             \
    for (y = 0; y < SUPER_SIZE; y++)

#define for_each_element(x, y)                                                 \
  for (x = 0; x < BLOCK_SIZE; x++)                                             \
    for (y = 0; y < BLOCK_SIZE; y++)

typedef struct {
  INNER_TYPE element[BLOCK_SIZE][BLOCK_SIZE];
} block;

typedef struct {
  block blocks[SUPER_SIZE][SUPER_SIZE]; // TODO: Replace with L2 cache.
} matrix;

matrix *allocate_matrix();

/// ASSUME: mat has been initialized.
void show_matrix(matrix *mat);

/// ASSYME: blk has been initialized.
void show_block(block *blk);

/// ASSUME: left, right and dest has been initialized.
/// NOTE: This function will not initialize dest.
///       The result will be added into dest.
#define BLOCK_MULT(left, right, dest)                                          \
  do {                                                                         \
    DOC(TODO : replace fast algorithm)                                         \
    DOC(__builtin_prefetch(&(left)->element, 0))                               \
    DOC(__builtin_prefetch(&(right)->element, 0))                              \
    DOC(__builtin_prefetch(&(dest)->element, 1))                               \
    for (unsigned short width = 0; width < BLOCK_SIZE; width++) {              \
      for (unsigned short height = 0; height < BLOCK_SIZE; height++) {         \
        for (unsigned short index = 0; index < BLOCK_SIZE; index++) {          \
          (dest)->element[height][width] +=                                    \
              (left)->element[height][index] * (right)->element[index][width]; \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  } while (0)

/// ASSUME: left, right, dest has been allocated.
/// ASSUME: dest has been initialized.

static inline void matrix_mult(matrix *left, matrix *right, matrix *dest) {
  for (size_t y = 0; y < SUPER_SIZE; y++) {
    for (size_t x = 0; x < SUPER_SIZE; x++) {
      for (size_t i = 0; i < SUPER_SIZE; i++) {
        BLOCK_MULT(&left->blocks[y][i], &right->blocks[i][x],
                   &dest->blocks[y][x]);
      }
    }
  }
}
