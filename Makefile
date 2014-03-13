CC := gcc
CFLAGS := -Wall

all:
	$(CC) $(CFLAGS) ace_fen.c ace_main.c -o ace

test:
	$(CC) $(CFLAGS) ace_fen.c ace_test.c -o test

clean:
	rm -f *.o test ace