# Builds targets for sorting arrays in parallel.
# CSC 357, Assignment 7
# Given code, Winter '24

CFLAGS = -Wall -Wextra -std=gnu11 -pedantic

all: fsort.a

debug: CFLAGS += -g
debug: clean all

# TODO: Add additional object files if necessary.
fsort.a: fsort.o
	ar -rcs fsort.a fsort.o

%.o: %.c
	gcc $(CFLAGS) -c $<

test: debug fsorttests.c
	gcc $(CFLAGS) fsorttests.c fsort.a
	valgrind --leak-check=full --show-leak-kinds=all ./a.out

clean:
	rm -f *.o
