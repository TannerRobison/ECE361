// check.c: fsm_check, which finds problems in a transition table such as
// lock_table. See check.h for what it checks.
//
// ECE 361, Fall 2026, HW5 bonus. Tanner Robison.

#include <stdio.h>

#include "check.h"
#include "lock.h"

// 1 if t is a real transition: filled in, with a next state in range.
static int is_real(const transition_t *t)
{
	return t->action != A_UNDEFINED && (unsigned)t->next < N_STATES;
}

int fsm_check(const transition_t table[N_STATES][N_EVENTS], state_t start,
	      FILE *out)
{
	int problems = 0;

	// Every entry filled in, and every next state in range.
	for (state_t s = 0; s < N_STATES; s++) {
		for (event_t e = 0; e < N_EVENTS; e++) {
			const transition_t *t = &table[s][e];
			if (t->action == A_UNDEFINED) {
				fprintf(out, "left out: %s + %s has action A_UNDEFINED\n",
					state_name(s), event_name(e));
				problems++;
			}
			if ((unsigned)t->next >= N_STATES) {
				fprintf(out, "out of range: %s + %s goes to state %d\n",
					state_name(s), event_name(e), (int)t->next);
				problems++;
			}
		}
	}

	// Every state reachable from start. Breadth-first search: queue holds
	// the states reached but not yet looked at, from queue[head] up to
	// queue[tail - 1]. Each state is added at most once, so N_STATES slots
	// are enough.
	if ((unsigned)start >= N_STATES) {
		fprintf(out, "out of range: start state %d\n", (int)start);
		return problems + 1;
	}

	int reached[N_STATES] = {0};
	state_t queue[N_STATES];
	int head = 0;
	int tail = 0;

	reached[start] = 1;
	queue[tail++] = start;
	while (head < tail) {
		state_t s = queue[head++];
		for (event_t e = 0; e < N_EVENTS; e++) {
			const transition_t *t = &table[s][e];
			if (is_real(t) && !reached[t->next]) {
				reached[t->next] = 1;
				queue[tail++] = t->next;
			}
		}
	}

	for (state_t s = 0; s < N_STATES; s++) {
		if (!reached[s]) {
			fprintf(out, "unreachable: %s cannot be reached from %s\n",
				state_name(s), state_name(start));
			problems++;
		}
	}

	// Every state has a way out: some event leads to a different state.
	for (state_t s = 0; s < N_STATES; s++) {
		int way_out = 0;
		for (event_t e = 0; e < N_EVENTS; e++) {
			const transition_t *t = &table[s][e];
			if (is_real(t) && t->next != s) {
				way_out = 1;
			}
		}
		if (!way_out) {
			fprintf(out, "no way out: every event leaves %s in %s\n",
				state_name(s), state_name(s));
			problems++;
		}
	}

	return problems;
}
