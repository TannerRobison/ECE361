#ifndef STORE_H
#define STORE_H

#include <stddef.h>
#include <stdint.h>

#include "fsm.h"
#include "sensor.h"

typedef struct store store_t;

typedef struct {
	reading_t reading;
	state_t state;
	uint32_t status;
} record_t;

// allocates the memory for a store and initializes members to appropriate value
// (0 or NULL)
// returns the store to user declared pointer
store_t *store_create(void);

// compares the truncated RH key of each node,
// returns 1 if a > b
// retunrs 0 if a = b
// returns -1 if a < b
int record_compare(const record_t *a, const record_t *b);

// inserts a new node in the binary tree at the appropriate location.
// returns 0 on success, -1 on failure
int store_insert(store_t *store, const record_t *record);

// returns the number of nodes in the store
size_t store_count(const store_t *store);

// callback function pointer
typedef void (*store_visit_fn)(const record_t *record, void *context);
// walks through the store and calls the callback on each of the nodes in the
// tree.
//  returns nothing (void type)
void store_walk(const store_t *store, store_visit_fn callback, void *context);

// calls callback on every record whose key (%RH, truncated to a whole percent)
//  equals key, in insertion order, and returns how many there were.
//  callback may be NULL to only count.
size_t store_find_by_key(const store_t *store, store_visit_fn callback,
			 void *context, int key);

// frees the allocated memory for each node and the store
// returns nothing
void store_destroy(store_t *store);

// returns levels on the longest path from the root: 0 for an empty tree, 1 for
// one record
int store_height(const store_t *store);

#endif
