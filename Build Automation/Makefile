# Builds targets for converting characters between DOS and UNIX.
# CSC 357, Assignment 1
# Given code, Spring '25
# NOTE: Do not alter this file.

all: tounix todos

tounix: tounix.o chars.o
	gcc -o tounix tounix.o chars.o

todos: todos.o chars.o
	gcc -o todos todos.o chars.o

tounix.o: tounix.c
	gcc -Wall -Wextra -Werror -ansi -pedantic -c tounix.c

todos.o: todos.c
	gcc -Wall -Wextra -Werror -ansi -pedantic -c todos.c

chars.o: chars.c chars.h
	gcc -Wall -Wextra -Werror -ansi -pedantic -c chars.c

test:
	gcc -Wall -Wextra -Werror -ansi -pedantic chars.c chartests.c
	./a.out
