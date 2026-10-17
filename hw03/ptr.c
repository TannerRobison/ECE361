#include "ptr.h"

#include <stdio.h>

// swaps 2 float values, assumes the inputs are non-NULL pointers
void swap_float(float *a, float *b)
{
	float temp;
	temp = *a;
	*a = *b;
	*b = temp;
}

// stores the smallest and largest of a in min and max respectively
int minmax(const float *a, int n, float *min, float *max)
{
	if (a == NULL || min == NULL || max == NULL || n < 1) {
		return -1;
	}

	*max = *a;
	*min = *a;
	// p++ automatically increments sizeof(float)
	for (const float *p = a; p < a + n; p++) {
		if (*p < *min) {
			*min = *p;
		}
		if (*p > *max) {
			*max = *p;
		}
	}
	return 0;
}

// returns a pointer to the first value in a greater than threshold, or NULL if
// none
const float *find_above(const float *a, int n, float threshold)
{
	if (a == NULL) {
		return NULL;
	}

	for (const float *p = a; p < a + n; p++) {
		if (*p > threshold) {
			return p;
		}
	}
	return NULL;
}

// reverses a[0..n-1] in place, does nothing if a is NULL or n < 2
void reverse(float *a, int n)
{
	if (a == NULL || n < 2) {
		return;
	}

	float *p = a;
	float *q = a + (n - 1); // points to last element of a
	for (; p < q; p++, q--) {
		swap_float(p, q);
	}
}
