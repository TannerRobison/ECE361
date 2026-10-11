/*
 * main.c: ECE 361, Fall 2026, HW2.
 *
 * Reads sensor readings from standard input and prints a summary: the number
 * of readings, the minimum, maximum, and mean temperature and humidity, the
 * longest run of readings above a temperature threshold, and a histogram of
 * the temperatures.
 *
 * Input: one reading per line, "tick temperature humidity", for example
 *     12  23.5  41.0
 * Blank lines and lines starting with '#' are ignored. A line that does not
 * hold three numbers is counted as skipped.
 *
 * Usage: ./readings [threshold] < data/normal.txt
 *        threshold is in degrees C, default 30.0
 *
 * This is the starter readings.c split into modules. main owns the arrays and
 * the printing; reading, stats and histogram do the work. The readings are
 * locals here and are passed to the functions that need them, so there are no
 * globals. Nothing prints except main and histogram_print.
 *
 * Tanner Robison.
 */
#include <stdio.h>
#include <stdlib.h>

#include "histogram.h"
#include "reading.h"
#include "stats.h"

int main(int argc, char *argv[]) {
    int   ticks[READING_MAX];
    float temps[READING_MAX];
    float hums[READING_MAX];
    float threshold = 30.0f;
    int   count, skipped;

    if (argc > 2) {
        fprintf(stderr, "usage: %s [threshold] < readings.txt\n", argv[0]);
        return 1;
    }
    if (argc == 2) {
        char *end;
        threshold = strtof(argv[1], &end);
        if (*end != '\0') {
            fprintf(stderr, "error: threshold '%s' is not a number\n", argv[1]);
            return 1;
        }
    }

    count = reading_read(stdin, ticks, temps, hums, READING_MAX, &skipped);

    printf("readings: %d\n", count);
    printf("skipped:  %d\n", skipped);
    if (count == 0) {
        printf("no readings, no summary\n");
        return 0;
    }

    /* The same three functions serve both columns: no repeated loop. */
    printf("temperature: min %6.1f  max %6.1f  mean %6.2f C\n",
           stats_min(temps, count), stats_max(temps, count), stats_mean(temps, count));
    printf("humidity:    min %6.1f  max %6.1f  mean %6.2f %%RH\n",
           stats_min(hums, count), stats_max(hums, count), stats_mean(hums, count));

    /* stats returns an index, so main can turn it back into a tick. */
    int start;
    int best = stats_longest_run_above(temps, count, threshold, &start);
    if (best == 0)
        printf("above %.1f C: never\n", threshold);
    else
        printf("above %.1f C: longest run %d readings, from tick %d to tick %d\n",
               threshold, best, ticks[start], ticks[start + best - 1]);

    histogram_print(temps, count);
    return 0;
}
