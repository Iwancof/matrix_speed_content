#include<stdio.h>
#include<stdlib.h>

#define INNER_TYPE double
#define CACHE_SIZE 0x100

#define BLOCK_SIZE 0x10

#define for_each_blocks(matrix, block_name, text) \
  do { \
    size_t __block_size_width = matrix.width / BLOCK_SIZE; \
    size_t __block_size_height = matrix.height / BLOCK_SIZE; \
    block* block_name; \
    for(size_t __index_of_height = 0; __index_of_height < __block_size_height; __index_of_height++) { \
      for(size_t __index_of_width = 0; __index_of_width < __block_size_width; __index_of_width++) { \
        block_name = matrix.blocks[__index_of_height][__index_of_width]; \
        text \
      } \
    } \
  } while(0)

typedef struct {
  INNER_TYPE element[BLOCK_SIZE][BLOCK_SIZE];
} block;

typedef struct {
  size_t width, height;
  block ***blocks; // TODO: Replace with L2 cache.
} matrix;

block* allocate_block();
matrix allocate_matrix(size_t width, size_t height);
void show_matrix(matrix mat);
