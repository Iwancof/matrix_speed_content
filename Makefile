CC := gcc
OBJS := main.o matrix.o
CFLAGS := -W -Wall -Wextra

main: $(OBJS)

$(OBJS): matrix.h

.PHONY: clean
clean: 
	rm $(OBJS)
	rm main

.DEFAULT_GOAL = main
