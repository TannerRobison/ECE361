#ifndef CHECK_H
#define CHECK_H

#include <stdio.h>

#include "lock.h"

/* Checks a transition table for four kinds of problem and prints one line to
 * out for each one it finds:
 *   - an entry left out, which is all zeros and so has action A_UNDEFINED;
 *   - an entry whose next state is out of range;
 *   - a state that cannot be reached from start (breadth-first search);
 *   - a state with no way out: every event leads back to that state.
 * Entries that are left out or out of range are not real transitions, so the
 * last two checks ignore them. Returns the number of problems; 0 means the
 * table passed. */
int fsm_check(const transition_t table[N_STATES][N_EVENTS], state_t start,
	      FILE *out);

#endif
