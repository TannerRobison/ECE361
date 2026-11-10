// unit tests for store.c: record_compare and the binary search tree.
//  each check prints one PASS or FAIL line; make test counts them.
//  returns 1 if any check failed, 0 otherwise.

#include <stdio.h>
#include <time.h>

#include "../store.h"

static int failures = 0;

// prints PASS or FAIL with a name, and counts the failures
static void check(int ok, const char *name)
{
	if (ok) {
		printf("PASS: %s\n", name);
	} else {
		printf("FAIL: %s\n", name);
		failures++;
	}
}

// builds a record with only the fields the store looks at filled in
static record_t make_record(float rh, time_t t)
{
	record_t r = {0};
	r.reading.humidity = rh;
	r.reading.time = t;
	return r;
}

// context for the callbacks: remembers what was visited, in order
#define MAX_SEEN 128
typedef struct {
	int n;
	float rh[MAX_SEEN];
	time_t t[MAX_SEEN];
} seen_t;

// callback: records the humidity and timestamp of each record it is given
static void collect(const record_t *record, void *context)
{
	seen_t *seen = context;
	if (seen->n < MAX_SEEN) {
		seen->rh[seen->n] = record->reading.humidity;
		seen->t[seen->n] = record->reading.time;
	}
	seen->n++;
}

// --- record_compare ---------------------------------------------------------

static void test_compare_same_step(void)
{
	record_t a = make_record(45.2f, 0);
	record_t b = make_record(45.7f, 0);
	check(record_compare(&a, &b) == 0, "compare: 45.2 and 45.7 are the same step");
	check(record_compare(&b, &a) == 0, "compare: 45.7 and 45.2 are the same step");
}

static void test_compare_neighbor_steps(void)
{
	record_t a = make_record(45.7f, 0);
	record_t b = make_record(46.2f, 0);
	check(record_compare(&a, &b) < 0, "compare: 45.7 < 46.2 (neighboring steps)");
	check(record_compare(&b, &a) > 0, "compare: 46.2 > 45.7 (neighboring steps)");
}

static void test_compare_boundary(void)
{
	record_t below = make_record(45.9f, 0);
	record_t on = make_record(46.0f, 0);
	record_t top = make_record(46.9f, 0);
	check(record_compare(&on, &below) > 0, "compare: 46.0 starts step 46, above 45.9");
	check(record_compare(&on, &top) == 0, "compare: 46.0 and 46.9 are the same step");
}

// --- empty store ------------------------------------------------------------

static void test_empty(void)
{
	store_t *s = store_create();
	check(s != NULL, "empty: create returns a store");

	seen_t seen = {0};
	store_walk(s, collect, &seen);
	check(store_count(s) == 0, "empty: count is 0");
	check(store_height(s) == 0, "empty: height is 0");
	check(seen.n == 0, "empty: walk visits nothing");
	check(store_find_by_key(s, collect, &seen, 50) == 0, "empty: find returns 0");

	store_destroy(s); // destroy an empty tree; the sanitizer catches leaks
	check(1, "empty: destroy an empty tree");
}

static void test_null_arguments(void)
{
	record_t r = make_record(50.0f, 0);
	store_t *s = store_create();
	check(store_insert(NULL, &r) == -1, "null: insert into NULL store fails");
	check(store_insert(s, NULL) == -1, "null: insert NULL record fails");
	check(store_count(s) == 0, "null: failed inserts do not change count");
	check(store_count(NULL) == 0, "null: count of NULL store is 0");
	check(store_height(NULL) == 0, "null: height of NULL store is 0");
	store_destroy(s);
	store_destroy(NULL);
	check(1, "null: destroy NULL does nothing");
}

// --- insert, count, height --------------------------------------------------

static void test_insert_one(void)
{
	store_t *s = store_create();
	record_t r = make_record(52.3f, 100);
	check(store_insert(s, &r) == 0, "insert 1: returns 0");
	check(store_count(s) == 1, "insert 1: count is 1");
	check(store_height(s) == 1, "insert 1: height is 1");

	seen_t seen = {0};
	check(store_find_by_key(s, collect, &seen, 52) == 1, "insert 1: find key 52 returns 1");
	check(seen.n == 1 && seen.t[0] == 100, "insert 1: found the inserted record");
	store_destroy(s);
}

static void test_insert_two(void)
{
	store_t *s = store_create();
	record_t a = make_record(52.0f, 0);
	record_t b = make_record(50.0f, 1);
	store_insert(s, &a);
	store_insert(s, &b);
	check(store_count(s) == 2, "insert 2: count is 2");
	check(store_height(s) == 2, "insert 2: height is 2");
	store_destroy(s);
}

// the tree from SPEC.md: 52 at the root, 50 and 55 below it, 51 and 54 below those
static void test_known_shape(void)
{
	store_t *s = store_create();
	float rh[] = {52.0f, 50.0f, 55.0f, 51.0f, 54.0f};
	for (int i = 0; i < 5; i++) {
		record_t r = make_record(rh[i], i);
		store_insert(s, &r);
	}
	check(store_count(s) == 5, "shape: count is 5");
	check(store_height(s) == 3, "shape: height is 3");
	store_destroy(s);
}

static void test_insert_100(void)
{
	// keys in increasing order: each new node goes right of the last,
	//  so the tree is one chain and its height equals its count
	store_t *s = store_create();
	for (int i = 0; i < 100; i++) {
		record_t r = make_record((float)i, i);
		store_insert(s, &r);
	}
	check(store_count(s) == 100, "insert 100 sorted: count is 100");
	check(store_height(s) == 100, "insert 100 sorted: height is 100 (a chain)");
	store_destroy(s);

	// the same key 100 times: duplicates go right, so also a chain
	s = store_create();
	for (int i = 0; i < 100; i++) {
		record_t r = make_record(52.5f, i);
		store_insert(s, &r);
	}
	check(store_count(s) == 100, "insert 100 equal: count is 100");
	check(store_height(s) == 100, "insert 100 equal: height is 100 (a chain)");
	check(store_find_by_key(s, NULL, NULL, 52) == 100, "insert 100 equal: find returns all 100");
	store_destroy(s);
}

// --- find -------------------------------------------------------------------

static void test_find_duplicates(void)
{
	// three records in step 52, mixed in with others
	store_t *s = store_create();
	float rh[] = {52.0f, 50.3f, 55.1f, 52.9f, 51.0f, 52.4f, 54.0f};
	for (int i = 0; i < 7; i++) {
		record_t r = make_record(rh[i], i);
		store_insert(s, &r);
	}

	seen_t seen = {0};
	size_t found = store_find_by_key(s, collect, &seen, 52);
	check(found == 3, "find dup: key 52 returns 3");
	check(seen.n == 3, "find dup: callback called 3 times");
	check(seen.n == 3 && seen.t[0] == 0 && seen.t[1] == 3 && seen.t[2] == 5,
	      "find dup: all three found, in insertion order");
	store_destroy(s);
}

static void test_find_missing(void)
{
	store_t *s = store_create();
	float rh[] = {52.0f, 50.3f, 55.1f};
	for (int i = 0; i < 3; i++) {
		record_t r = make_record(rh[i], i);
		store_insert(s, &r);
	}

	seen_t seen = {0};
	check(store_find_by_key(s, collect, &seen, 53) == 0, "find missing: key 53 returns 0");
	check(seen.n == 0, "find missing: callback never called");
	check(store_find_by_key(s, collect, &seen, 49) == 0, "find missing: key below all returns 0");
	check(store_find_by_key(s, collect, &seen, 56) == 0, "find missing: key above all returns 0");
	store_destroy(s);
}

// --- walk -------------------------------------------------------------------

static void test_walk_order(void)
{
	store_t *s = store_create();
	float rh[] = {60.2f, 55.0f, 70.9f, 55.5f, 65.0f, 48.1f, 59.9f};
	for (int i = 0; i < 7; i++) {
		record_t r = make_record(rh[i], i);
		store_insert(s, &r);
	}

	seen_t seen = {0};
	store_walk(s, collect, &seen);
	check(seen.n == 7, "walk: visits all 7 records");

	int in_order = 1;
	for (int i = 1; i < seen.n; i++) {
		if ((int)seen.rh[i - 1] > (int)seen.rh[i]) {
			in_order = 0;
		}
	}
	check(in_order, "walk: records come out in key order");
	check(seen.t[1] == 1 && seen.t[2] == 3,
	      "walk: equal keys (55.0, 55.5) come out in insertion order");
	store_destroy(s);
}

int main(void)
{
	test_compare_same_step();
	test_compare_neighbor_steps();
	test_compare_boundary();
	test_empty();
	test_null_arguments();
	test_insert_one();
	test_insert_two();
	test_known_shape();
	test_insert_100();
	test_find_duplicates();
	test_find_missing();
	test_walk_order();

	return failures != 0;
}
