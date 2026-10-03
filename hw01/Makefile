CC = clang
CFLAGS = -std=c11 -Wall -Wextra -g -I.

# Library objects.
OBJS = bits.o status.o

TEST_BIN = tests/test_bits
TEST_OBJ = tests/test_bits.o

all: $(OBJS)

# Each .c compiles to its own .o. Matches tests/test_bits.c too.
%.o: %.c bits.h
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_BIN): $(OBJS) $(TEST_OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(TEST_OBJ)

test: $(TEST_BIN)
	./$(TEST_BIN)

clean:
	rm -f $(OBJS) $(TEST_OBJ) $(TEST_BIN)

.PHONY: all test clean
