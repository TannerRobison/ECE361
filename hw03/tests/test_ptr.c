/*
 * tests/test_ptr.c: unit tests for the pointer library.
 *
 * Prints PASS or FAIL for each test and a summary, and exits nonzero if any
 * test failed, so that "make test" stops on a failure.
 *
 * ECE 361, Fall 2026, HW3. Tanner Robison.
 */
#include <stdio.h>

#include "../ptr.h"

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

/* Exact ==, no epsilon: every float these functions return or store is
   copied from the input, never computed, so it must match bit for bit. */
static void test_float(const char *name, float expected, float actual) {
    total++;
    if (expected == actual) {
        printf("PASS  %s\n", name);
    } else {
        fails++;
        printf("FAIL  %s: expected %g, got %g\n", name, (double) expected,
               (double) actual);
    }
}

static void test_array(const char *name, const float *expected,
                       const float *actual, int n) {
    total++;
    for (int i = 0; i < n; i++) {
        if (expected[i] != actual[i]) {
            fails++;
            printf("FAIL  %s: at index %d expected %g, got %g\n", name, i,
                   (double) expected[i], (double) actual[i]);
            return;
        }
    }
    printf("PASS  %s\n", name);
}

/* prints where p points, as "NULL" or an offset from the start of the array */
static void print_where(const float *base, const float *p) {
    if (p == NULL)
        printf("NULL");
    else
        printf("a + %td", p - base);
}

static void test_ptr(const char *name, const float *base,
                     const float *expected, const float *actual) {
    total++;
    if (expected == actual) {
        printf("PASS  %s\n", name);
    } else {
        fails++;
        printf("FAIL  %s: expected ", name);
        print_where(base, expected);
        printf(", got ");
        print_where(base, actual);
        printf("\n");
    }
}

int main(void) {
    float x;
    float y;
    float lo;
    float hi;
    int   rc;

    /* swap_float */
    printf("\nswap_float\n");
    x = 1.5f;
    y = -2.25f;
    swap_float(&x, &y);
    test_float("swap_float, x gets y's value", -2.25f, x);
    test_float("swap_float, y gets x's value", 1.5f, y);
    x = 3.0f;
    swap_float(&x, &x);
    test_float("swap_float, a and b point to the same float", 3.0f, x);
    {
        float v[]      = {1.0f, 2.0f, 3.0f};
        float expect[] = {3.0f, 2.0f, 1.0f};
        swap_float(v, v + 2);
        test_array("swap_float, two elements of one array", expect, v, 3);
    }

    /* minmax on several values */
    printf("\nminmax, several values\n");
    {
        float v[] = {3.0f, 1.0f, 4.0f, 1.0f, 5.0f};
        rc = minmax(v, 5, &lo, &hi);
        test_int("minmax, several values: returns 0", 0, rc);
        test_float("minmax, several values: min", 1.0f, lo);
        test_float("minmax, several values: max", 5.0f, hi);
    }
    {
        float down[] = {4.0f, 3.0f, 2.0f, 1.0f};    /* max first, min last */
        minmax(down, 4, &lo, &hi);
        test_float("minmax, min at the end", 1.0f, lo);
        test_float("minmax, max at the start", 4.0f, hi);
    }
    {
        float neg[] = {-5.0f, -2.0f, -9.0f};        /* a max that started at
                                                       0 would stay 0 */
        minmax(neg, 3, &lo, &hi);
        test_float("minmax, all negative: min", -9.0f, lo);
        test_float("minmax, all negative: max", -2.0f, hi);
    }
    {
        float v[] = {3.0f, 1.0f, 4.0f};
        lo = -99.0f;                                /* these would win every */
        hi = 99.0f;                                 /* comparison if minmax  */
        minmax(v, 3, &lo, &hi);                     /* started from them     */
        test_float("minmax, ignores the old *min", 1.0f, lo);
        test_float("minmax, ignores the old *max", 4.0f, hi);
    }

    /* minmax on one value: min and max are the same element */
    printf("\nminmax, one value\n");
    {
        float one[] = {7.5f};
        rc = minmax(one, 1, &lo, &hi);
        test_int("minmax, one value: returns 0", 0, rc);
        test_float("minmax, one value: min", 7.5f, lo);
        test_float("minmax, one value: max", 7.5f, hi);
    }

    /* bad arguments: -1, and *min and *max keep what they held before */
    printf("\nminmax, bad arguments: returns -1 and changes nothing\n");
    {
        float v[] = {3.0f, 1.0f, 4.0f};

        lo = -99.0f;
        hi = 99.0f;
        rc = minmax(v, 0, &lo, &hi);
        test_int("minmax, n = 0: returns -1", -1, rc);
        test_float("minmax, n = 0: *min unchanged", -99.0f, lo);
        test_float("minmax, n = 0: *max unchanged", 99.0f, hi);

        lo = -99.0f;
        hi = 99.0f;
        rc = minmax(v, -1, &lo, &hi);
        test_int("minmax, n = -1: returns -1", -1, rc);
        test_float("minmax, n = -1: *min unchanged", -99.0f, lo);
        test_float("minmax, n = -1: *max unchanged", 99.0f, hi);

        lo = -99.0f;
        hi = 99.0f;
        rc = minmax(NULL, 3, &lo, &hi);
        test_int("minmax, a is NULL: returns -1", -1, rc);
        test_float("minmax, a is NULL: *min unchanged", -99.0f, lo);
        test_float("minmax, a is NULL: *max unchanged", 99.0f, hi);

        hi = 99.0f;
        rc = minmax(v, 3, NULL, &hi);
        test_int("minmax, min is NULL: returns -1", -1, rc);
        test_float("minmax, min is NULL: *max unchanged", 99.0f, hi);

        lo = -99.0f;
        rc = minmax(v, 3, &lo, NULL);
        test_int("minmax, max is NULL: returns -1", -1, rc);
        test_float("minmax, max is NULL: *min unchanged", -99.0f, lo);
    }

    /* find_above: the pointer it returns must be the element itself */
    printf("\nfind_above\n");
    {
        float v[] = {1.0f, 5.0f, 2.0f, 7.0f};
        test_ptr("find_above, two matches: returns the first", v, v + 1,
                 find_above(v, 4, 3.0f));
        test_ptr("find_above, no match: NULL", v, NULL,
                 find_above(v, 4, 10.0f));
        test_ptr("find_above, a value equal to the threshold is not above it",
                 v, v + 3, find_above(v, 4, 5.0f));
        test_ptr("find_above, match at the first element", v, v,
                 find_above(v, 4, 0.0f));
        test_ptr("find_above, match at the last element", v, v + 3,
                 find_above(v, 4, 6.0f));
        test_ptr("find_above, stops at n: the 7 is past it", v, NULL,
                 find_above(v, 3, 6.0f));
        test_ptr("find_above, n = 0: NULL", v, NULL, find_above(v, 0, 0.0f));
        test_ptr("find_above, a is NULL: NULL", v, NULL,
                 find_above(NULL, 4, 0.0f));
    }
    {
        float at[] = {5.0f, 5.0f, 5.0f};
        test_ptr("find_above, every value equal to the threshold: NULL", at,
                 NULL, find_above(at, 3, 5.0f));
    }

    /* reverse: odd lengths meet on the middle element, even lengths cross */
    printf("\nreverse\n");
    {
        float v[]      = {1.0f, 2.0f, 3.0f, 4.0f};
        float expect[] = {4.0f, 3.0f, 2.0f, 1.0f};
        reverse(v, 4);
        test_array("reverse, even length", expect, v, 4);
    }
    {
        float v[]      = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
        float expect[] = {5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
        reverse(v, 5);
        test_array("reverse, odd length", expect, v, 5);
    }
    {
        float v[]      = {1.0f, 2.0f};
        float expect[] = {2.0f, 1.0f};
        reverse(v, 2);
        test_array("reverse, two elements", expect, v, 2);
    }
    {
        float v[]      = {1.0f, 2.0f, 3.0f, 9.0f};
        float expect[] = {3.0f, 2.0f, 1.0f, 9.0f};
        reverse(v, 3);
        test_array("reverse, touches only a[0..n-1]", expect, v, 4);
    }
    {
        float v[]      = {7.0f};
        float expect[] = {7.0f};
        reverse(v, 1);
        test_array("reverse, n = 1: unchanged", expect, v, 1);
    }
    {
        float v[]      = {1.0f, 2.0f, 3.0f};
        float expect[] = {1.0f, 2.0f, 3.0f};
        reverse(v, 0);
        test_array("reverse, n = 0: unchanged", expect, v, 3);
    }
    reverse(NULL, 4);           /* without the NULL check this would crash */
    test_int("reverse, a is NULL: returns", 0, 0);

    printf("\n%d tests, %d failed\n", total, fails);
    return fails != 0;
}
