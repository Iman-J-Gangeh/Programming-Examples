all: batch

batch: batch.o
	gcc batch.o -o batch

debug: batch.o
	gcc -Wall -Wextra -std=gnu11 -pedantic -g batch.o -o batch

batch.o: batch.c
	gcc -Wall -Wextra -std=gnu11 -pedantic -c batch.c

clean:
	rm -f batch batch.o