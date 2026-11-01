# AI Usage — Homework 5

Tanner Robison

## Tools used

**Claude Code (Claude Opus 5.5)**, run from the terminal in `hw05/`.

## What I used it for

Claude summarized the assignment and explained the table-driven approach: what designated
initializers are, how `[state][event] = {next, action}` fills a `transition_t`, why `lock_step`
returns the action through a pointer, and how the three name functions differ from the logic.

Written by me, reviewed by Claude:

- The types in `lock.h` and its include guard.
- `lock_table` in `lock.c`, all 16 entries. Claude's review found three entries that did not match
  the rules of the lock: `[GOT1][E_CLOSE]` and `[GOT2][E_CLOSE]` went back to LOCKED instead of
  changing nothing, and `[OPEN][E_TIMEOUT]` stayed OPEN instead of locking. It also found the
  missing commas between the rows. I fixed all of them.
- A first draft of `lock_event`, and the `code` string it indexes. Claude found that the draft
  did not compile (it had a condition in a `case` label) and that its digit check compared
  `code[s]` with itself instead of with `c`, so every digit counted as right and `777` would
  have opened the lock. Claude wrote the fixed version.

Written by Claude at my direction:

- `lock_step`, after explaining how the table lookup works.
- `state_name`, `event_name`, `action_name` and their name arrays, following the interface in
  `lock.h`, and the function comments in `lock.h`.
- `main.c`, `data/codes.txt`, `tests/test_lock.c` (58 tests), the `Makefile` (modeled on my HW4
  one), `README.md` with the ASCII state diagram, and a draft of this file.
- The bonus: `check.h` and `check.c` (`fsm_check` and its breadth-first search),
  `tests/test_check.c` (30 tests), the `make bonus` target, and `BONUS.md`, including the choice
  of the three broken tables.

How Claude checked its work:

- It ran `make`, `make test`, `make bonus` and `make clean`, and used `make -n` to show that
  touching `lock.h` rebuilds all five objects, touching `check.h` rebuilds only `check.o`,
  `tests/test_check.o` and the checker's test, and touching `lock.c` rebuilds only `lock.o` and
  the links.
- It ran the tests and `lock` under AddressSanitizer and UBSan, and also fed `lock` empty input,
  input with no events in it, and a last line with no newline.
- It built the tests against four broken copies of `lock.c` to show that the tests catch each
  bug: `[OPEN][E_TIMEOUT]` staying open, `[GOT1][E_CLOSE]` starting over, `lock_event` calling
  every digit right (the bug in my draft), and `[GOT2][E_RIGHT]` left out, so that it is
  `A_UNDEFINED`. Each one failed between 1 and 5 tests and made the test program exit 1.
- It built the checker's tests against four broken copies of `check.c`: left-out entries counted
  as transitions, the search always starting at LOCKED instead of `start`, `>` for `>=` in the
  range check, and a state that only loops back to itself counted as having a way out. Each one
  failed between 1 and 4 tests and made the test program exit 1.
- For `BONUS.md`, it ran `fsm_check` and `lock` on each of the three broken tables and copied the
  real output. For the out-of-range table it ran `lock` under AddressSanitizer and UBSan to show
  the read past the end of `lock_table`, and it compiled a scratch file to confirm that gcc warns
  about swapped fields in an entry but not about a plain number as a state.
- It checked the alignment of the ASCII state diagram with a script.

## One thing the model got wrong

The first time Claude checked that a failing test run exits nonzero, it printed the exit status
with `${PIPESTATUS[0]}`, which is bash syntax. Claude's commands run in zsh, where that expands to
nothing, so the check printed `exit status:` with no number, which proves nothing either way.
Claude noticed the blank and reran the check with `$?`, which showed 1. It was the same kind of
shell mismatch as in HW4.
