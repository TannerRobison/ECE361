#ifndef LOCK_H
#define LOCK_H

typedef enum { LOCKED, GOT1, GOT2, OPEN, N_STATES } state_t;
typedef enum { E_RIGHT, E_WRONG, E_CLOSE, E_TIMEOUT, N_EVENTS } event_t;

typedef enum {
	A_UNDEFINED,
	A_NONE,
	A_UNLOCK,
	A_LOCK,
	A_BEEP,
	N_ACTIONS
} action_t;

typedef struct {
	state_t next;
	action_t action;
} transition_t;

/* The transition table: lock_table[s][e] is the next state, and the action to
 * take, when event e happens in state s. All 16 entries are filled in. */
extern const transition_t lock_table[N_STATES][N_EVENTS];

/* One step of the lock: looks up lock_table[s][e], stores its action in
 * *action, and returns the next state. s and e must be in range. */
state_t lock_step(state_t s, event_t e, action_t *action);

/* Turns the keypad character c into an event for state s. A digit is E_RIGHT
 * if it is the next digit of the code 3-1-4 and E_WRONG otherwise (so every
 * digit is E_WRONG while OPEN). 'c' (door closed) is E_CLOSE and 't' (timer ran
 * out) is E_TIMEOUT. Returns -1 for any other character. */
int lock_event(state_t s, char c);

/* The name of state s, such as "GOT1"; "?" if s is out of range. */
const char *state_name(state_t s);

/* The name of event e without its E_ prefix, such as "RIGHT"; "?" if out of
 * range. */
const char *event_name(event_t e);

/* The name of action a without its A_ prefix, such as "UNLOCK"; "?" if out of
 * range. */
const char *action_name(action_t a);

#endif
