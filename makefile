CC = gcc
CFLAGS = -g -std=c99 -Wall -Wextra -Wpedantic

.PHONY = test clean

test: cfmt.h test.c
	$(CC) $(CFLAGS) test.c
	./a.out

clean:
	rm -f a.out
