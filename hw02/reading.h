/*
 * reading.h: reading and parsing the input lines.
 *
 * ECE 361, Fall 2026, HW2. Tanner Robison.
 */
#ifndef READING_H
#define READING_H

#include <stdio.h>

/* The most readings this module will store in one call. */
#define READING_MAX 1000

/*
 * Reads sensor readings from in, one per line, "tick temperature humidity".
 *
 * A line whose first non-blank character is '\n', '\0' or '#' is ignored and
 * counted nowhere. Any other line that does not hold three numbers is counted
 * in *skipped. After max readings have been stored the rest of the input is
 * ignored and one warning goes to stderr.
 *
 * Takes:    in, open for reading; ticks, temps and hums, each with room for
 *           at least max elements; skipped, where the count of bad lines goes.
 * Returns:  the number of readings stored, 0 to max. The first n elements of
 *           ticks, temps and hums hold them; the rest are untouched.
 * Assumes:  max >= 1, and that in, ticks, temps, hums and skipped are not NULL.
 */
int reading_read(FILE *in, int ticks[], float temps[], float hums[],
                 int max, int *skipped);

#endif /* READING_H */
