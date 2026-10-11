# Makefile: ECE 361, Fall 2026, HW2. Tanner Robison.
#
#   make         builds ./readings
#   make test    builds and runs the unit tests, then the regression test
#   make clean   removes everything the build produced

CC     = gcc
CFLAGS = -std=c11 -Wall -Wextra -I.

PROG = readings
OBJS = main.o reading.o stats.o histogram.o

TEST_PROG = tests/test_stats
TEST_OBJS = tests/test_stats.o
TEST_LINK = tests/test_stats.o stats.o

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

$(TEST_PROG): $(TEST_LINK)
	$(CC) $(CFLAGS) -o $@ $(TEST_LINK)

# Each .o is compiled by make's built-in rule, $(CC) $(CPPFLAGS) $(CFLAGS) -c.
# These lines add the headers each one includes, so that editing a header
# rebuilds exactly the files that read it: change stats.h and main.o, stats.o
# and tests/test_stats.o are all out of date; change histogram.c and only
# histogram.o and the link are.
main.o:             main.c      reading.h stats.h histogram.h
reading.o:          reading.c   reading.h
stats.o:            stats.c     stats.h
histogram.o:        histogram.c histogram.h
tests/test_stats.o: tests/test_stats.c stats.h   # it includes ../stats.h

test: $(PROG) $(TEST_PROG)
	./$(TEST_PROG)
	sh regress.sh

clean:
	rm -f $(PROG) $(OBJS) $(TEST_PROG) $(TEST_OBJS)

.PHONY: all test clean
