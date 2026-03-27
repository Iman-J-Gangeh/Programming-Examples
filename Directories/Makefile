all: tree

tree: tree.o
	gcc tree.o -o tree

debug: tree
	gcc -Wall -Wextra -ansi -pedantic -g  tree.c -o tree

tree.o: tree.c
	gcc -Wall -Wextra -ansi -pedantic -c tree.c

