CC     = clang
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic
LDLIB  = -lm
LDFILEC= iom361_r4.c sensor.c fsm.c store.c
LDFILEH= iom361_r4.h sensor.h fsm.h store.h

# tests run under the sanitizers; -fno-sanitize-recover makes undefined
#  behavior stop the test instead of printing a warning and going on
TESTFLAGS = $(CFLAGS) -g -I. -fsanitize=address,undefined -fno-sanitize-recover=all
TESTS     = tests/test_store

incubator: main.c $(LDFILEC) $(LDFILEH)
	$(CC) $(CFLAGS) main.c $(LDFILEC) -o incubator $(LDLIB)

run: incubator
	./incubator

tests/test_store: tests/test_store.c store.c store.h fsm.h sensor.h
	$(CC) $(TESTFLAGS) tests/test_store.c store.c -o $@ $(LDLIB)

# runs every test, then prints one line: how many checks passed and failed.
#  a test that crashes or trips a sanitizer without printing a FAIL line
#  counts as one failure
test: $(TESTS)
	@rm -f tests/results.txt
	@for t in $(TESTS); do \
		./$$t > tests/out.txt 2>&1; rc=$$?; \
		cat tests/out.txt; cat tests/out.txt >> tests/results.txt; \
		if [ $$rc -ne 0 ] && ! grep -q '^FAIL' tests/out.txt; then \
			echo "FAIL: $$t exited with status $$rc" | tee -a tests/results.txt; \
		fi; \
	done; \
	echo "$$(grep -c '^PASS' tests/results.txt) passed, $$(grep -c '^FAIL' tests/results.txt) failed"

clean:
	rm -f incubator *.o $(TESTS) tests/out.txt tests/results.txt

.PHONY: run test clean
