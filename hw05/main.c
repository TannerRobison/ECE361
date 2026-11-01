// main.c: runs the 3-1-4 combination lock on the keys typed on standard input.
//
//   ./lock < data/codes.txt
//
// The lock starts in LOCKED. Digits are keypad presses, c means the door
// closed and t means the 5-second timer ran out; every other character
// (newlines, spaces) is skipped. Each event prints one line, such as
// "GOT2 4 RIGHT -> OPEN UNLOCK", and the last line is "final: STATE".
//
// ECE 361, Fall 2026, HW5. Tanner Robison.

#include <stdio.h>

#include "lock.h"

int main(void)
{
	state_t s = LOCKED;
	int c;

	while ((c = getchar()) != EOF) {
		int e = lock_event(s, (char)c);
		if (e < 0) {
			continue;
		}

		action_t a;
		state_t next = lock_step(s, (event_t)e, &a);
		printf("%s %c %s -> %s %s\n", state_name(s), c,
		       event_name((event_t)e), state_name(next), action_name(a));
		s = next;
	}

	printf("final: %s\n", state_name(s));
	return 0;
}
