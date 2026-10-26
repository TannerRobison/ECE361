# AI Usage — Homework 4

Tanner Robison

## Tools used

**Claude Code (Claude Opus 5.5)**, run from the terminal in `hw04/`.

## What I used it for

Written by me, reviewed by Claude:

- `tally.h` and `tally.c`: the interface comments, the pool, `find`, the shift in `tally_add`,
  and the length check before the `memcpy`.

Written by Claude at my direction:

- `events.c`, `data/door.txt` and `data/full.txt`.
- `tests/test_tally.c` (61 tests), the `Makefile` (modeled on my HW3 one), `README.md`, and a
  draft of this file.

How Claude checked its work:

- It ran `make`, `make test` and `make clean`, and used `make -n` to show that touching
  `tally.h` rebuilds all three objects while touching `tally.c` rebuilds only `tally.o` and the
  links.
- It ran the tests and `events` under AddressSanitizer and UBSan. It also fed `events` lines of
  127 and 300 characters, a last line with no newline, and empty input.
- It built the tests against seven broken copies of `tally.c` to show that the tests catch each
  bug: shifting from `i` upward, no full check, `>=` for `>` in the length check, empty names
  allowed, `destroy` not freeing the slot, `create` not emptying the tally, and no sorting.
  Every one failed between 2 and 17 tests and made `make test` exit 1.
- For Part 3 it compiled a scratch file that includes only `tally.h` and reads `t->n`, copied
  gcc's real error into the README, and deleted the file.

## One thing the model got wrong

To test long input lines, Claude built the 300-character test strings with fish shell syntax,
`(string repeat -n 300 x)`. My terminal runs fish, but Claude's commands run in a different
shell, which treated the parentheses as a filename pattern and printed `no matches found`.
`events` then read empty input and printed `names: 0, lines skipped: 0`, which looks like a
normal run. Claude noticed that the output could not be right for that input and reran the test
with portable shell. Had it only checked that `events` did not crash, the long-line case would
have looked tested when it never ran.
