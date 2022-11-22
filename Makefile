DOCKER_CC := icx
HOST_CC := gcc
PYTHON := python3
CONTAINER_NAME := icc
OBJS := main.o matrix.o
LINKER := mold
 
COMMON_CFLAGS := -W -Wall -Wextra -flto -march=native -mtune=native -Ofast -msse4.2 -fexcess-precision=fast -ffast-math

HOST_GCC_CFLAGS := -fopenmp -funroll-all-loops -static-libgcc
DOCKER_ICX_FLAGS := -fiopenmp

TEST_CFLAGS := $(CFLAGS) -lcunit -ggdb3

block_test_value: tests/gen_block_test_value.py settings.h
	$(PYTHON) tests/gen_block_test_value.py
block_add_test_value: tests/gen_block_add_test_value.py settings.h
	$(PYTHON) tests/gen_block_add_test_value.py
matrix_test_value: tests/gen_matrix_test_value.py settings.h
	$(PYTHON) tests/gen_matrix_test_value.py
matrix_transpose_value: tests/gen_matrix_transpose.py settings.h
	$(PYTHON) tests/gen_matrix_transpose.py

host_main.o: main.c matrix.h settings.h
	$(HOST_CC) main.c $(COMMON_CFLAGS) $(HOST_GCC_CFLAGS) -c -o $@
host_matrix.o: matrix.c matrix.h settings.h
	$(HOST_CC) matrix.c $(COMMON_CFLAGS) $(HOST_GCC_CFLAGS) -c -o $@
host_main: host_main.o host_matrix.o
	$(HOST_CC) $^ $(COMMON_CFLAGS) $(HOST_GCC_CFLAGS) -o $@
host_unit_tests.o: tests/unit_tests.c matrix.h settings.h
	$(HOST_CC) tests/unit_tests.c $(COMMON_CFLAGS) $(HOST_GCC_CFLAGS) -c -o $@
host_tests: host_matrix.o host_unit_tests.o block_test_value block_add_test_value matrix_test_value matrix_transpose_value
	$(HOST_CC) host_matrix.o host_unit_tests.o $(COMMON_CFLAGS) $(HOST_GCC_CFLAGS) $(TEST_CFLAGS) -o $@

inner_docker_main.o: main.c matrix.h settings.h
	$(DOCKER_CC) main.c $(COMMON_CFLAGS) $(DOCKER_ICX_FLAGS) -c -o $@
inner_docker_matrix.o: matrix.c matrix.h settings.h
	$(DOCKER_CC) matrix.c $(COMMON_CFLAGS) $(DOCKER_ICX_FLAGS) -c -o $@
inner_docker_main: inner_docker_main.o inner_docker_matrix.o
	$(DOCKER_CC) $^ $(COMMON_CFLAGS) $(DOCKER_ICX_FLAGS) -o docker_main
inner_docker_unit_tests.o: tests/unit_tests.c matrix.h settings.h
	$(DOCKER_CC) tests/unit_tests.c $(COMMON_CFLAGS) $(DOCKER_ICX_FLAGS) -c -o $@
inner_docker_tests: inner_docker_matrix.o inner_docker_unit_tests.o block_test_value block_add_test_value matrix_test_value matrix_transpose_value
	$(DOCKER_CC) inner_docker_matrix.o inner_docker_unit_tests.o $(COMMON_CFLAGS) $(DOCKER_ICX_FLAGS) $(TEST_CFLAGS) -o docker_tests

docker_main: main.c matrix.c matrix.h settings.h
	docker exec $(CONTAINER_NAME) make -C /project inner_docker_main
docker_tests:  matrix.c matrix.h settings.h tests/unit_tests.c block_test_value block_add_test_value matrix_test_value matrix_transpose_value
	docker exec $(CONTAINER_NAME) make -C /project inner_docker_tests

.PHONY: clean
clean: 
	rm -rf *.o
	rm -rf host_main host_tests docker_main docker_tests

.PHONY: clean_gen
clean_gen: clean
	rm -rf tests/block_test_value
	rm -rf tests/block_add_test_value
	rm -rf tests/matrix_test_value
	rm -rf tests/matrix_transpose_value

.DEFAULT_GOAL = host_main
