/*
 * histogram.h: printing the histogram.
 *
 * ECE 361, Fall 2026, HW2. Tanner Robison.
 */
#ifndef HISTOGRAM_H
#define HISTOGRAM_H

/*
 * Prints a histogram of a to stdout: the line "histogram:", then one line per
 * non-empty bin, each with its range, a row of '*', and the count. The bins
 * are 5.0 wide and cover 0 to 100; a value below 0 lands in the first bin and
 * a value at or above 100 in the last.
 *
 * Takes:    a, an array, and n, its length.
 * Returns:  nothing.
 * Assumes:  n >= 1 and a is not NULL. The caller has already decided there is
 *           something to print.
 */
void histogram_print(const float a[], int n);

#endif /* HISTOGRAM_H */
