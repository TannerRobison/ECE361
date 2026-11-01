#include <stdio.h>
#include <string.h>

#include "../lock.h"

#define COUNT(a) (sizeof(a) / sizeof((a)[0]))

static int passed;
static int failed;

/* Print PASS or FAIL for one test and count the result. */
static void check(int ok, const char *name)
{
	printf("%s  %s\n", ok ? "PASS" : "FAIL", name);
	if (ok) {
		passed++;
	} else {
		failed++;
	}
}

/* Feed keys to the lock starting from LOCKED, skipping characters that are
 * not events (like main.c does). Returns the final state and the last action. */
static state_t run(const char *keys, action_t *last)
{
	state_t s = LOCKED;
	*last = A_NONE;
	for (const char *p = keys; *p != '\0'; p++) {
		int e = lock_event(s, *p);
		if (e < 0) {
			continue;
		}
		s = lock_step(s, (event_t)e, last);
	}
	return s;
}

/* Every entry of lock_table, written out from the lock's rules. */
static const struct {
	state_t s;
	event_t e;
	state_t next;
	action_t action;
	const char *label;
} table_cases[] = {
	{LOCKED, E_RIGHT, GOT1, A_NONE, "LOCKED + RIGHT   -> GOT1, NONE"},
	{LOCKED, E_WRONG, LOCKED, A_BEEP, "LOCKED + WRONG   -> LOCKED, BEEP"},
	{LOCKED, E_CLOSE, LOCKED, A_NONE, "LOCKED + CLOSE   -> LOCKED, NONE"},
	{LOCKED, E_TIMEOUT, LOCKED, A_NONE, "LOCKED + TIMEOUT -> LOCKED, NONE"},
	{GOT1, E_RIGHT, GOT2, A_NONE, "GOT1 + RIGHT     -> GOT2, NONE"},
	{GOT1, E_WRONG, LOCKED, A_BEEP, "GOT1 + WRONG     -> LOCKED, BEEP"},
	{GOT1, E_CLOSE, GOT1, A_NONE, "GOT1 + CLOSE     -> GOT1, NONE"},
	{GOT1, E_TIMEOUT, LOCKED, A_NONE, "GOT1 + TIMEOUT   -> LOCKED, NONE"},
	{GOT2, E_RIGHT, OPEN, A_UNLOCK, "GOT2 + RIGHT     -> OPEN, UNLOCK"},
	{GOT2, E_WRONG, LOCKED, A_BEEP, "GOT2 + WRONG     -> LOCKED, BEEP"},
	{GOT2, E_CLOSE, GOT2, A_NONE, "GOT2 + CLOSE     -> GOT2, NONE"},
	{GOT2, E_TIMEOUT, LOCKED, A_NONE, "GOT2 + TIMEOUT   -> LOCKED, NONE"},
	{OPEN, E_RIGHT, OPEN, A_NONE, "OPEN + RIGHT     -> OPEN, NONE"},
	{OPEN, E_WRONG, OPEN, A_NONE, "OPEN + WRONG     -> OPEN, NONE"},
	{OPEN, E_CLOSE, LOCKED, A_LOCK, "OPEN + CLOSE     -> LOCKED, LOCK"},
	{OPEN, E_TIMEOUT, LOCKED, A_LOCK, "OPEN + TIMEOUT   -> LOCKED, LOCK"},
};

/* lock_event: a right and a wrong digit in each state, digits while open,
 * 'c' and 't' in every state, and characters that are not events. */
static const struct {
	state_t s;
	char c;
	int want;
	const char *label;
} event_cases[] = {
	{LOCKED, '3', E_RIGHT, "LOCKED '3'  -> RIGHT"},
	{LOCKED, '5', E_WRONG, "LOCKED '5'  -> WRONG"},
	{GOT1, '1', E_RIGHT, "GOT1   '1'  -> RIGHT"},
	{GOT1, '3', E_WRONG, "GOT1   '3'  -> WRONG"},
	{GOT2, '4', E_RIGHT, "GOT2   '4'  -> RIGHT"},
	{GOT2, '1', E_WRONG, "GOT2   '1'  -> WRONG"},
	{OPEN, '3', E_WRONG, "OPEN   '3'  -> WRONG"},
	{OPEN, '4', E_WRONG, "OPEN   '4'  -> WRONG"},
	{LOCKED, 'c', E_CLOSE, "LOCKED 'c'  -> CLOSE"},
	{GOT1, 'c', E_CLOSE, "GOT1   'c'  -> CLOSE"},
	{GOT2, 'c', E_CLOSE, "GOT2   'c'  -> CLOSE"},
	{OPEN, 'c', E_CLOSE, "OPEN   'c'  -> CLOSE"},
	{LOCKED, 't', E_TIMEOUT, "LOCKED 't'  -> TIMEOUT"},
	{GOT1, 't', E_TIMEOUT, "GOT1   't'  -> TIMEOUT"},
	{GOT2, 't', E_TIMEOUT, "GOT2   't'  -> TIMEOUT"},
	{OPEN, 't', E_TIMEOUT, "OPEN   't'  -> TIMEOUT"},
	{LOCKED, 'x', -1, "LOCKED 'x'  -> -1"},
	{GOT1, ' ', -1, "GOT1   ' '  -> -1"},
	{GOT2, '\n', -1, "GOT2   '\\n' -> -1"},
	{OPEN, 'C', -1, "OPEN   'C'  -> -1"},
	{LOCKED, 'T', -1, "LOCKED 'T'  -> -1"},
};

/* Whole key sequences, run from LOCKED. */
static const struct {
	const char *keys;
	state_t want;
	action_t last;
	const char *label;
} seq_cases[] = {
	{"314", OPEN, A_UNLOCK, "3 1 4   opens (UNLOCK)"},
	{"3314", LOCKED, A_BEEP, "3 3 1 4 does not open"},
	{"31t4", LOCKED, A_BEEP, "3 1 t 4 does not open"},
	{"314c", LOCKED, A_LOCK, "3 1 4 c locks again (LOCK)"},
};

/* Check that a name function returned want; call is how it was called. */
static void check_name(const char *got, const char *want, const char *call)
{
	char label[80];
	snprintf(label, sizeof label, "%-24s is \"%s\"", call, want);
	check(strcmp(got, want) == 0, label);
}

int main(void)
{
	puts("lock_step: all 16 table entries");
	for (size_t i = 0; i < COUNT(table_cases); i++) {
		action_t a = A_UNDEFINED;
		state_t next = lock_step(table_cases[i].s, table_cases[i].e, &a);
		check(next == table_cases[i].next && a == table_cases[i].action,
		      table_cases[i].label);
	}

	puts("\nlock_event");
	for (size_t i = 0; i < COUNT(event_cases); i++) {
		int e = lock_event(event_cases[i].s, event_cases[i].c);
		check(e == event_cases[i].want, event_cases[i].label);
	}

	puts("\nsequences");
	for (size_t i = 0; i < COUNT(seq_cases); i++) {
		action_t last;
		state_t s = run(seq_cases[i].keys, &last);
		check(s == seq_cases[i].want && last == seq_cases[i].last,
		      seq_cases[i].label);
	}

	puts("\nnames");
	check_name(state_name(LOCKED), "LOCKED", "state_name(LOCKED)");
	check_name(state_name(GOT1), "GOT1", "state_name(GOT1)");
	check_name(state_name(GOT2), "GOT2", "state_name(GOT2)");
	check_name(state_name(OPEN), "OPEN", "state_name(OPEN)");
	check_name(state_name(N_STATES), "?", "state_name(N_STATES)");
	check_name(state_name((state_t)-1), "?", "state_name(-1)");
	check_name(event_name(E_RIGHT), "RIGHT", "event_name(E_RIGHT)");
	check_name(event_name(E_WRONG), "WRONG", "event_name(E_WRONG)");
	check_name(event_name(E_CLOSE), "CLOSE", "event_name(E_CLOSE)");
	check_name(event_name(E_TIMEOUT), "TIMEOUT", "event_name(E_TIMEOUT)");
	check_name(event_name(N_EVENTS), "?", "event_name(N_EVENTS)");
	check_name(action_name(A_UNDEFINED), "UNDEFINED", "action_name(A_UNDEFINED)");
	check_name(action_name(A_NONE), "NONE", "action_name(A_NONE)");
	check_name(action_name(A_UNLOCK), "UNLOCK", "action_name(A_UNLOCK)");
	check_name(action_name(A_LOCK), "LOCK", "action_name(A_LOCK)");
	check_name(action_name(A_BEEP), "BEEP", "action_name(A_BEEP)");
	check_name(action_name(N_ACTIONS), "?", "action_name(N_ACTIONS)");

	printf("\n%d passed, %d failed\n", passed, failed);
	return failed == 0 ? 0 : 1;
}
