# ECE 361 Homework 3 — pointers

Tanner Robison, Fall 2026.

A small pointer library, `ptr.[ch]`: four functions that take addresses, so that they can change
the caller's variables or hand back more than one result, and that walk arrays with pointer
arithmetic instead of indexes. There is no program to run; HW3 is the library and its tests.

## Build and test

```sh
make         # builds ptr.o and tests/test_ptr
make test    # builds if needed, then runs the unit tests
make clean   # removes everything the build produced
```

Everything compiles with `gcc -std=c11 -Wall -Wextra` and no warnings. `make test` runs
`tests/test_ptr`, which prints one `PASS` or `FAIL` line per test, grouped by function, and a
summary line (`45 tests, 0 failed`). It exits nonzero if any test fails, so `make test` fails too.

## The functions

| Function | What it does |
| --- | --- |
| `void swap_float(float *a, float *b)` | exchanges `*a` and `*b` |
| `int minmax(const float *a, int n, float *min, float *max)` | stores the smallest and largest of `a[0..n-1]` in `*min` and `*max` and returns 0; returns −1 and changes nothing if `n < 1` or any pointer is NULL |
| `const float *find_above(const float *a, int n, float threshold)` | returns a pointer to the first value strictly greater than `threshold`, or NULL if there is none |
| `void reverse(float *a, int n)` | reverses `a[0..n-1]` in place; does nothing if `a` is NULL or `n < 2` |

### Design notes

- **Pointer arithmetic only.** There is no `a[i]` in `ptr.c`. Each loop walks a pointer `p` from
  `a` up to `a + n`, the address one past the last element, which is compared against but never
  dereferenced. `p++` moves one whole `float`, not one byte.
- **`const` where a function only reads.** `minmax` and `find_above` take `const float *a` and
  walk it with a `const float *p`, so the compiler rejects any write into the caller's array.
  `p++` is still allowed: the `const` is on the float, not on the pointer. `find_above` returns a
  `const float *` for the same reason.
- **`minmax` reports failure apart from the answer.** Any float it stored could be a real
  minimum or maximum, so a failure returns −1 instead, and the check comes before anything is
  written. On success both results start from `*a`, so they do not depend on whatever the caller
  had in `*min` and `*max`.
- **`find_above` returns a pointer into the caller's array**, the element itself rather than a
  copy of its value, so `r - a` is its index.
- **`reverse` uses two pointers that move toward each other**, calling `swap_float(p, q)` while
  `p < q`. On an odd length they meet on the middle element, which stays put; on an even length
  they cross.

```
  1  2  3  4  5
  p           q      swap 1 and 5   ->  5  2  3  4  1
     p     q         swap 2 and 4   ->  5  4  3  2  1
       p=q           they meet on the middle: stop
```

## Files

| File | What it is |
| --- | --- |
| `ptr.h`, `ptr.c` | the pointer library |
| `tests/test_ptr.c` | unit tests for all four functions, including `n = 0`, `n < 0`, NULL pointers, and values equal to the threshold |
| `Makefile` | `make`, `make test`, `make clean` |
| `AI_USAGE.md` | what AI tools were used for, and one thing a model got wrong |
