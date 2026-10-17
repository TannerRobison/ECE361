# AI Usage — Homework 3

Tanner Robison

## Tools used

**Claude Code (Claude Opus 5.5)**, run from the terminal in `hw03/`.

## What I used it for

This time I wrote most of the pointer library myself and used Claude as a tutor and a
reviewer, then handed it the tests, the `Makefile` and the `README`. The Shine asks about the
boxes and arrows behind my own functions, so the functions were the part I needed to write.

Written by me, reviewed by Claude:

- `swap_float`, `minmax` and `reverse` in `ptr.c`, and `ptr.h`: the prototypes, the include
  guard, and the comments for `swap_float` and `minmax`.
- After each attempt Claude read the file, compiled it with `-Wall -Wextra`, ran the function
  on small cases in a scratch file, and told me what was wrong without writing the fix. It
  caught `&` where I needed `*` in `swap_float`; a `minmax` that compared and assigned
  addresses instead of values, with both comparisons backwards and no starting value; and a
  `reverse` whose loop ran `n` times, so it swapped every pair back.
- Concepts it explained along the way: `&` versus `*`, traced with made-up addresses; when to
  pass `&x` and when to pass a pointer as it is; `p++` moving one whole `float`; `a + n` as
  one past the end; `const float *p` versus `float *const p`; and the two-pointer technique
  for `reverse`.

Designed by me, typed by Claude:

- `find_above`: I designed the loop and the match condition; Claude typed it into `ptr.c`,
  added the NULL guard and its comment in `ptr.h`, and removed my placeholder stub.

Written by Claude at my direction:

- `tests/test_ptr.c` (45 tests), the `Makefile` (modeled on my HW2 one), and `README.md`.
- The comments for `reverse` in `ptr.c` and `ptr.h`, and the final wording of the `minmax`
  comment in `ptr.h`.
- Claude ran `make`, `make test` and `make clean`, checked that editing `ptr.h` rebuilds both
  objects, and built the tests against a copy of `ptr.c` with two of my earlier bugs put back
  in, to show that the tests fail on them (10 failures, exit status 1).
- This file: Claude drafted it from the session, and I reviewed and edited it.

Every git commit stayed mine.

## One thing the model got wrong

When I asked Claude to write `find_above`, it edited `ptr.c` without reading it again first.
I had already added a placeholder at the bottom of the file,
`const float find_above(...) { return 0; }`, so its edit put the real function above my stub,
and the file then had two definitions of `find_above`. gcc refused it:
`conflicting types for 'find_above'`. Claude caught the error on its next compile and deleted
the stub, but it had changed a file without first looking at what was in it.
