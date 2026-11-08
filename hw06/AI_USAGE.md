# AI Usage — Homework 6

Tanner Robison

## Tools used

**Claude Code (Claude Opus 5.5)**, run from the terminal in `hw06/`.

## What I used it for

Claude read the assignment PDF and summarized the requirements. It explained what a scenario
file is and how `scenario.c` and `main.c` fit together, and how XOR finds the switches that
changed. I wrote the functions the Shine is most likely to ask about and had Claude review them.
I handed off the scenario parser, the replay loop and the tests.

Written by me, reviewed by Claude:

- `sensor_read`, ported from my project `sensor.c`. Claude compared it with the HW6 spec and
  found what had to change: the name, the field names, the timestamp passed in as `now` instead
  of read with `time()`, the mask for bits 19..0, and returning iom361's code instead of -1.
  Claude made those changes.
- `panel_led`. The first draft had five problems that Claude's review found:
  - `rc` was never declared.
  - It passed `*base` instead of `base`, so it handed the value of the switch register to
    `iom361_readReg` as an address.
  - There was no range check on `n` before `1u << n`.
  - It had no `return`.
  - It never checked the write's return code.

  The second draft was missing `#include <stddef.h>` for `NULL`. Its return codes also didn't
  match what `panel.h` promised. I fixed all of them.
- `panel_toggle`, which passed review the first time.
- The idea for `panel_pressed` and `panel_released`: XOR gives the switches that changed. Claude
  showed me how to finish it, with `& now` for pressed and `& before` for released, and wrote
  the two functions.

Written by Claude at my direction:

- `panel.h`, `scenario.h`, `scenario.c`, and `main.c`.
- The `Makefile` (modeled on my HW2 one) and `regress.sh` (my HW2 one, adapted).
- `tests/test_hw06.c`, with 76 tests.
- The three scenarios and their expected traces.
- `README.md`, and a draft of this file.

How Claude checked its work:

- `make` builds with no warnings under `-Wall -Wextra -Wpedantic`, and `make test` passes.
- It built the unit tests against ten broken copies of the code. Each one was rebuilt and run.
  The bugs:
  - `panel_led` writing only the mask, with no read first.
  - `panel_toggle` setting the bit instead of flipping it.
  - `panel_pressed` returning `now`.
  - `panel_released` using `now`.
  - `sensor_read` without rounding.
  - `sensor_read` writing the timestamp before a read that fails.
  - `scenario_load` accepting a repeated tick.
  - `scenario_load` accepting an extra field.
  - `scenario_at` picking the wrong line.
  - A limit of 999 lines instead of 1000.

  Every one failed at least one test and made the test program exit 1.
- For the expected traces, Claude wrote what each scenario should do in its comments, such as
  "LED 0 on at tick 2, off again at tick 8". It ran `replay` and compared every line with those
  comments before saving the output as `expected/`. I checked `expected/basic.txt` by hand.
  It also changed one character of an expected trace and confirmed that `make test` then
  failed.

## One thing the model got wrong

At first, the broken copy that should have accepted an extra field passed every test. That made
it look as if the tests could not catch a line like `0 20 40 0x0 7`. The problem was the broken
copy, not the tests. Claude's `sed` command meant to remove the check for leftover characters
didn't match, so the only change it made was `!= 4` to `< 3`. That change is harmless, because
the leftover-characters check still rejects a line with only three fields. So the "broken" copy
was really a working one. Claude noticed, removed the check properly, and that copy failed the
"an extra field" test.

It is the same lesson as in HW2, from the other direction. A check that passes means something
only if it is checking the thing you think it is.
