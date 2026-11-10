# ECE 361 Fall 2026 project 25: Egg incubator, specification

Tanner Robison, Fall 2026.

This file says what my device does where `ASSIGNMENT.md` leaves the choice to me. Open items are
marked `TODO (week N):`, where N is the week in the term plan when that piece lands.
`grep -n TODO SPEC.md` lists what is still open.

## 1. Device

Project 25, device type A (climate regulator). Key K4, second structure S1, report Q6.

The device is an incubator for chicken eggs. It holds temperature and humidity within narrow
limits, turns the eggs regularly, stops turning and raises the humidity for the last days, and
supports the hatch. As a type A device it holds its set points with hysteresis, so the actuators
do not chatter. The readings decide most transitions; the switches are disturbances and
overrides.

## 2. States

The states are given by the assignment.

| State | Meaning |
|---|---|
| `IDLE` | no eggs (initial state) |
| `WARMUP` | heating to incubation temperature |
| `INCUBATE` | holding temperature and humidity |
| `TURN` | turner motor running |
| `LOCKDOWN` | no turning, higher humidity |
| `HATCH` | chicks hatching |

TODO (week 5): one paragraph that defends or criticizes these states.

## 3. Inputs and outputs

`iom361_initialize(3, 3, &rc)`: 3 switches, 3 LEDs. Switch k and LED k are bit k of their
registers. A **level** switch means something for as long as it is on. A **press** is an event:
each change of its bit from 0 to 1 counts once. All three switches are presses.

| Bit | Switch | Kind | Meaning |
|---|---|---|---|
| 0 | `EGGS_IN` | press | eggs loaded |
| 1 | `LOCKDOWN_DAY` | press | day 18 reached |
| 2 | `HATCH_SEEN` | press | first chick hatched |

| Bit | LED | On means |
|---|---|---|
| 0 | `HEATER` | heater on |
| 1 | `HUMIDIFIER` | humidifier on |
| 2 | `TURNER` | turner motor on |

The RGB LED shows the state, one distinct color per state (section 11).

## 4. Operating rules

1. Hold 37.5 °C in every state but IDLE: heater on below 37.2 °C, off at 37.8 °C. WARMUP ends
   at 37.2 °C.
2. INCUBATE holds 50 to 55 %RH; LOCKDOWN and HATCH hold 65 to 70 %RH. The humidifier turns on
   below the band and off at its top.
3. In INCUBATE, TURN runs every 240 ticks for 6 ticks.

| Threshold | Value | Applies in |
|---|---|---|
| heater on below | 37.2 °C | every state but IDLE |
| heater off at | 37.8 °C | every state but IDLE |
| WARMUP ends at | 37.2 °C | WARMUP |
| humidifier on below / off at | 50 / 55 %RH | INCUBATE |
| humidifier on below / off at | 65 / 70 %RH | LOCKDOWN, HATCH |
| turn period / turn length | 240 / 6 ticks | INCUBATE, TURN |

Readings are rounded to 0.1 °C and 0.1 %RH before they are compared with a threshold, so a
scenario that sets 37.2 reads 37.2.

## 5. Events

TODO (week 5): list each event, with its source (a reading crossing a threshold, a switch
press, or the turn timer) and exactly when it fires.

## 6. Transition table

TODO (week 5): fill in every state and event pair.

| State | Event | Next state | Action |
|---|---|---|---|
| | | | |

## 7. State machine drawing

TODO (week 5): ASCII drawing, or an image in the repository.

## 8. Where event detection lives

The code that turns a reading and the switch register into events lives in `fsm.c`, next to the
transition table and `fsm_step`. It is one function that takes the current state, the reading,
the switch register for this tick and the last one, and the turn timer, and returns the set of
events that fired as a bitmask, one bit per `event_t`. `main` then steps the state machine once
for each event in the set, in enum order, which is priority order.

I put it there for four reasons:

1. **An event's meaning belongs with the table that uses it.** What `E_TEMP_REACHED` means
   (37.2 °C), what makes a press (a 0 to 1 change), and when `E_TURN_DUE` and `E_TURN_DONE`
   fire (240 and 6 ticks) are part of the state machine's definition, just like the
   `event_t` enum and the table. With all of them in `fsm.c`, changing an event means
   changing one file, and the table can be read with the definition of every event beside it.
2. **It can be tested on its own.** The function only computes: it reads no registers,
   prints nothing, and keeps no hidden state, because everything it needs is passed in. A unit
   test can feed it any reading, any pair of switch values and any timer value, and check the
   bits that come back, without iom361 and without running `main`. A test cannot call code
   that is buried in the loop in `main.c`.
3. **`main` stays the glue.** `main` owns the clock, the hardware and the replay: it reads the
   sensor and the switches, passes them in, runs `fsm_step`, carries out the actions, and
   writes the LEDs, the store and the stack. It never has to know what any threshold is.
4. **Detection depends on the state.** The turn timer only means something in INCUBATE and
   TURN. Knowledge about states already lives in `fsm.c`.

The cost is that `main` must keep the previous switch register and the turn timer and pass
them in on every tick, and that `fsm.h` includes `sensor.h` for `reading_t`. That include goes
one way only: `sensor.h` never includes `fsm.h`. I accept both, because the alternative, the
same values kept as `static` variables inside `fsm.c`, would make the function give different
answers to the same arguments and make it much harder to test.

## 9. Decisions where a rule was silent

TODO (weeks 5 and 6): answer each one, and name the scenario that tests it.

- [ ] LOCKDOWN_DAY during TURN.
- [ ] The temperature dropping during TURN.
- [ ] HATCH_SEEN before LOCKDOWN_DAY.
- [ ] Threshold sides: what happens at exactly 37.2 and exactly 37.8 °C, and at exactly the
      bottom and top of each humidity band? (Scenario 3.)
- [ ] Between the thresholds the heater and humidifier keep their last state. Where is that
      state kept: in the FSM, or in `main`?
- [ ] Humidity in WARMUP and TURN: rule 2 names only INCUBATE, LOCKDOWN and HATCH.
- [ ] Rule 3 timing: does the 240-tick count start when INCUBATE is first entered, or again
      after each TURN? Do the 6 TURN ticks count toward the next 240?
- [ ] The temperature drops below 37.2 °C in INCUBATE: back to WARMUP, or just heat?
- [ ] A press that means nothing in the current state, such as EGGS_IN outside IDLE or
      LOCKDOWN_DAY in WARMUP. (Scenario 5.)
- [ ] Is there any way back to IDLE? HATCH has no exit in the assignment.

## 10. Status word

One `uint32_t`, packed and unpacked with `get_field` and `set_field` from HW1. Unused bits are 0.

| Field | Width | Position |
|---|---|---|
| state | 3 | TODO (week 3) |
| `HEATER` | 1 | TODO (week 3) |
| `HUMIDIFIER` | 1 | TODO (week 3) |
| `TURNER` | 1 | TODO (week 3) |
| turn due | 1 | TODO (week 3) |

TODO (week 3): the value of each state in the state field.

## 11. RGB colors

| State | Red | Green | Blue |
|---|---|---|---|
| `IDLE` | | | |
| `WARMUP` | | | |
| `INCUBATE` | | | |
| `TURN` | | | |
| `LOCKDOWN` | | | |
| `HATCH` | | | |

TODO (week 5): one distinct color per state, each duty cycle 0 to 255.

## 12. Time

The device keeps time as a `time_t`: seconds since the Unix epoch, 1 January 1970, 00:00:00
UTC. The scenario file gives the start time, and the clock advances by one second per tick.
Every reading's timestamp is the start time plus the tick. Timestamps are broken down with
`gmtime()` and printed with `strftime()` as `2026-10-06 08:00:46`. All times are UTC, so a trace
is the same on every computer. Durations in the rules are differences between timestamps, in
seconds.

## 13. The year 2038

A signed 32-bit `time_t` holds at most 2^31 − 1 = 2147483647 seconds, which is
2038-01-19 03:14:07 UTC. One second later the count does not fit. Signed overflow is undefined
behavior in C; in practice the value wraps to −2^31, which is 1901-12-13 20:45:52 UTC.

For this device that means:

- every trace line after the wrap shows a date 136 years in the past;
- a scenario whose start time is after the limit cannot even be read into a `time_t`;
- every rule that uses a difference between timestamps breaks across the wrap. The difference
  comes out hugely negative, so "every 240 ticks" never comes due again and the eggs stop being
  turned.

On this machine (gcc, Linux, x86-64) `time_t` is 64 bits, which lasts about 292 billion years.

## 14. `record_t` and the store (key K4)

A `record_t` holds the reading, the state at that moment, and the status word.

The store is a binary search tree behind an opaque type, ordered by relative humidity quantized
to 1 %: 45.2 and 45.7 %RH have the same key. That example rules out rounding to the nearest
percent, which would put 45.7 in step 46. The comparison is one function, `record_compare`.

### `record_t`

```c
typedef struct {
	reading_t reading;   /* timestamp (time_t), temperature in °C, humidity in %RH */
	state_t state;       /* the state machine's state at that tick */
	uint32_t status;     /* the status word (section 10) */
} record_t;
```

The store keeps its own copy of each record, so the caller's record can be reused for the next
tick as soon as `store_insert` returns.

### Quantization

The key is the humidity truncated to a whole percent:

```c
key = (int)rh;
```

Casting a float to `int` drops the fraction. Humidity is never negative, so this is the same as
rounding down: 45.2 and 45.7 both give 45, and 45.9 gives 45.

At exactly 46.0 %RH the key is 46: a boundary value starts the step above it, and 46.0 and 46.9
share a key. That holds for real readings, not only in theory: `sensor.c` rounds every reading to
0.1 %RH, and a whole number such as 46.0 is stored exactly in a `float`, so it never comes back as
45.99999.

The truncation is in one private function, `rh_key`, used by both `record_compare` and
`store_find_by_key`, so the two can never disagree about which step a record is in.
`record_compare` returns −1, 0 or 1 when the first record's key is less than, equal to, or greater
than the second's.

### The tree

`store.c` defines two structs that no other file can see. A `struct node` holds one record and
its `left` and `right` pointers. `struct store` holds the `root` pointer and a `count`, so that
`store_count` is O(1) instead of a walk of the whole tree. An empty store has `root == NULL`.

| Operation | How | Cost |
|---|---|---|
| `store_insert` | loop down from the root: left if smaller, right if equal or larger; link at the first `NULL` | O(h) |
| `store_find_by_key` | the same path as insert; after a match, keep going right for duplicates | O(h) |
| `store_walk` | recursive, in order: left subtree, this node, right subtree | O(n) |
| `store_height` | recursive: 0 for `NULL`, else 1 + the taller side | O(n) |
| `store_destroy` | recursive, post-order: both subtrees first, then the node, then the store | O(n) |
| `store_count` | returns the stored count | O(1) |

Here h is the tree's height. An empty tree has height 0, and one record has height 1.

### Duplicate keys: equal keys go right

A record whose key equals a node's key goes into that node's right subtree, exactly as if it were
larger. Every node holds one record, and there is no special case for duplicates in the insert.

I chose this over the other two options:

- **A count in each node** loses information. Every record is a different tick with a different
  timestamp, temperature and state, and the report needs all of them. A count can only say how
  many readings there were, not which ones.
- **A list in each node** keeps every record, but it adds a second structure, with its own
  allocation and its own free, inside every node. That is more code and more ways to leak memory,
  for no gain in what the store can answer.
- **Equal keys go right** keeps every record, needs no extra code, and gives two useful
  properties for free:
  1. All records with the same key are in the right subtree of the first one, never in the
     left, because the insert only ever sends an equal key right. `store_find_by_key` finds the
     first match by the usual search, then keeps going right, and stops at `NULL`.
  2. Records with the same key come out in insertion order, which is timestamp order, from both
     `store_find_by_key` and `store_walk`. That is because each later duplicate is placed further
     right than the earlier ones.

The cost: this device makes many duplicates, because humidity changes slowly and a scenario can
hold one value for hundreds of ticks. Each duplicate goes right of the last, so a long run of
equal keys builds a chain, a tree with one node per level. Keys that only rise, or only fall, do
the same. A chain of n records has height n, so insert and find become O(n), like a linked list,
and the recursive walk, height and destroy go n calls deep. The unit tests show it on purpose:
100 records with the same key, or with keys in increasing order, give height 100.

I accept this. A scenario is a few hundred ticks, so the deepest chain is a few hundred nodes:
fast enough, and nowhere near the stack limit for the recursion. A self-balancing tree (AVL or
red-black) would keep the height near log₂ n, but it would roughly triple the size of `store.c`
and is outside this course.

### Tests

`tests/test_store.c` covers `record_compare` on its own (the same 1 % step, neighboring steps, and
46.0 on a boundary), insert into an empty tree, duplicates found in insertion order, a key that is
not there, the in-order walk, count and height after 1, 2 and 100 inserts, `NULL` arguments, and
destroying an empty tree. `make test` builds it with `-fsanitize=address,undefined`, so a leak or
a use after free fails the run.

## 15. The stack (S1)

The stack holds the event history, most recent on top: every turn and phase change, with its
tick. It is a dynamic array behind an opaque type that starts with room for 8 events and
doubles when it is full. `main` prints the last 10 events on exit.

TODO (week 7): why it doubles, and what that costs per push.

TODO (week 7): one paragraph that compares this stack with one alternative from the reading,
with the cost of each operation.

## 16. The report (Q6)

The report gives the first and the last tick at which the temperature was outside 37.0 to
38.0 °C, and the total number of such readings. If the condition never held, it reports nothing
and says so. It reads the store through `store.h` only.

TODO (week 9): the exact definition (is 37.0 °C itself outside?), and the exact output format,
including the line printed when the condition never held.
