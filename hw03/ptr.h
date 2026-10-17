#ifndef PTR_H
#define PTR_H

// swaps 2 float values, assumes the inputs are non-NULL pointers
void swap_float(float *a, float *b);

// stores the smallest and largest of a in min and max respectively
// returns 0, or -1 and changes nothing if n < 1 or any pointer is NULL
int minmax(const float *a, int n, float *min, float *max);

// returns a pointer to the first value in a[0..n-1] strictly greater than
// threshold, or NULL if there is none (or a is NULL); does not modify a
const float *find_above(const float *a, int n, float threshold);

// reverses a[0..n-1] in place by swapping from both ends toward the middle;
// does nothing if a is NULL or n < 2
void reverse(float *a, int n);
#endif
