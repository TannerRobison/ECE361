# Makefile: ECE 361, Fall 2026, HW5. Tanner Robison.
#
#   make         builds lock
#   make test    builds and runs the unit tests
#   make bonus   builds and runs the tests for fsm_check, the table checker
#   make clean   removes everything the build produced

CC     = gcc
CFLAGS = -std=c11 -Wall -Wextra -I.

PROG = lock
OBJS = main.o lock.o

TEST_PROG = tests/test_lock
TEST_OBJS = tests/test_lock.o
TEST_LINK = tests/test_lock.o lock.o

BONUS_PROG = tests/test_check
BONUS_OBJS = tests/test_check.o check.o
BONUS_LINK = tests/test_check.o check.o lock.o

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

$(TEST_PROG): $(TEST_LINK)
	$(CC) $(CFLAGS) -o $@ $(TEST_LINK)

$(BONUS_PROG): $(BONUS_LINK)
	$(CC) $(CFLAGS) -o $@ $(BONUS_LINK)

# Each .o is compiled by make's built-in rule, $(CC) $(CPPFLAGS) $(CFLAGS) -c.
# These lines add the headers each one includes, so that editing a header
# rebuilds exactly the files that read it: change lock.h and all five objects
# are out of date; change check.h and only check.o, tests/test_check.o and
# the checker's test are.
main.o:             main.c             lock.h
lock.o:             lock.c             lock.h
check.o:            check.c            check.h lock.h
tests/test_lock.o:  tests/test_lock.c  lock.h           # it includes ../lock.h
tests/test_check.o: tests/test_check.c check.h lock.h   # and ../check.h

test: $(TEST_PROG)
	./$(TEST_PROG)

bonus: $(BONUS_PROG)
	./$(BONUS_PROG)

clean:
	rm -f $(PROG) $(OBJS) $(TEST_PROG) $(TEST_OBJS) $(BONUS_PROG) $(BONUS_OBJS)

.PHONY: all test bonus clean
