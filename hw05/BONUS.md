# Bonus — a transition table checker

Tanner Robison, Fall 2026.

`fsm_check(table, start, out)` in `check.c` looks for four kinds of problem in a transition table,
prints one line to `out` for each one, and returns how many it found:

| Problem | How it is found |
| --- | --- |
| an entry left out | its action is `A_UNDEFINED`. A left-out entry is all zeros, and `A_UNDEFINED` is the first action, so it is 0 |
| a next state out of range | `next` is not between 0 and `N_STATES - 1` |
| a state that cannot be reached from `start` | a breadth-first search over the table, from `start` |
| a state with no way out | every event leads back to that state |

**The breadth-first search.** `reached[]` marks the states found so far, and `queue[]` holds the
ones found but not yet looked at. It starts with only `start` marked and queued. Then, until the
queue is empty, it takes the next state out and looks at its row of the table: every next state
that is not marked yet is marked and added to the queue. Each state is added at most once, so
the queue needs only `N_STATES` slots. A state still unmarked at the end cannot be reached from
`start` by any sequence of events.

Entries that are left out or out of range are reported, and then the last two checks ignore
them, since they are not real transitions. A left-out entry is `{LOCKED, A_UNDEFINED}`, so
counting it would give every state a way out to LOCKED, and an out-of-range `next` would index
`reached[]` past its end.

```sh
make bonus   # builds tests/test_check and runs it
```

`tests/test_check` runs `fsm_check` on the real table (no problems) and on broken copies of it,
covering each kind of problem, a `start` that changes the answer, a left-out entry that must not
count as a way out, a table of all zeros (23 problems), and a `start` out of range. Under each
test it shows what the checker printed, then `PASS` or `FAIL` for the count, for one line per
problem, and for each line it should have printed (`30 passed, 0 failed`).

## Three broken tables

Each of these is `lock.c` with one change. The checker output is from running
`fsm_check(lock_table, LOCKED, stdout)` on the changed table, and the `lock` output is from
building `main.c` with it. The same three tables are tests in `tests/test_check.c`.

### 1. OPEN never locks again

```c
[OPEN] = {
    [E_RIGHT]   = {OPEN, A_NONE},
    [E_WRONG]   = {OPEN, A_NONE},
    [E_CLOSE]   = {OPEN, A_NONE},     /* was {LOCKED, A_LOCK} */
    [E_TIMEOUT] = {OPEN, A_NONE},     /* was {LOCKED, A_LOCK} */
},
```

```
no way out: every event leaves OPEN in OPEN
```

```
$ printf '314ct' | ./lock
...
GOT2 4 RIGHT -> OPEN UNLOCK
OPEN c CLOSE -> OPEN NONE
OPEN t TIMEOUT -> OPEN NONE
final: OPEN
```

**Why it is a real bug.** Once anyone types the code, the door stays unlocked for good: closing
it does not lock it, and neither does the timer. This is the worst way a lock can fail, and it
fails silently. Nothing beeps, and every entry is filled in with a real action, so a test that
only checks that the code opens the door still passes. Every entry looks reasonable on its own;
the bug only shows up when you look at a whole row, which is what "no way out" does.

### 2. OPEN + CLOSE left out

```c
[OPEN] = {
    [E_RIGHT]   = {OPEN, A_NONE},
    [E_WRONG]   = {OPEN, A_NONE},
                                      /* [E_CLOSE] line deleted */
    [E_TIMEOUT] = {LOCKED, A_LOCK},
},
```

```
left out: OPEN + CLOSE has action A_UNDEFINED
```

```
$ printf '314c' | ./lock
...
GOT2 4 RIGHT -> OPEN UNLOCK
OPEN c CLOSE -> LOCKED UNDEFINED
final: LOCKED
```

**Why it is a real bug.** The compiler fills the missing entry with zeros, `{LOCKED,
A_UNDEFINED}`, without a warning. The next state happens to be right, so the program believes the
door is locked, but the action that throws the bolt, `A_LOCK`, never happens. The software says
LOCKED while the door is unlocked. Because `A_UNDEFINED` is 0, a forgotten entry can never pass
for a real action, and the checker finds it without knowing anything about locks.

### 3. GOT2 + RIGHT goes to state 4

```c
[GOT2] = {
    [E_RIGHT]   = {4, A_UNLOCK},      /* was {OPEN, A_UNLOCK}: OPEN is the 4th state, counting from 1 */
    ...
},
```

```
out of range: GOT2 + RIGHT goes to state 4
unreachable: OPEN cannot be reached from LOCKED
```

```
$ printf '314' | ./lock
...
GOT2 4 RIGHT -> ? UNLOCK
final: ?
```

**Why it is a real bug.** OPEN is the fourth state, but its value is 3, because enums count from
0. gcc accepts a plain number as a `state_t` without a warning. (It does warn, with
`-Wenum-conversion` in `-Wextra`, if the two fields of an entry are swapped, because then one
enum type is used for another.) After the right code, UNLOCK still happens, but the lock goes to
state 4, which does not exist: `state_name` prints `?`. OPEN can no longer be reached, so the OPEN
row, the only place where closing the door or the timer locks it again, is never used. The next
key is worse. `lock_step` reads `lock_table[4][e]`, past the end of the table, so what the lock
does next depends on whatever memory follows it. Built with `-fsanitize=address,undefined`, the
next key (`printf '3141'`) stops the program at line 41 of `lock.c`, in `lock_step`:

```
lock.c:41:22: runtime error: index 4 out of bounds for type 'transition_t [4][4]'
==...==ERROR: AddressSanitizer: global-buffer-overflow on address ...
READ of size 4 at ... thread T0
```

The checker reports both problems: the bad entry itself, and the state that is now cut off.
