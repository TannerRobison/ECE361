# ECE 361 Homework 4 — the tally ADT

Tanner Robison, Fall 2026.

A tally counts how often each name occurs, such as the events a device sees. It is an abstract
data type: callers use it only through `tally.h`, and only `tally.c` knows what is inside a
`tally_t`. `events` reads event names from standard input and prints how often each one came up.

## Build and test

```sh
make         # builds events
make test    # builds tests/test_tally if needed, then runs it
make clean   # removes everything the build produced
```

Everything compiles with `gcc -std=c11 -Wall -Wextra` and no warnings. `make test` runs
`tests/test_tally`, which prints one `PASS` or `FAIL` line per test, grouped by what it tests,
and a summary line (`61 tests, 0 failed`). It exits nonzero if any test fails, so `make test`
fails too.

## Running events

```sh
./events < data/door.txt
```

`events` reads standard input a line at a time with `fgets` and removes the newline. It skips
blank lines and lines that start with `#`, and adds every other line to one tally. At the end it
prints the tally and `names: N, lines skipped: S`, where `S` counts the lines `tally_add`
refused. `data/door.txt` is a day of front-door sensor events. One of them,
`door_left_open_alert`, is 20 characters long, more than `TALLY_LEN`, so it is refused:

```
battery_low 1
door_closed 3
door_open 3
door_open_alarm 1
doorbell 1
knock 2
lock 3
motion 5
unlock 3
names: 9, lines skipped: 1
```

`data/full.txt` has 18 different names, two more than a tally can hold. `./events <
data/full.txt` prints the first 16 and `names: 16, lines skipped: 2`, and the repeated `ev01`
still counts after the tally is full.

A line too long for `events`' 128-byte buffer is read in more than one piece by `fgets`.
`events` throws away the rest of such a line, so it counts as one refused line, not several.

## The interface

| Function | What it does |
| --- | --- |
| `tally_t *tally_create(void)` | a new, empty tally; NULL if `TALLY_MAX` (4) tallies are already in use |
| `void tally_destroy(tally_t *t)` | gives the tally back so that `tally_create` can hand it out again; NULL does nothing |
| `int tally_add(tally_t *t, const char *name)` | counts one more `name` and returns its new count; returns −1 and changes nothing if `t` or `name` is NULL, `name` is empty or longer than `TALLY_LEN` (15), or `name` is new and the tally already holds `TALLY_NAMES` (16) names |
| `int tally_count(const tally_t *t, const char *name)` | how often `name` was added: 0 if never, or for NULL |
| `int tally_size(const tally_t *t)` | how many different names the tally holds: 0 for NULL |
| `void tally_print(const tally_t *t, FILE *out)` | one line per name, `name count`, in alphabetical (`strcmp`) order |

### Design notes

- **The inside.** `struct tally` holds `in_use`, `n` (how many names it holds) and
  `entries[TALLY_NAMES]`, where each entry is a `name[TALLY_LEN + 1]` and a `count`.
  `entries[0..n-1]` are always sorted by `strcmp`, so `tally_print` just walks them in order.
- **No `malloc`.** All the tallies there are live in `static tally_t pool[TALLY_MAX]` in
  `tally.c`. `tally_create` hands out the first one whose `in_use` is 0 and empties it;
  `tally_destroy` sets `in_use` back to 0. The pool is the only variable outside a function.
- **`find`**, a `static` helper, walks the sorted entries until it finds `name` or reaches the
  first entry that sorts after it. Either way the index it returns is where `name` is or belongs.
  `tally_add` and `tally_count` both use it.
- **Inserting a new name in the middle.** `tally_add` first checks that there is room, then
  moves `entries[i..n-1]` up by one, starting from the last. That entry moves into
  `entries[n]`, which is free, and every entry after it moves into the slot that was just
  emptied. Starting from `i` and going up would copy `entries[i]` over `entries[i + 1]`
  before `entries[i + 1]` had moved, so it would be lost. Adding `lock` to three names:

```
                                   [0]       [1]     [2]     [3]
find("lock") returns i = 1         doorbell  motion  unlock  (free)     n = 3
j = 3: entries[3] = entries[2]     doorbell  motion  unlock  unlock
j = 2: entries[2] = entries[1]     doorbell  motion  motion  unlock
copy "lock" into entries[1]        doorbell  lock    motion  unlock     n = 4
```

- **Copying names safely.** `tally_add` checks `strlen(name)` against `TALLY_LEN` before it
  touches the tally. Only then does it `memcpy` `len + 1` bytes, the `'\0'` included, into the
  entry's `name[TALLY_LEN + 1]`. A name that is too long is refused, not cut short.
- **Refusing changes nothing.** Every check in `tally_add` comes before the first write. A full
  tally is checked before anything moves, because shifting a full array would write one entry
  past its end.

## What the opaque type prevents

`tally.h` declares only `typedef struct tally tally_t;` and never says what is in a
`struct tally`. To see what that does, I compiled a file that includes only `tally.h` and tries
to read one of the members:

```c
#include "tally.h"

int main(void)
{
	tally_t *t = tally_create();
	int n = t->n;
	return n;
}
```

gcc 16.2 refuses it:

```
peek.c: In function ‘main’:
peek.c:6:18: error: invalid use of incomplete typedef ‘tally_t’ {aka ‘struct tally’}
    6 |         int n = t->n;
      |                  ^~
```

Outside `tally.c`, `struct tally` is an incomplete type. The compiler knows the name but not the
members or the size, so a caller can hold a `tally_t *` and pass it around, but cannot write
`t->n`, `t->entries[0]` or `sizeof(tally_t)`, and cannot declare a `tally_t` of its own. That
stops any code outside `tally.c` from breaking what the tally depends on. Nothing outside can
set `n` past the end of `entries`, unsort the entries, write a name without its `'\0'`, or make a
tally that is not in the pool. Every rule is enforced by the functions in one file, the one the
tests check. It also means callers do not depend on the layout. `tally.c` can swap the sorted
array for a tree, or rename `n`, and `events.c` and the tests compile unchanged, because the
compiler never let them see the inside. (The test file was only for this experiment and is not in
the repository; the code pushed here compiles.)

## Files

| File | What it is |
| --- | --- |
| `tally.h`, `tally.c` | the tally ADT |
| `events.c` | counts the event names on standard input |
| `data/door.txt` | a day of front-door sensor events, with comments, blank lines and a name that is too long |
| `data/full.txt` | 18 different names, to fill a tally |
| `tests/test_tally.c` | unit tests for every function: counting up, names never added, empty names, names of `TALLY_LEN` and `TALLY_LEN + 1` characters, NULL arguments, a full tally, the pool of `TALLY_MAX`, create after destroy, independent tallies, and print order |
| `Makefile` | `make`, `make test`, `make clean` |
| `AI_USAGE.md` | what AI tools were used for, and one thing a model got wrong |
