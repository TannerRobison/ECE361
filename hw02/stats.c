/*
 * stats.c: minimum, maximum, and mean of an array, and the longest run above
 * a threshold.
 *
 * ECE 361, Fall 2026, HW2. Tanner Robison.
 */
#include "stats.h"

float stats_min(const float a[], int n) {
    float min = a[0];
    for (int i = 1; i < n; i++)
        if (a[i] < min)
            min = a[i];
    return min;
}

/*
 * Recursive. One frame per element: stats_max(a, n) cannot return until
 * stats_max(a, n - 1) has, so all n frames are on the stack at the base case.
 */
float stats_max(const float a[], int n) {
    if (n == 1)
        return a[0];
    float rest = stats_max(a, n - 1);
    return a[n - 1] > rest ? a[n - 1] : rest;
}

float stats_mean(const float a[], int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; i++)
        sum += a[i];
    return sum / n;
}

int stats_longest_run_above(const float a[], int n, float threshold, int *start) {
    int run = 0, best = 0, best_start = -1, this_start = 0;

    for (int i = 0; i < n; i++) {
        if (a[i] > threshold) {
            if (run == 0)
                this_start = i;
            run++;
            if (run > best) {
                best = run;
                best_start = this_start;
            }
        } else {
            run = 0;
        }
    }
    *start = best_start;
    return best;
}
