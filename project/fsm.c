#include <stddef.h>
#include <stdint.h>

#include "fsm.h"

/* Every cell is filled in; a cell left out would be {IDLE, A_UNDEFINED}. */
const transition_t transition_table[N_STATES][N_EVENTS] = {
	[IDLE] = {
		[E_EGGS_IN]      = {WARMUP,   A_NONE},
		[E_LOCKDOWN_DAY] = {IDLE,     A_NONE},
		[E_HATCH_SEEN]   = {IDLE,     A_NONE},
		[E_TURN_DONE]    = {IDLE,     A_NONE},
		[E_TURN_DUE]     = {IDLE,     A_NONE},
		[E_TEMP_REACHED] = {IDLE,     A_NONE},
		[E_NONE]         = {IDLE,     A_NONE},
	},
	[WARMUP] = {
		[E_EGGS_IN]      = {WARMUP,   A_NONE},
		[E_LOCKDOWN_DAY] = {WARMUP,   A_NONE},
		[E_HATCH_SEEN]   = {WARMUP,   A_NONE},
		[E_TURN_DONE]    = {WARMUP,   A_NONE},
		[E_TURN_DUE]     = {WARMUP,   A_NONE},
		[E_TEMP_REACHED] = {INCUBATE, A_START_TIMER},
		[E_NONE]         = {WARMUP,   A_NONE},
	},
	[INCUBATE] = {
		[E_EGGS_IN]      = {INCUBATE, A_NONE},
		[E_LOCKDOWN_DAY] = {LOCKDOWN, A_NONE},
		[E_HATCH_SEEN]   = {HATCH,    A_NONE},
		[E_TURN_DONE]    = {INCUBATE, A_NONE},
		[E_TURN_DUE]     = {TURN,     A_START_TURN},
		[E_TEMP_REACHED] = {INCUBATE, A_NONE},
		[E_NONE]         = {INCUBATE, A_NONE},
	},
	[TURN] = {
		[E_EGGS_IN]      = {TURN,     A_NONE},
		[E_LOCKDOWN_DAY] = {LOCKDOWN, A_STOP_TURN},
		[E_HATCH_SEEN]   = {HATCH,    A_STOP_TURN},
		[E_TURN_DONE]    = {INCUBATE, A_STOP_TURN},
		[E_TURN_DUE]     = {TURN,     A_NONE},
		[E_TEMP_REACHED] = {TURN,     A_NONE},
		[E_NONE]         = {TURN,     A_NONE},
	},
	[LOCKDOWN] = {
		[E_EGGS_IN]      = {LOCKDOWN, A_NONE},
		[E_LOCKDOWN_DAY] = {LOCKDOWN, A_NONE},
		[E_HATCH_SEEN]   = {HATCH,    A_NONE},
		[E_TURN_DONE]    = {LOCKDOWN, A_NONE},
		[E_TURN_DUE]     = {LOCKDOWN, A_NONE},
		[E_TEMP_REACHED] = {LOCKDOWN, A_NONE},
		[E_NONE]         = {LOCKDOWN, A_NONE},
	},
	[HATCH] = {
		[E_EGGS_IN]      = {IDLE,     A_NONE},
		[E_LOCKDOWN_DAY] = {HATCH,    A_NONE},
		[E_HATCH_SEEN]   = {HATCH,    A_NONE},
		[E_TURN_DONE]    = {HATCH,    A_NONE},
		[E_TURN_DUE]     = {HATCH,    A_NONE},
		[E_TEMP_REACHED] = {HATCH,    A_NONE},
		[E_NONE]         = {HATCH,    A_NONE},
	},
};

state_t fsm_step(state_t s, event_t e, action_t *action)
{
	/* A state or event out of range is not in the table: stay put. */
	if (s < 0 || s >= N_STATES || e < 0 || e >= N_EVENTS) {
		if (action != NULL)
			*action = A_UNDEFINED;
		return s;
	}

	transition_t t = transition_table[s][e];
	if (action != NULL)
		*action = t.action;
	return t.next;
}
