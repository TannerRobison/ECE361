/*
 * tests/test_tally.c: unit tests for the tally ADT.
 *
 * Prints PASS or FAIL for each test and a summary, and exits nonzero if any
 * test failed, so that "make test" stops on a failure.
 *
 * Every group destroys the tallies it creates, so that the next one starts
 * with the whole pool free.
 *
 * ECE 361, Fall 2026, HW4. Tanner Robison.
 */
#include <stdio.h>
#include <string.h>

#include "../tally.h"

static int total = 0;
static int fails = 0;

static void test_int(const char *name, int expected, int actual) {
    total++;
    if (expected == actual) {
        printf("PASS  %s\n", name);
    } else {
        fails++;
        printf("FAIL  %s: expected %d, got %d\n", name, expected, actual);
    }
}

static void test_true(const char *name, int ok) {
    total++;
    if (ok) {
        printf("PASS  %s\n", name);
    } else {
        fails++;
        printf("FAIL  %s\n", name);
    }
}

/* runs tally_print into a temporary file, reads back what it wrote, and
   compares that with expected */
static void test_print(const char *name, const tally_t *t,
                       const char *expected) {
    char got[1024];
    FILE *f = tmpfile();

    total++;
    if (f == NULL) {
        fails++;
        printf("FAIL  %s: tmpfile() failed\n", name);
        return;
    }
    tally_print(t, f);
    rewind(f);
    size_t n = fread(got, 1, sizeof got - 1, f);
    got[n] = '\0';
    fclose(f);

    if (strcmp(expected, got) == 0) {
        printf("PASS  %s\n", name);
    } else {
        fails++;
        printf("FAIL  %s: expected\n%sgot\n%s", name, expected, got);
    }
}

/* fills buf with len copies of c and a '\0'; buf holds at least len + 1 */
static void make_name(char *buf, int len, char c) {
    memset(buf, c, (size_t) len);
    buf[len] = '\0';
}

int main(void) {
    tally_t *t;
    tally_t *u;

    /* a new tally is empty */
    printf("tally_create\n");
    t = tally_create();
    test_true("create: returns a tally", t != NULL);
    test_int("create: a new tally has size 0", 0, tally_size(t));
    test_int("create: a new tally counts nothing", 0,
             tally_count(t, "door_open"));
    test_print("create: a new tally prints nothing", t, "");
    tally_destroy(t);

    /* counting up, and names that were never added */
    printf("\ntally_add and tally_count, counting up\n");
    t = tally_create();
    test_int("add: first door_open returns 1", 1, tally_add(t, "door_open"));
    test_int("add: second door_open returns 2", 2, tally_add(t, "door_open"));
    test_int("add: third door_open returns 3", 3, tally_add(t, "door_open"));
    test_int("count: door_open is 3", 3, tally_count(t, "door_open"));
    test_int("size: one name added three times is 1", 1, tally_size(t));
    test_int("add: a second name returns 1", 1, tally_add(t, "motion"));
    test_int("count: the second name does not change the first", 3,
             tally_count(t, "door_open"));
    test_int("size: two names", 2, tally_size(t));
    test_int("count: a name never added is 0", 0, tally_count(t, "knock"));
    test_int("count: names are case sensitive", 0,
             tally_count(t, "Door_open"));
    test_int("count: a prefix is a different name", 0,
             tally_count(t, "door"));
    tally_destroy(t);

    /* the tally keeps its own copy of each name */
    printf("\ntally_add copies the name\n");
    {
        char buf[] = "motion";
        t = tally_create();
        tally_add(t, buf);
        buf[0] = 'n';                   /* buf is now "notion" */
        test_int("copy: the tally still holds motion", 1,
                 tally_count(t, "motion"));
        test_int("copy: the changed buffer is not in the tally", 0,
                 tally_count(t, buf));
        tally_destroy(t);
    }

    /* empty names, and names right at and just past TALLY_LEN */
    printf("\ntally_add, name lengths\n");
    {
        char exact[TALLY_LEN + 1];      /* TALLY_LEN characters */
        char over[TALLY_LEN + 2];       /* TALLY_LEN + 1 characters */
        char expect[TALLY_LEN + 8];

        make_name(exact, TALLY_LEN, 'x');
        make_name(over, TALLY_LEN + 1, 'y');
        t = tally_create();

        test_int("add: an empty name returns -1", -1, tally_add(t, ""));
        test_int("add: an empty name is not added", 0, tally_size(t));
        test_int("count: an empty name is 0", 0, tally_count(t, ""));

        test_int("add: TALLY_LEN characters returns 1", 1,
                 tally_add(t, exact));
        test_int("count: TALLY_LEN characters", 1, tally_count(t, exact));
        snprintf(expect, sizeof expect, "%s 1\n", exact);
        test_print("print: a name of TALLY_LEN characters prints whole", t,
                   expect);

        test_int("add: TALLY_LEN + 1 characters returns -1", -1,
                 tally_add(t, over));
        test_int("count: TALLY_LEN + 1 characters is 0", 0,
                 tally_count(t, over));
        over[TALLY_LEN] = '\0';         /* its first TALLY_LEN characters */
        test_int("add: the refused name was not cut short and stored", 0,
                 tally_count(t, over));
        test_int("size: only the TALLY_LEN name went in", 1, tally_size(t));
        tally_destroy(t);
    }

    /* NULL arguments: -1 or 0, and nothing changes */
    printf("\nNULL arguments\n");
    t = tally_create();
    tally_add(t, "knock");
    test_int("add: t is NULL returns -1", -1, tally_add(NULL, "knock"));
    test_int("add: name is NULL returns -1", -1, tally_add(t, NULL));
    test_int("add: name is NULL changes nothing", 1, tally_size(t));
    test_int("count: t is NULL is 0", 0, tally_count(NULL, "knock"));
    test_int("count: name is NULL is 0", 0, tally_count(t, NULL));
    test_int("size: t is NULL is 0", 0, tally_size(NULL));
    test_print("print: t is NULL writes nothing", NULL, "");
    tally_print(t, NULL);       /* without the NULL check this would crash */
    test_true("print: out is NULL returns", 1);
    tally_destroy(NULL);        /* so would this */
    test_true("destroy: NULL returns", 1);
    test_int("destroy: NULL leaves the other tally alone", 1,
             tally_count(t, "knock"));
    tally_destroy(t);

    /* a full tally refuses new names but still counts the ones it holds */
    printf("\na full tally\n");
    t = tally_create();
    {
        char name[] = "a";
        int ok = 1;
        for (int i = 0; i < TALLY_NAMES; i++) {
            name[0] = (char) ('a' + i);         /* "a", "b", ... */
            if (tally_add(t, name) != 1) {
                ok = 0;
            }
        }
        test_true("full: TALLY_NAMES different names all go in", ok);
    }
    test_int("full: size is TALLY_NAMES", TALLY_NAMES, tally_size(t));
    test_int("full: a new name returns -1", -1, tally_add(t, "zzz"));
    test_int("full: the new name is not counted", 0, tally_count(t, "zzz"));
    test_int("full: a new name that sorts first returns -1", -1,
             tally_add(t, "A"));        /* 'A' < 'a' in strcmp */
    test_int("full: size is still TALLY_NAMES", TALLY_NAMES, tally_size(t));
    test_int("full: the first name still counts", 2, tally_add(t, "a"));
    {
        char last[] = {(char) ('a' + TALLY_NAMES - 1), '\0'};
        test_int("full: the last name still counts", 2, tally_add(t, last));
    }
    tally_destroy(t);

    /* the pool: TALLY_MAX tallies, then NULL, then create after destroy */
    printf("\nthe pool of TALLY_MAX tallies\n");
    {
        tally_t *all[TALLY_MAX];
        int ok = 1;
        for (int i = 0; i < TALLY_MAX; i++) {
            all[i] = tally_create();
            if (all[i] == NULL) {
                ok = 0;
            }
            for (int j = 0; j < i; j++) {
                if (all[i] == all[j]) {
                    ok = 0;
                }
            }
        }
        test_true("pool: TALLY_MAX different tallies at the same time", ok);
        test_true("pool: one more returns NULL", tally_create() == NULL);
        tally_destroy(NULL);
        test_true("pool: destroy(NULL) does not free a tally",
                  tally_create() == NULL);

        tally_add(all[1], "door_open");
        tally_destroy(all[1]);
        u = tally_create();
        test_true("pool: create after destroy returns a tally", u != NULL);
        test_int("pool: the tally it returns is empty", 0, tally_size(u));
        test_int("pool: and holds none of the old names", 0,
                 tally_count(u, "door_open"));
        test_true("pool: full again, so create returns NULL",
                  tally_create() == NULL);

        tally_destroy(u);               /* the slot all[1] had */
        for (int i = 0; i < TALLY_MAX; i++) {
            if (i != 1) {
                tally_destroy(all[i]);
            }
        }
    }

    /* two tallies do not share anything */
    printf("\ntwo tallies are independent\n");
    t = tally_create();
    u = tally_create();
    tally_add(t, "knock");
    tally_add(t, "knock");
    tally_add(u, "knock");
    tally_add(u, "motion");
    test_int("independent: knock is 2 in t", 2, tally_count(t, "knock"));
    test_int("independent: knock is 1 in u", 1, tally_count(u, "knock"));
    test_int("independent: motion is 0 in t", 0, tally_count(t, "motion"));
    test_int("independent: motion is 1 in u", 1, tally_count(u, "motion"));
    test_int("independent: size of t is 1", 1, tally_size(t));
    test_int("independent: size of u is 2", 2, tally_size(u));
    tally_destroy(u);
    test_int("independent: destroying u leaves t alone", 2,
             tally_count(t, "knock"));
    tally_destroy(t);

    /* the entries stay sorted whether a name goes at the front, the
       middle or the end */
    printf("\ntally_print, order\n");
    t = tally_create();
    tally_add(t, "motion");
    tally_add(t, "unlock");             /* goes at the end */
    tally_add(t, "doorbell");           /* goes at the front */
    test_print("print: names added at the front and the end", t,
               "doorbell 1\nmotion 1\nunlock 1\n");
    tally_add(t, "lock");               /* goes in the middle */
    tally_add(t, "knock");              /* the middle again */
    tally_add(t, "lock");
    tally_add(t, "door");               /* a prefix sorts first */
    tally_add(t, "ALARM");              /* uppercase sorts before lowercase */
    test_print("print: alphabetical (strcmp) order, \"name count\" per line",
               t,
               "ALARM 1\ndoor 1\ndoorbell 1\nknock 1\nlock 2\nmotion 1\n"
               "unlock 1\n");
    tally_destroy(t);

    printf("\n%d tests, %d failed\n", total, fails);
    return fails != 0;
}
