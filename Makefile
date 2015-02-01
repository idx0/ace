CC := gcc
CFLAGS := -Wall -Wunused -Wno-format -Wno-unused-function 

OBJS := \
	ace_util.o \
	ace_fen.o \
	ace_display.o \
	ace_init.o \
	ace_const.o \
	ace_zobrist.o \
	ace_magic.o \
	ace_move.o \
	ace_input.o \
	ace_pgn.o \
	ace_str.o \
	ace_evaluate.o \
	ace_search.o \
	ace_hash.o \
	ace_thread.o \
	ace_uci.o \
	ace_perft.o

INCLUDES :=

LIBS := \
	-lm \
	-lpthread

TARGET := ace

MAIN_OBJ := ace_main.o
TEST_OBJ := ace_test.o

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

ace_test.o: ace_test.c
	$(CC) $(CFLAGS) -D_DEBUG -c -o $@ $<

all: CFLAGS += -O3 -Wuninitialized -DNDEBUG
all: main

main: $(MAIN_OBJ) $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(MAIN_OBJ) $(LIBS)

debug: CFLAGS += -ggdb
debug: main

test: CFLAGS += -ggdb
test: $(TEST_OBJ) $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(TEST_OBJ) $(LIBS)

clean:
	rm -f *.o test ace

.PHONY: all clean test
.SECONDARY:
