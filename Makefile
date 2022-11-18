DOCKER_CC := icx
HOST_CC := gcc
PYTHON := python3
CONTAINER_NAME := icc
OBJS := main.o matrix.o
LINKER := mold

COMMON_CFLAGS := -W -Wall -Wextra -O3 

HOST_GCC_CFLAGS := -fopenmp
DOCKER_ICX_FLAGS := -fiopenmp

TEST_CFLAGS := $(CFLAGS) -lcunit -ggdb3

block_test_value:
	$(PYTHON) ./gen_block_test_value.py
matrix_test_value:
	$(PYTHON) ./gen_matrix_test_value.py

host_main.o: main.c matrix.h
	$(HOST_CC) main.c $(COMMON_CFLAGS) $(HOST_GCC_CFLAGS) -c -o $@
host_matrix.o: matrix.c matrix.h
	$(HOST_CC) matrix.c $(COMMON_CFLAGS) $(HOST_GCC_CFLAGS) -c -o $@
host_main: host_main.o host_matrix.o
	$(HOST_CC) $^ $(COMMON_CFLAGS) $(HOST_GCC_CFLAGS) -o $@
host_unit_tests.o: host_unit_tests.c
	$(HOST_CC) $^ $(COMMON_CFLAGS) $(HOST_GCC_CFLAGS) -c -o $@
host_tests: host_main.o host_matrix.o host_unit_tests.o block_test_value matrix_test_value
	$(HOST_CC) $^ $(COMMON_CFLAGS) $(HOST_GCC_CFLAGS) $(TEST_CFLAGS) -o $@

inner_docker_main.o: main.c matrix.h
	$(DOCKER_CC) main.c $(COMMON_CFLAGS) $(DOCKER_ICX_FLAGS) -c -o $@
inner_docker_matrix.o: matrix.c matrix.h
	$(DOCKER_CC) matrix.c $(COMMON_CFLAGS) $(DOCKER_ICX_FLAGS) -c -o $@
inner_docker_main: inner_docker_main.o inner_docker_matrix.o
	$(DOCKER_CC) $^ $(COMMON_CFLAGS) $(DOCKER_ICX_FLAGS) -o docker_main
inner_docker_unit_tests.o: unit_tests.c
	$(DOCKER_CC) $^ $(COMMON_CFLAGS) $(DOCKER_ICX_FLAGS) -c -o $@
inner_docker_tests: inner_docker_matrix.o inner_docker_unit_tests.o
	$(DOCKER_CC) $^ $(COMMON_CFLAGS) $(DOCKER_ICX_FLAGS) $(TEST_CFLAGS) -o docker_tests

docker_main: main.c matrix.c matrix.h
	docker exec $(CONTAINER_NAME) make -C /project inner_docker_main
docker_tests:  matrix.c matrix.h unit_tests.c block_test_value matrix_test_value
	docker exec $(CONTAINER_NAME) make -C /project inner_docker_tests

.PHONY: clean
clean: 
	rm -rf *.o
	rm -rf host_main host_tests docker_main docker_tests

.PHONY: clean_gen
clean_gen: clean
	rm -rf block_test_value
	rm -rf matrix_test_value

.DEFAULT_GOAL = host_main
