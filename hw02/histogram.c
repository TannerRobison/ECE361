/*
 * histogram.c: printing the histogram.
 *
 * ECE 361, Fall 2026, HW2. Tanner Robison.
 */
#include <stdio.h>

#include "histogram.h"

#define NUM_BINS  20           /* 0 to 100 in 5 C bins */
#define BIN_WIDTH 5.0f

/*
 * The bin a value belongs in. Values below 0 go in the first bin and values
 * at or above 100 in the last, so every value lands somewhere. Private: the
 * bin layout is this module's business.
 */
static int bin_index(float v) {
    int b = (int) (v / BIN_WIDTH);
    if (v < 0.0f)
        b = 0;
    if (b >= NUM_BINS)
        b = NUM_BINS - 1;
    return b;
}

void histogram_print(const float a[], int n) {
    int bins[NUM_BINS] = {0};

    for (int i = 0; i < n; i++)
        bins[bin_index(a[i])]++;

    printf("histogram:\n");
    for (int b = 0; b < NUM_BINS; b++) {
        if (bins[b] == 0)
            continue;
        printf("  %5.1f to %5.1f C | ", b * BIN_WIDTH, (b + 1) * BIN_WIDTH);
        for (int k = 0; k < bins[b]; k++)
            putchar('*');
        printf(" %d\n", bins[b]);
    }
}
