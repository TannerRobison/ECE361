# Makefile: ECE 361, Fall 2026, HW4. Tanner Robison.
#
#   make         builds events
#   make test    builds and runs the unit tests
#   make clean   removes everything the build produced

CC     = gcc
CFLAGS = -std=c11 -Wall -Wextra -I.

PROG = events
OBJS = events.o tally.o

TEST_PROG = tests/test_tally
TEST_OBJS = tests/test_tally.o
TEST_LINK = tests/test_tally.o tally.o

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

$(TEST_PROG): $(TEST_LINK)
	$(CC) $(CFLAGS) -o $@ $(TEST_LINK)

# Each .o is compiled by make's built-in rule, $(CC) $(CPPFLAGS) $(CFLAGS) -c.
# These lines add the headers each one includes, so that editing a header
# rebuilds exactly the files that read it: change tally.h and all three
# objects are out of date; change tally.c and only tally.o and the links are.
events.o:           events.c           tally.h
tally.o:            tally.c            tally.h
tests/test_tally.o: tests/test_tally.c tally.h   # it includes ../tally.h

test: $(TEST_PROG)
	./$(TEST_PROG)

clean:
	rm -f $(PROG) $(OBJS) $(TEST_PROG) $(TEST_OBJS)

.PHONY: all test clean
