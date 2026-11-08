# Makefile: ECE 361, Fall 2026, HW6. Tanner Robison.
#
#   make         builds ./replay
#   make test    builds and runs the unit tests, then the regression test
#   make clean   removes everything the build produced

CC     = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -I.
LDLIBS = -lm

PROG = replay
MODS = sensor.o panel.o scenario.o iom361_r4.o
OBJS = main.o $(MODS)

TEST_PROG = tests/test_hw06
TEST_OBJS = tests/test_hw06.o
TEST_LINK = tests/test_hw06.o $(MODS)

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDLIBS)

$(TEST_PROG): $(TEST_LINK)
	$(CC) $(CFLAGS) -o $@ $(TEST_LINK) $(LDLIBS)

# Each .o is compiled by make's built-in rule, $(CC) $(CPPFLAGS) $(CFLAGS) -c.
# These lines add the headers each one includes, so that editing a header
# rebuilds exactly the files that read it.
main.o:             main.c      sensor.h panel.h scenario.h iom361_r4.h
sensor.o:           sensor.c    sensor.h iom361_r4.h
panel.o:            panel.c     panel.h  iom361_r4.h
scenario.o:         scenario.c  scenario.h
iom361_r4.o:        iom361_r4.c iom361_r4.h
tests/test_hw06.o:  tests/test_hw06.c sensor.h panel.h scenario.h iom361_r4.h

test: $(PROG) $(TEST_PROG)
	./$(TEST_PROG)
	sh regress.sh

clean:
	rm -f $(PROG) $(OBJS) $(TEST_PROG) $(TEST_OBJS)

.PHONY: all test clean
