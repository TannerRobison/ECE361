/*
 * tests/test_stats.c: unit tests for the stats module.
 *
 * Prints PASS or FAIL for each test and a summary, and exits nonzero if any
 * test failed, so that "make test" stops on a failure.
 *
 * ECE 361, Fall 2026, HW2. Tanner Robison.
 */
#include <stdio.h>

#include "../stats.h"

#define EPS   1e-6f          /* every expected value here is exact in binary */
#define BIG_N 10000

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

static void test_float(const char *name, float expected, float actual) {
    float diff = expected - actual;
    if (diff < 0.0f)
        diff = -diff;

    total++;
    if (diff <= EPS) {
        printf("PASS  %s\n", name);
    } else {
        fails++;
        printf("FAIL  %s: expected %g, got %g\n", name, (double) expected,
               (double) actual);
    }
}

int main(void) {
    static float big[BIG_N];                        /* 10,000 values, so that
                                                       stats_max recurses
                                                       10,000 deep */
    float one[]    = {42.5f};
    float up[]     = {1.0f, 2.0f, 3.0f, 4.0f};      /* min first, max last  */
    float down[]   = {4.0f, 3.0f, 2.0f, 1.0f};      /* max first, min last  */
    float neg[]    = {-3.5f, -1.0f, -9.25f};
    float tie[]    = {5.0f, 5.0f, 5.0f};
    int   start;
    int   best;

    /* one value: the base case of the recursion, and nothing to loop over */
    printf("\none value\n");
    test_float("stats_min, one value", 42.5f, stats_min(one, 1));
    test_float("stats_max, one value", 42.5f, stats_max(one, 1));
    test_float("stats_mean, one value", 42.5f, stats_mean(one, 1));

    /* the extreme at the start and at the end */
    printf("\nthe extreme at the start and at the end\n");
    test_float("stats_max, maximum at the end", 4.0f, stats_max(up, 4));
    test_float("stats_max, maximum at the start", 4.0f, stats_max(down, 4));
    test_float("stats_min, minimum at the start", 1.0f, stats_min(up, 4));
    test_float("stats_min, minimum at the end", 1.0f, stats_min(down, 4));
    test_float("stats_mean, 1 2 3 4", 2.5f, stats_mean(up, 4));
    test_float("stats_max, all negative", -1.0f, stats_max(neg, 3));
    test_float("stats_min, all negative", -9.25f, stats_min(neg, 3));
    test_float("stats_mean, all negative", -4.5833335f, stats_mean(neg, 3));
    test_float("stats_max, every value equal", 5.0f, stats_max(tie, 3));

    /* a value exactly equal to the threshold is not above it: it ends a run */
    printf("\na value exactly equal to the threshold\n");
    {
        float at[] = {31.0f, 30.0f, 31.0f, 31.0f, 31.0f, 29.0f};
        best = stats_longest_run_above(at, 6, 30.0f, &start);
        test_int("longest_run_above, the equal value ends the run", 3, best);
        test_int("longest_run_above, the run starts after it", 2, start);
    }
    {
        float all_at[] = {30.0f, 30.0f, 30.0f};
        best = stats_longest_run_above(all_at, 3, 30.0f, &start);
        test_int("longest_run_above, every value equal to the threshold", 0, best);
        test_int("longest_run_above, start is -1 when there is no run", -1, start);
    }

    /* the rest of stats_longest_run_above */
    printf("\nthe longest run\n");
    {
        float tail[] = {1.0f, 40.0f, 40.0f};
        best = stats_longest_run_above(tail, 3, 30.0f, &start);
        test_int("longest_run_above, the run reaches the last element", 2, best);
        test_int("longest_run_above, its start index", 1, start);
    }
    {
        float two[] = {40.0f, 40.0f, 1.0f, 40.0f, 40.0f};
        best = stats_longest_run_above(two, 5, 30.0f, &start);
        test_int("longest_run_above, two runs of the same length", 2, best);
        test_int("longest_run_above, the earlier run wins", 0, start);
    }
    {
        float none[] = {0.0f};
        best = stats_longest_run_above(none, 0, 30.0f, &start);
        test_int("longest_run_above, no elements at all", 0, best);
        test_int("longest_run_above, start is -1 on an empty array", -1, start);
    }

    /* the recursion on a big array: 10,000 frames before the base case */
    printf("\n10,000 values\n");
    for (int i = 0; i < BIG_N; i++)
        big[i] = (float) (i % 100);
    big[7777] = 1234.5f;
    big[3333] = -500.0f;
    test_float("stats_max, 10000 values", 1234.5f, stats_max(big, BIG_N));
    test_float("stats_min, 10000 values", -500.0f, stats_min(big, BIG_N));

    printf("\n%d tests, %d failed\n", total, fails);
    return fails != 0;
}
