CC := gcc
PYTHON := python3
OBJS := main.o matrix.o
CFLAGS := -W -Wall -Wextra -fopenmp -O3

TEST_CFLAGS := $(CFLAGS) -lcunit -g3

main: $(OBJS)
	$(CC) $^ $(CFLAGS) -o $@

block_test_value:
	$(PYTHON) ./gen_block_test_value.py

matrix_test_value:
	$(PYTHON) ./matrix_block_test_value.py

unit_tests: unit_tests.c matrix.c matrix.h block_test_value matrix_test_value
	$(CC) unit_tests.c matrix.c $(TEST_CFLAGS) -o $@

$(OBJS): matrix.h

.PHONY: clean
clean: 
	rm -rf $(OBJS)
	rm -rf main
	rm -rf unit_tests

.DEFAULT_GOAL = main
