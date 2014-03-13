CC := gcc
CFLAGS := -Wall

all:
	$(CC) $(CFLAGS) ace_fen.c ace_display.c ace_init.c ace_main.c -o ace -lm

test:
	$(CC) $(CFLAGS) -D_DEBUG ace_fen.c ace_display.c ace_init.c ace_test.c -o test -lm

clean:
	rm -f *.o test ace
