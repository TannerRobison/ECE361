#include "tally.h"

#include <string.h>

// one name and how often it was added
struct entry {
	char name[TALLY_LEN + 1]; // + 1 for the '\0'
	int count;
};

// The only place struct tally is completed. Every other file sees just the
// name tally_t, so it can hold a pointer to a tally but never look inside one:
// it has to go through the functions in tally.h.
struct tally {
	int in_use;                        // 1 while handed out by tally_create
	int n;                             // names held, in entries[0..n-1]
	struct entry entries[TALLY_NAMES]; // kept sorted by name (strcmp)
};

// Every tally there is. static, so no other file can reach it by name, and
// zeroed before main runs, so every tally starts free (in_use is 0).
static tally_t pool[TALLY_MAX];

// looks for name in t->entries[0..n-1]. Returns its index and sets *found to
// 1, or, if name is not there, returns the index where it belongs to keep the
// entries sorted (n if it goes after all of them) and sets *found to 0
static int find(const tally_t *t, const char *name, int *found)
{
	int i;
	for (i = 0; i < t->n; i++) {
		int cmp = strcmp(t->entries[i].name, name);
		if (cmp == 0) {
			*found = 1;
			return i;
		}
		if (cmp > 0) {
			break; // the entries are sorted, so name belongs right here
		}
	}
	*found = 0;
	return i;
}

// returns the first free tally in the pool, emptied, or NULL if none is free
tally_t *tally_create(void)
{
	for (int i = 0; i < TALLY_MAX; i++) {
		if (!pool[i].in_use) {
			pool[i].in_use = 1;
			pool[i].n = 0; // forget whatever it held before it was destroyed
			return &pool[i];
		}
	}
	return NULL;
}

// marks t free again; does nothing if t is NULL
void tally_destroy(tally_t *t)
{
	if (t == NULL) {
		return;
	}
	t->in_use = 0;
	t->n = 0;
}

// counts one more name and returns its new count, or -1 and changes nothing
int tally_add(tally_t *t, const char *name)
{
	if (t == NULL || name == NULL) {
		return -1;
	}

	// check the length before anything is copied: name and its '\0' must
	// fit in an entry's name[TALLY_LEN + 1]
	size_t len = strlen(name);
	if (len == 0 || len > TALLY_LEN) {
		return -1;
	}

	int found;
	int i = find(t, name, &found);
	if (found) {
		t->entries[i].count++;
		return t->entries[i].count;
	}

	if (t->n == TALLY_NAMES) {
		return -1; // full: no room for a new name
	}

	// Open a gap at i by moving entries[i..n-1] up by one. Start from the
	// last one: it moves into entries[n], which is free (n < TALLY_NAMES
	// here), and each one after it moves into the slot just emptied. Going
	// from i upward instead would copy entries[i] over entries[i + 1]
	// before entries[i + 1] had moved, and lose it.
	for (int j = t->n; j > i; j--) {
		t->entries[j] = t->entries[j - 1];
	}

	// len <= TALLY_LEN, so these len + 1 bytes, the '\0' included, fit
	memcpy(t->entries[i].name, name, len + 1);
	t->entries[i].count = 1;
	t->n++;
	return 1;
}

// returns how often name was added, 0 if never or if t or name is NULL
int tally_count(const tally_t *t, const char *name)
{
	if (t == NULL || name == NULL) {
		return 0;
	}

	int found;
	int i = find(t, name, &found);
	return found ? t->entries[i].count : 0;
}

// returns how many different names t holds, 0 if t is NULL
int tally_size(const tally_t *t)
{
	if (t == NULL) {
		return 0;
	}
	return t->n;
}

// writes "name count" per line; the entries are already in strcmp order
void tally_print(const tally_t *t, FILE *out)
{
	if (t == NULL || out == NULL) {
		return;
	}

	for (int i = 0; i < t->n; i++) {
		fprintf(out, "%s %d\n", t->entries[i].name, t->entries[i].count);
	}
}
