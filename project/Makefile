CC     = clang
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic
LDLIB  = -lm
LDFILEC= main.c iom361_r4.c sensor.c
LFFILEH= iom361_r4.h sensor.h

incubator: $(LDFILEC) $(LDFILEH)
	$(CC) $(CFLAGS) $(LDFILEC) -o incubator $(LDLIB)

run: incubator
	./incubator

clean:
	rm -f incubator

.PHONY: run clean
