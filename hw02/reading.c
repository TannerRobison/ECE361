/*
 * reading.c: reading and parsing the input lines.
 *
 * ECE 361, Fall 2026, HW2. Tanner Robison.
 */
#include <stdio.h>

#include "reading.h"

#define LINE_LEN 128

/*
 * True if the line carries no reading: it is blank, it is only whitespace, or
 * its first non-blank character is '#'. Private to this file; nothing outside
 * reading_read needs to know what counts as blank.
 */
static int is_skippable(const char *line) {
    int i = 0;
    while (line[i] == ' ' || line[i] == '\t')
        i++;
    return line[i] == '\n' || line[i] == '\0' || line[i] == '#';
}

int reading_read(FILE *in, int ticks[], float temps[], float hums[],
                 int max, int *skipped) {
    char line[LINE_LEN];
    int  count = 0;

    *skipped = 0;
    while (fgets(line, sizeof line, in) != NULL) {
        if (is_skippable(line))
            continue;
        if (count == max) {
            fprintf(stderr, "warning: more than %d readings, the rest are ignored\n", max);
            break;
        }
        if (sscanf(line, "%d %f %f", &ticks[count], &temps[count], &hums[count]) == 3)
            count++;
        else
            (*skipped)++;
    }
    return count;
}
