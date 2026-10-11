/*
 * stats.h: minimum, maximum, and mean of an array, and the longest run above
 * a threshold.
 *
 * Every function here works on a plain array of float, so the same code serves
 * the temperatures and the humidities.
 *
 * ECE 361, Fall 2026, HW2. Tanner Robison.
 */
#ifndef STATS_H
#define STATS_H

/*
 * The smallest value in a.
 *
 * Takes:    a, an array, and n, its length.
 * Returns:  the smallest of a[0] to a[n-1].
 * Assumes:  n >= 1 and a is not NULL.
 */
float stats_min(const float a[], int n);

/*
 * The largest value in a, computed recursively: the larger of the last element
 * and the largest of the rest. The recursion is n deep, one frame per element.
 *
 * Takes:    a, an array, and n, its length.
 * Returns:  the largest of a[0] to a[n-1].
 * Assumes:  n >= 1 and a is not NULL. n < 1 would never reach the base case.
 */
float stats_max(const float a[], int n);

/*
 * The arithmetic mean of a.
 *
 * Takes:    a, an array, and n, its length.
 * Returns:  the sum of a[0] to a[n-1], accumulated in order as a float,
 *           divided by n.
 * Assumes:  n >= 1 and a is not NULL.
 */
float stats_mean(const float a[], int n);

/*
 * The longest run of consecutive elements strictly greater than threshold. An
 * element equal to threshold is not above it, and ends a run.
 *
 * Takes:    a, an array; n, its length; threshold; and start, where the index
 *           of the first element of the longest run is written.
 * Returns:  the length of the longest run, or 0 if no element is above
 *           threshold. On 0, *start is set to -1. When two runs tie, the
 *           earlier one wins.
 * Assumes:  n >= 0, start is not NULL, and a is not NULL when n > 0.
 */
int stats_longest_run_above(const float a[], int n, float threshold, int *start);

#endif /* STATS_H */
