CFLAGS= -Wall -std=gnu11 -g -Wextra -Werror -Wpedantic -Wmissing-declarations -Wmissing-prototypes -Wold-style-definition

prog: mmake.o parser.o
	gcc -o mmake parser.o mmake.o

parser.o: parser.c parser.h
	gcc -c $(CFLAGS) parser.c

mmake.o: mmake.c
	gcc -c $(CFLAGS) mmake.c

