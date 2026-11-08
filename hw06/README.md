# ECE 361 Homework 6 — the sensor, the panel, and the replay loop

Tanner Robison, Fall 2026.

The part of the project that is the same for every device: reading the sensor and the switches
through iom361's registers, lighting LEDs without disturbing the others, keeping the clock, and
replaying a scenario file into a trace, one line per tick. The device here is simple: a press
of switch k toggles LED k.

## What it does

`./replay scenario.txt` reads a scenario, a script of what the world does:

```
start 1791273600                # tick 0 is 2026-10-06 08:00:00 UTC
# tick  temp   rh    switches
0       21.5   40    0x0
2       21.5   40    0x1        # switch 0 pressed
```

Lines starting with `#` (and anything after a `#`) are comments, and blank lines are skipped.
Between lines the values hold. For every tick from 0 to the last one, `replay` puts the line in
force into iom361, sets the clock to `start + tick`, reads the sensor and the switches back
through the registers, toggles LED k for every switch k just pressed, and prints:

```
2026-10-06 08:00:02 temp=21.5 rh=40.0 switches=0x1 pressed=0x1 leds=0x1
```

Times are printed in UTC with `gmtime` and `strftime`, so a trace is the same on every
computer. A scenario with a wrong line is rejected: `replay` prints the line number to `stderr`
and exits 1.

## Build and run

```sh
make                              # builds ./replay
./replay scenarios/basic.txt
make clean                        # removes everything the build produced
```

## Test

```sh
make test
```

That builds and runs `tests/test_hw06` (76 unit tests, one `PASS` or `FAIL` line each and a
summary) and then `sh regress.sh`, which runs `./replay` on every file in `scenarios/` and
`diff`s the trace against the file of the same name in `expected/`. Either one failing makes
`make test` fail.

| Scenario | What it covers |
| --- | --- |
| `basic.txt` | one switch pressed, held, released, and pressed again: LED 0 on, then off |
| `multi.txt` | all four switches, two pressed in the same tick, one held while another is pressed, 91.0 and -50.0 °C |
| `edges.txt` | a first line after tick 0, a switch above the four that exist, a long gap, and a date rollover |

## The modules

```
                  +--------------------------+
                  |          main.c          |
                  | the loop, the clock, and |
                  | the printing             |
                  +--+--------+---------+--+-+
          ___________|        |         |  |___________
         /                    |         |              \
        v                     v         v               v
+---------------+   +---------------+ +---------------+ +---------------+
|   scenario    |   |    sensor     | |    panel      | |  iom361_r4    |
| scenario.[ch] |   |  sensor.[ch]  | |  panel.[ch]   | |  (given)      |
+---------------+   +---------------+ +---------------+ +---------------+
| loads the     |   | reads TEMP_REG| | switch edges, | | the registers |
| file into a   |   | and HUMID_REG,| | LEDs by read- | | and the test  |
| fixed array;  |   | converts and  | | modify-write  | | functions     |
| the line in   |   | rounds to 0.1 | |               | | _iom361_set_* |
| force at tick |   |               | |               | |               |
+---------------+   +-------+-------+ +-------+-------+ +---------------+
                            |                 |                 ^
                            +-----------------+-----------------+
```

`sensor` and `panel` call only `iom361_readReg` and `iom361_writeReg`. Only `main.c` and the
tests call `_iom361_set_sensor1` and `_iom361_set_switches`: on a real board those would not
exist.

| Module | Files | Responsible for |
| --- | --- | --- |
| sensor | `sensor.h`, `sensor.c` | `reading_t` and `sensor_read` |
| panel | `panel.h`, `panel.c` | `panel_pressed`, `panel_released`, `panel_led`, `panel_toggle` |
| scenario | `scenario.h`, `scenario.c` | `scenario_load`, `scenario_at`, `scenario_last_tick` |
| main | `main.c` | the replay loop and the trace |

### Design notes

- **`sensor_read` reads both registers before it writes `*r`**, so a failed read leaves the
  reading exactly as it was. It keeps bits 19..0 of each register, and rounds in `double`
  before storing a `float`, so 91.0 °C reads back as exactly 91.0.
- **`panel_led` and `panel_toggle` read the LED register before they write it.** The register
  holds all 32 LEDs in one word; writing only the mask for LED n would turn every other LED off.
  `n` is checked before `1u << n`, because a shift by 32 or by a negative amount is undefined.
- **`panel_pressed` is `(now ^ before) & now`:** the bits that changed and are on now.
  `panel_released` is `(now ^ before) & before`. A held switch is neither.
- **No malloc.** `scenario_t` holds at most 1000 lines in a fixed array. It is about 16 KB,
  so `main` keeps it `static`.
- **`scenario_load` takes a `FILE *`**, so `main` handles a file that won't open and the tests
  can load from a temporary file. It returns 0 or the number of the first wrong line: the first
  non-comment line if it is not `start <timestamp>`, a line with a missing, extra, or non-numeric
  field, a tick that is negative or not larger than the one before, or the 1001st data line. A
  file with no `start` line or no data lines is reported as the line after the last one.
- **Error codes.** `sensor_read`, `panel_led` and `panel_toggle` return 0, -1 for a bad
  argument, or the iom361 return code of a failed register access.

## Files

| File | What it is |
| --- | --- |
| `main.c`, `sensor.[ch]`, `panel.[ch]`, `scenario.[ch]` | the program |
| `iom361_r4.[ch]` | the I/O module, unchanged |
| `Makefile` | `make`, `make test`, `make clean` |
| `tests/test_hw06.c` | unit tests for sensor, panel, and scenario |
| `regress.sh`, `scenarios/`, `expected/` | the regression test |
| `AI_USAGE.md` | how AI tools were used |
