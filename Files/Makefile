all: lzw

lzw: dict.o lzw.o
	gcc dict.o lzw.o -o lzw

debug: lzw
	gcc -Wall -Wextra -ansi -pedantic -g dict.c lzw.c -o lzw

dict.o: dict.c
	gcc -Wall -Wextra -ansi -pedantic -c dict.c

lzw.o: lzw.c
	gcc -Wall -Wextra -ansi -pedantic -c lzw.c

