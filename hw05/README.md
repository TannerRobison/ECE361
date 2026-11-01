# ECE 361 Homework 5 — a combination lock as a table

Tanner Robison, Fall 2026.

A finite state machine for a keypad lock with the code 3-1-4, written as a transition table.
`lock_table[state][event]` holds the next state and the action for every pair, so `lock_step`
is one array lookup instead of a chain of `if`s. `lock` reads keypad presses from standard
input and prints every step.

## Build and test

```sh
make         # builds lock
make test    # builds tests/test_lock if needed, then runs it
make bonus   # builds tests/test_check if needed, then runs it (see BONUS.md)
make clean   # removes everything the build produced
```

Everything compiles with `gcc -std=c11 -Wall -Wextra` and no warnings. `tests/test_lock` prints
one `PASS` or `FAIL` line per test, grouped by what it tests, and a summary line
(`58 passed, 0 failed`). It exits 1 if any test fails, so `make test` fails too. The tests cover:

- `lock_step` for all 16 table entries, checking both the next state and the action.
- `lock_event` for a right and a wrong digit in LOCKED, GOT1 and GOT2, digits while OPEN, `c`
  and `t` in every state, and characters that are not events (`x`, space, newline, `C`, `T`).
- The sequences 3 1 4 (opens), 3 3 1 4 and 3 1 t 4 (do not open), and 3 1 4 c (locks again),
  checking the final state and the last action.
- `state_name`, `event_name` and `action_name` for every value, and `"?"` for one out of range.

## Running lock

```sh
./lock < data/codes.txt
```

`lock` starts in LOCKED and reads standard input one character at a time. A digit is a keypad
press, `c` means the door closed, and `t` means the 5-second timer ran out. Any other character,
such as a newline, is not an event and is skipped, so the state carries over from one line to
the next. Each event prints `state key event -> next action`, and the last line is
`final: STATE`.

Each line of `data/codes.txt` tries one rule of the lock:

| Line | What it shows |
| --- | --- |
| `314c` | the code opens the door, and closing the door locks it again |
| `3314` | a wrong digit in the middle of the code beeps and starts over, so 1 and 4 are then wrong too |
| `31t4` | the timer running out starts over silently; the 4 is then a wrong first digit |
| `3c14` | closing the door while a code is being typed changes nothing; the code still opens |
| `9t` | a digit does nothing while open; the timer running out locks the door |
| `c` | closing the door while locked changes nothing |
| `314` | opens, and nothing follows, so the run ends OPEN |

```
LOCKED 3 RIGHT -> GOT1 NONE
GOT1 1 RIGHT -> GOT2 NONE
GOT2 4 RIGHT -> OPEN UNLOCK
OPEN c CLOSE -> LOCKED LOCK
LOCKED 3 RIGHT -> GOT1 NONE
GOT1 3 WRONG -> LOCKED BEEP
LOCKED 1 WRONG -> LOCKED BEEP
LOCKED 4 WRONG -> LOCKED BEEP
LOCKED 3 RIGHT -> GOT1 NONE
GOT1 1 RIGHT -> GOT2 NONE
GOT2 t TIMEOUT -> LOCKED NONE
LOCKED 4 WRONG -> LOCKED BEEP
LOCKED 3 RIGHT -> GOT1 NONE
GOT1 c CLOSE -> GOT1 NONE
GOT1 1 RIGHT -> GOT2 NONE
GOT2 4 RIGHT -> OPEN UNLOCK
OPEN 9 WRONG -> OPEN NONE
OPEN t TIMEOUT -> LOCKED LOCK
LOCKED c CLOSE -> LOCKED NONE
LOCKED 3 RIGHT -> GOT1 NONE
GOT1 1 RIGHT -> GOT2 NONE
GOT2 4 RIGHT -> OPEN UNLOCK
final: OPEN
```

## State diagram

LOCKED is the start state. Each arrow is labeled with its event, and with its action when there
is one; an arrow with no action has `A_NONE`. "right" over the arrow into OPEN goes with
"UNLOCK" under it.

```
             wrong / BEEP, or timeout
     +------------------+-------------------+
     |                  |                   |
     v                  |                   |
+--------+  right   +--------+  right   +--------+   right   +--------+
| LOCKED |--------->|  GOT1  |--------->|  GOT2  |---------->|  OPEN  |
+--------+          +--------+          +--------+  UNLOCK   +--------+
     ^                                                           |
     |              close / LOCK, or timeout / LOCK              |
     +-----------------------------------------------------------+
```

Events that leave the state where it is:

- LOCKED: wrong / BEEP, close, timeout
- GOT1 and GOT2: close
- OPEN: right and wrong (digits do nothing while open)

## Transition table

Each cell is the next state and the action.

| state \ event | E_RIGHT | E_WRONG | E_CLOSE | E_TIMEOUT |
| --- | --- | --- | --- | --- |
| **LOCKED** | GOT1, NONE | LOCKED, BEEP | LOCKED, NONE | LOCKED, NONE |
| **GOT1** | GOT2, NONE | LOCKED, BEEP | GOT1, NONE | LOCKED, NONE |
| **GOT2** | OPEN, UNLOCK | LOCKED, BEEP | GOT2, NONE | LOCKED, NONE |
| **OPEN** | OPEN, NONE | OPEN, NONE | LOCKED, LOCK | LOCKED, LOCK |

## How it works

- **The table.** `lock_table` in `lock.c` is written with designated initializers
  (`[GOT1] = { [E_RIGHT] = {GOT2, A_NONE}, ... }`), so every entry is labeled with its state and
  event, and the order of the lines does not matter. An entry left out would be all zeros,
  `{LOCKED, A_UNDEFINED}`. That is why `A_UNDEFINED` is the first action: a forgotten entry shows
  up as UNDEFINED instead of passing for a real action, and its `lock_step` test fails.
- **`lock_step`** returns `lock_table[s][e].next` and stores `lock_table[s][e].action` through
  the `action` pointer, because a C function returns only one value. It has no `if` or `switch`:
  the table makes every decision.
- **`lock_event`** turns a character into an event. `c` and `t` mean the same in every state.
  For a digit, the state already says how many digits have been right so far (LOCKED is 0, GOT1
  is 1, GOT2 is 2), which is also the index of the digit expected next in `code[] = "314"`. So a
  digit is `E_RIGHT` if `s < OPEN && c == code[s]` and `E_WRONG` otherwise; in OPEN every digit
  is wrong, and the table keeps the door open with no action. `lock_event` returns `int`, not
  `event_t`, because -1 is not an `event_t` value. `main` skips -1 before it calls `lock_step`,
  which does no range checking.
- **The names** come from `const char *` arrays indexed by the enums, also written with
  designated initializers. The functions return pointers to string literals, not copies.
  Casting the enum to `unsigned` lets one comparison catch both -1 and values past the end,
  which return `"?"`.

## Files

| File | What it is |
| --- | --- |
| `lock.h`, `lock.c` | the types, the transition table, `lock_step`, `lock_event` and the name functions |
| `main.c` | runs the lock on the keys from standard input |
| `data/codes.txt` | key sequences that try each rule of the lock |
| `tests/test_lock.c` | unit tests for every table entry, `lock_event`, the sequences from the assignment, and the names |
| `check.h`, `check.c` | bonus: `fsm_check`, which finds left-out entries, next states out of range, unreachable states and states with no way out |
| `tests/test_check.c` | bonus: tests for `fsm_check` on the real table and on broken copies of it |
| `BONUS.md` | bonus: how the checker works, and its output on three broken tables |
| `Makefile` | `make`, `make test`, `make bonus`, `make clean` |
| `AI_USAGE.md` | what AI tools were used for, and one thing a model got wrong |
