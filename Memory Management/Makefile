all: freq

freq: dict.o freq.o
	gcc dict.o freq.o -o freq

dicttests: dict.o dicttests.o
	gcc dict.o dicttests.o -o dicttests

debug: freq
	gcc -Wall -Wextra -ansi -pedantic -g dict.c freq.c -o freq

dict.o: dict.c
	gcc -Wall -Wextra -ansi -pedantic -c dict.c

freq.o: freq.c
	gcc -Wall -Wextra -ansi -pedantic -c freq.c

dicttests.o: dicttests.c
	gcc -Wall -Wextra -ansi -pedantic -g -c dicttests.c
