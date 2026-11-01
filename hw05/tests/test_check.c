#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../check.h"
#include "../lock.h"

static int passed;
static int failed;

/* The table under test: each test starts from a copy of lock_table and
 * breaks it. */
static transition_t t[N_STATES][N_EVENTS];

/* What the last run of fsm_check printed. */
static char out[4096];

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

/* Start a new test: print its name and copy lock_table into t. */
static void begin(const char *name)
{
	printf("\n%s\n", name);
	memcpy(t, lock_table, sizeof t);
}

/* Change one entry of t. A left-out entry is all zeros: {LOCKED,
 * A_UNDEFINED}. */
static void set(state_t s, event_t e, state_t next, action_t action)
{
	t[s][e].next = next;
	t[s][e].action = action;
}

/* Run fsm_check on t from start, keep what it prints in out, and show it
 * indented with "|". Checks that it returns want and prints exactly one line
 * per problem. */
static void expect(state_t start, int want)
{
	FILE *f = tmpfile();
	if (f == NULL) {
		perror("tmpfile");
		exit(2);
	}
	int got = fsm_check(t, start, f);
	rewind(f);
	size_t len = fread(out, 1, sizeof out - 1, f);
	out[len] = '\0';
	fclose(f);

	int lines = 0;
	int at_start = 1;
	for (size_t i = 0; i < len; i++) {
		if (at_start) {
			fputs("    | ", stdout);
		}
		putchar(out[i]);
		at_start = out[i] == '\n';
		if (out[i] == '\n') {
			lines++;
		}
	}

	char name[80];
	snprintf(name, sizeof name, "returns %d (got %d)", want, got);
	check(got == want, name);
	snprintf(name, sizeof name, "prints one line per problem (%d lines)", lines);
	check(lines == got, name);
}

/* Check that the last run of fsm_check printed line, as a whole line. */
static void printed(const char *line)
{
	char whole[128];
	char name[160];
	snprintf(whole, sizeof whole, "%s\n", line);
	snprintf(name, sizeof name, "prints \"%s\"", line);
	check(strstr(out, whole) != NULL, name);
}

int main(void)
{
	begin("lock_table, the real one: no problems");
	expect(LOCKED, 0);

	begin("left out: OPEN + CLOSE");
	set(OPEN, E_CLOSE, LOCKED, A_UNDEFINED);
	expect(LOCKED, 1);
	printed("left out: OPEN + CLOSE has action A_UNDEFINED");

	begin("out of range: GOT2 + RIGHT goes to 4 (OPEN, counting from 1)");
	set(GOT2, E_RIGHT, (state_t)4, A_UNLOCK);
	expect(LOCKED, 2);
	printed("out of range: GOT2 + RIGHT goes to state 4");
	printed("unreachable: OPEN cannot be reached from LOCKED");

	begin("unreachable: GOT2 + RIGHT stays in GOT2");
	set(GOT2, E_RIGHT, GOT2, A_NONE);
	expect(LOCKED, 1);
	printed("unreachable: OPEN cannot be reached from LOCKED");

	begin("the same table from OPEN: every state is reachable");
	set(GOT2, E_RIGHT, GOT2, A_NONE);
	expect(OPEN, 0);

	begin("no way out: close and timeout leave OPEN in OPEN");
	set(OPEN, E_CLOSE, OPEN, A_NONE);
	set(OPEN, E_TIMEOUT, OPEN, A_NONE);
	expect(LOCKED, 1);
	printed("no way out: every event leaves OPEN in OPEN");

	begin("a left-out entry is not a way out");
	set(OPEN, E_CLOSE, OPEN, A_NONE);
	set(OPEN, E_TIMEOUT, LOCKED, A_UNDEFINED);
	expect(LOCKED, 2);
	printed("left out: OPEN + TIMEOUT has action A_UNDEFINED");
	printed("no way out: every event leaves OPEN in OPEN");

	begin("every entry left out");
	memset(t, 0, sizeof t);
	expect(LOCKED, 23);
	printed("left out: LOCKED + RIGHT has action A_UNDEFINED");
	printed("left out: OPEN + TIMEOUT has action A_UNDEFINED");
	printed("unreachable: GOT1 cannot be reached from LOCKED");
	printed("no way out: every event leaves LOCKED in LOCKED");

	begin("start out of range");
	expect((state_t)9, 1);
	printed("out of range: start state 9");

	printf("\n%d passed, %d failed\n", passed, failed);
	return failed == 0 ? 0 : 1;
}
