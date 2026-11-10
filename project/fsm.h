#ifndef FSM_H
#define FSM_H

typedef enum { IDLE, WARMUP, INCUBATE, TURN, LOCKDOWN, HATCH, N_STATES} state_t;

//listed in order or priority highest -> lowest
typedef enum {
	E_EGGS_IN, 
	E_LOCKDOWN_DAY,
	E_HATCH_SEEN,
	E_TURN_DONE,
	E_TURN_DUE,
	E_TEMP_REACHED,
	E_NONE,		/* nothing fired this tick */
	N_EVENTS
} event_t;

typedef enum {
	A_UNDEFINED,	/* 0: a cell left out of the table shows up as this */
	A_NONE,
	A_START_TIMER,	/* start the 240-tick turn count */
	A_START_TURN,	/* TURNER on, remember the tick TURN began */
	A_STOP_TURN,	/* TURNER off, restart the 240-tick count */
	N_ACTIONS
} action_t;

typedef struct {
	state_t next;
	action_t action;
} transition_t;

// transition table: transition_table[s][e] is the next state and action to take
extern const transition_t transition_table[N_STATES][N_EVENTS];

/* One step of the state machine: looks up transition_table[s][e], stores its
 * action in *action, and returns the next state. If s or e is out of range,
 * stores A_UNDEFINED and returns s unchanged. action may be NULL. */
state_t fsm_step(state_t s, event_t e, action_t *action);

#endif
