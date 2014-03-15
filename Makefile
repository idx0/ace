CC := gcc
CFLAGS := -Wall -Wunused -Wuninitialized -Wno-format -std=c99

OBJS := \
	ace_fen.o \
	ace_display.o \
	ace_init.o \
	ace_zobrist.o \
	ace_magic.o \
	ace_move.o \
	ace_perft.o

INCLUDES :=

LIBS := \
	-lm

TARGET := ace

MAIN_OBJ := ace_main.o
TEST_OBJ := ace_test.o

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

ace_test.o: ace_test.c
	$(CC) $(CFLAGS) -D_DEBUG -c -o $@ $<

all: CFLAGS += -O3 -DNDEBUG
all: $(MAIN_OBJ) $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(MAIN_OBJ) $(LIBS)

test: $(TEST_OBJ) $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(TEST_OBJ) $(LIBS)

clean:
	rm -f *.o test ace

.PHONY: all clean test
.SECONDARY:
