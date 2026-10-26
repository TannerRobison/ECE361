#ifndef TALLY_H
#define TALLY_H

#include <stdio.h> // FILE, for tally_print

#define TALLY_MAX     4    /* tallies that can exist at the same time */
#define TALLY_NAMES  16    /* different names one tally can hold */
#define TALLY_LEN    15    /* longest name, without the '\0' */

typedef struct tally tally_t;      /* opaque */

// Every function that takes a tally_t *t assumes t is NULL or came from
// tally_create and has not been destroyed since.

// takes nothing; returns a new, empty tally, or NULL if TALLY_MAX tallies are
// already in use. The tally comes from a fixed pool inside tally.c, so there
// is nothing to free: hand it back with tally_destroy.
tally_t *tally_create(void);

// takes a tally and gives it back, so that tally_create can hand it out again;
// does nothing if t is NULL. Assumes the caller does not use t afterwards.
void     tally_destroy(tally_t *t);

// takes a tally and a '\0'-terminated name, which it copies; counts one more
// occurrence of name and returns its new count (1 for a new name). Returns -1
// and changes nothing if t or name is NULL, name is empty or longer than
// TALLY_LEN, or name is new and the tally already holds TALLY_NAMES names.
int      tally_add(tally_t *t, const char *name);

// takes a tally and a name; returns how often name was added: 0 if never, or
// if t or name is NULL. Does not change the tally.
int      tally_count(const tally_t *t, const char *name);

// takes a tally; returns how many different names it holds, 0 if t is NULL.
int      tally_size(const tally_t *t);

// takes a tally and an open stream; writes one line per name, "name count",
// in alphabetical (strcmp) order. Writes nothing if t or out is NULL.
void     tally_print(const tally_t *t, FILE *out);

#endif
