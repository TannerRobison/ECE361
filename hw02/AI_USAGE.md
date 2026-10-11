# AI Usage — Homework 2

Tanner Robison

## Tools used

**Claude Code (Claude Opus 5)**, run from the terminal in this directory.

## What I used it for

I kept the system design and delegated the programming. HW1 I did the other way around —
I wrote the functions myself and used Claude as a reviewer. This time the design was the part
worth owning, because the assignment hands you the module split and then grades you on the
interfaces: what each function takes, what it returns, and what it assumes.

Decided by me:

- The module boundaries. I broke the program into modules on paper in the Oct 8 studio; the
  four boxes and the one-way arrows in `README.md` are that diagram. I also drew the call
  stack for the recursive maximum there.
- The scope of this pass: Parts 1–3 only. `project/` waits until I have the iom361 folder in
  hand, and I deferred the bonus rather than start it before the required work was green.
- The delegation model itself. I told it to write the code and then produce a walkthrough of
  it, because I have to explain every line in the Shine and reading finished code is not the
  same as being able to defend it.
- The five rules on page 2 as hard constraints, and the verification I wanted before I
  approved the plan — in particular that it was not enough to pass `regress.sh`, since the
  four cases in `expected/` do not touch the `READING_MAX` cap or the threshold boundary.
- This file and every git commit stayed mine. Claude was told not to write either one.

Reviewed and approved, not authored by me:

- The function signatures — `reading_read`, the four `stats_*` functions, `histogram_print` —
  along with which helpers became `static` and how the `Makefile`'s dependency lines are
  written. Claude proposed all of this in a written plan; I read it and approved it before
  any code existed.

Written by Claude at my direction:

- `reading.[ch]`, `stats.[ch]`, `histogram.[ch]`, and `main.c`.
- The `Makefile`.
- `tests/test_stats.c`.
- `README.md`.

The check I cared about most, because rule 5 is the one that is easy to fail silently: the
original `readings.c` was compiled separately as a second binary, and both programs were run
side by side over nine input files, seven thresholds, and four bad-argument cases — 67 runs.
`stdout`, `stderr`, and the exit code were identical on every one. The extra inputs covered
what `expected/` does not: a 1005-reading file, a comment line arriving after the cap is hit,
a run ending at the final reading, and a threshold set exactly equal to a value in the data.

## Something the model got wrong

It wrote `tests/test_stats.c` with the 10,000-element array declared above `main`:

```c
static float big[BIG_N];

int main(void) {
```

That compiled with no warnings and all 24 tests passed. It is also exactly what rule 1
forbids — a variable at file scope — even though `static` keeps the symbol out of the link and
so nothing would ever have collided with it.

What makes this one worth reporting is how close it came to surviving. Claude had written a
`grep` check to verify the rules, run it, and reported that there were no globals. The check
scanned the four module files and not the test file, so it could not have found this no matter
what the answer was. I caught it reading `tests/test_stats.c` afterward. The fix was to move
the array inside `main`, where it is still `static` — but now that is a statement about
storage duration, keeping 40 KB out of the stack frame, rather than about scope.

Two things I took from it. A clean build and passing tests say nothing at all about a rule
that is about structure instead of behavior; nothing in `-Wall -Wextra` or in `regress.sh`
has an opinion about where a variable is declared. And a check the model writes and then
grades itself against is worth reading for what it leaves out, because a check that passes
looks the same whether it was thorough or whether it simply never looked.
