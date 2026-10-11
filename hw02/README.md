# ECE 361 Homework 2 — functions, recursion, and modules

Tanner Robison, Fall 2026.

The starter `readings.c` split into four modules plus `main.c`. The structure changed; the
output did not. Every byte the program prints is the same as before.

## What it does

Reads sensor readings from standard input, one per line, `tick temperature humidity`:

```
0       24.7   68.5
60      25.0   68.8
```

Blank lines and lines whose first non-blank character is `#` are ignored. Any other line that
does not hold three numbers is counted as skipped. The program prints the number of readings,
the minimum, maximum, and mean of both columns, the longest run of readings above a
temperature threshold, and a histogram of the temperatures.

## Build and run

```sh
make                              # builds ./readings
./readings < data/normal.txt      # default threshold, 30.0 C
./readings 25 < data/edge.txt     # threshold in degrees C
make clean                        # removes everything the build produced
```

## Test

```sh
make test
```

That builds and runs `tests/test_stats` (unit tests for the stats module, one `PASS` or `FAIL`
line per test and a summary) and then `sh regress.sh`, which runs `./readings` on every file in
`data/` and `diff`s the output against `expected/`. Either one failing makes `make test` fail.

## The modules

```
                       +-----------------+
                       |     main.c      |
                       |                 |
                       | the command line,
                       | the arrays, and |
                       | the printing    |
                       +--+-----+-----+--+
             ______________|     |     |______________
            /                    |                    \
           v                     v                     v
  +------------------+  +------------------+  +------------------+
  |     reading      |  |      stats       |  |    histogram     |
  |  reading.[ch]    |  |   stats.[ch]     |  |  histogram.[ch]  |
  +------------------+  +------------------+  +------------------+
  | reads and parses |  | min, max, mean,  |  | prints the       |
  | the input lines: |  | and the longest  |  | histogram: 20    |
  | skips comments,  |  | run above a      |  | bins, 5 C wide,  |
  | blank lines, and |  | threshold, on    |  | empty bins left  |
  | bad lines        |  | any float array  |  | out              |
  +------------------+  +------------------+  +------------------+
```

The arrows go one way only. `main` calls all three; none of the three calls another, and none
of them calls back into `main`. There is no cycle to break.

| Module | Files | Responsible for |
| --- | --- | --- |
| reading | `reading.h`, `reading.c` | reading and parsing the input lines |
| stats | `stats.h`, `stats.c` | minimum, maximum, mean, and the longest run above a threshold |
| histogram | `histogram.h`, `histogram.c` | printing the histogram |
| main | `main.c` | the command line, calling the modules, and printing the results |

### Design notes

- **No globals.** `ticks`, `temps`, `hums`, `count`, `skipped` and `threshold` are locals in
  `main` and are passed to the functions that need them.
- **One recursive maximum.** `stats_max(const float a[], int n)` is the only maximum in the
  program, and it recurses: the larger of the last element and the maximum of the rest. It is
  called twice, once for the temperatures and once for the humidities, so the duplicated loop
  in the starter is gone. The same is true of `stats_min` and `stats_mean`.
- **`stats_longest_run_above` returns an index, not a tick.** The stats module never sees the
  tick column; `main` turns the index back into a tick for the output line.
- **Helpers are `static`.** `is_skippable` in `reading.c` and `bin_index` in `histogram.c` are
  private to their file. Everything a header declares carries its module's prefix.
- **Only `main.c` and `histogram.c` print.** `reading.c` writes one warning to `stderr` when
  the input runs past `READING_MAX`; `stats.c` prints nothing at all.

## Files

| File | What it is |
| --- | --- |
| `main.c`, `reading.[ch]`, `stats.[ch]`, `histogram.[ch]` | the program |
| `Makefile` | `make`, `make test`, `make clean` |
| `tests/test_stats.c` | unit tests for the stats module |
| `regress.sh`, `data/`, `expected/` | the regression test, from the starter, unchanged |
| `readings.c` | the original one-file version, kept untouched for reference. Nothing in the `Makefile` compiles it. |
