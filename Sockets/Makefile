CFLAGS = -Wall -Wextra -std=gnu11 -pedantic

all: gossip

debug: CFLAGS += -g
debug: clean all

gossip: gossip.o
	gcc $(CFLAGS) -o gossip gossip.o

%.o: %.c
	gcc $(CFLAGS) -c $<

test: debug
	valgrind --leak-check=full --show-leak-kinds=all ./gossip 2187

clean:
	rm -f *.o gossip