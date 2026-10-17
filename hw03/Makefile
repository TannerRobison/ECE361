# Makefile: ECE 361, Fall 2026, HW3. Tanner Robison.
#
#   make         builds ptr.o and tests/test_ptr
#   make test    builds and runs the unit tests
#   make clean   removes everything the build produced

CC     = gcc
CFLAGS = -std=c11 -Wall -Wextra -I.

OBJS = ptr.o

TEST_PROG = tests/test_ptr
TEST_OBJS = tests/test_ptr.o
TEST_LINK = tests/test_ptr.o ptr.o

all: $(OBJS) $(TEST_PROG)

$(TEST_PROG): $(TEST_LINK)
	$(CC) $(CFLAGS) -o $@ $(TEST_LINK)

# Each .o is compiled by make's built-in rule, $(CC) $(CPPFLAGS) $(CFLAGS) -c.
# These lines add the headers each one includes, so that editing a header
# rebuilds exactly the files that read it: change ptr.h and both ptr.o and
# tests/test_ptr.o are out of date; change ptr.c and only ptr.o and the link
# are.
ptr.o:            ptr.c            ptr.h
tests/test_ptr.o: tests/test_ptr.c ptr.h   # it includes ../ptr.h

test: $(TEST_PROG)
	./$(TEST_PROG)

clean:
	rm -f $(OBJS) $(TEST_PROG) $(TEST_OBJS)

.PHONY: all test clean
