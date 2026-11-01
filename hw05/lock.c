#include <stdint.h>
#include <stdio.h>

#include "lock.h"

static const char code[] = "314";

const transition_t lock_table[N_STATES][N_EVENTS] = {
    [LOCKED] =
	{
	    [E_RIGHT] = {GOT1, A_NONE},
	    [E_WRONG] = {LOCKED, A_BEEP},
	    [E_CLOSE] = {LOCKED, A_NONE},
	    [E_TIMEOUT] = {LOCKED, A_NONE},
	},
    [GOT1] =
	{
	    [E_RIGHT] = {GOT2, A_NONE},
	    [E_WRONG] = {LOCKED, A_BEEP},
	    [E_CLOSE] = {GOT1, A_NONE},
	    [E_TIMEOUT] = {LOCKED, A_NONE},
	},
    [GOT2] =
	{
	    [E_RIGHT] = {OPEN, A_UNLOCK},
	    [E_WRONG] = {LOCKED, A_BEEP},
	    [E_CLOSE] = {GOT2, A_NONE},
	    [E_TIMEOUT] = {LOCKED, A_NONE},
	},
    [OPEN] =
	{
	    [E_RIGHT] = {OPEN, A_NONE},
	    [E_WRONG] = {OPEN, A_NONE},
	    [E_CLOSE] = {LOCKED, A_LOCK},
	    [E_TIMEOUT] = {LOCKED, A_LOCK},
	},
};

state_t lock_step(state_t s, event_t e, action_t *action)
{
	*action = lock_table[s][e].action;
	return lock_table[s][e].next;
}

int lock_event(state_t s, char c)
{
	switch (c) {
	case 'c':
		return E_CLOSE;
	case 't':
		return E_TIMEOUT;
	}

	if (c < '0' || c > '9') {
		return -1;
	}

	// c is a digit: right only if it is the next digit of the code
	if (s < OPEN && c == code[s]) {
		return E_RIGHT;
	}
	return E_WRONG;
}

static const char *state_names[N_STATES] = {
	[LOCKED] = "LOCKED",
	[GOT1] = "GOT1",
	[GOT2] = "GOT2",
	[OPEN] = "OPEN",
};

const char *state_name(state_t s)
{
	if ((unsigned)s >= N_STATES) {
		return "?";
	}
	return state_names[s];
}

static const char *event_names[N_EVENTS] = {
	[E_RIGHT] = "RIGHT",
	[E_WRONG] = "WRONG",
	[E_CLOSE] = "CLOSE",
	[E_TIMEOUT] = "TIMEOUT",
};

const char *event_name(event_t e)
{
	if ((unsigned)e >= N_EVENTS) {
		return "?";
	}
	return event_names[e];
}

static const char *action_names[N_ACTIONS] = {
	[A_UNDEFINED] = "UNDEFINED",
	[A_NONE] = "NONE",
	[A_UNLOCK] = "UNLOCK",
	[A_LOCK] = "LOCK",
	[A_BEEP] = "BEEP",
};

const char *action_name(action_t a)
{
	if ((unsigned)a >= N_ACTIONS) {
		return "?";
	}
	return action_names[a];
}
