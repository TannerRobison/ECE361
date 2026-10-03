# ECE 361 — Homework 1

Tanner Robison

A small C library for number representation and bit manipulation, plus a decoder for a
16-bit thermostat status word built on top of it.

## What this library does

`bits.c` provides four generic operations on a `uint32_t`: printing it in binary, grabbing a
bit field out of it, setting a bit field into it, and reinterpreting a narrow field as a
two's complement number. Fields are described by a `(pos, width)` pair, where `pos` is the
number of the field's least significant bit and `width` is how many bits it spans. Bits are
numbered from 0 right to left.

`status.c` uses those functions to decode a packed 16-bit thermostat status word into a
`status_t` struct with one member per field. It contains no bit arithmetic of its own — every
field comes out through `get_field`, and the one signed field through `sign_extend`.

`bits.c` performs no I/O apart from `print_binary`, so it can be reused without
modification for other word formats.

## Build and test

```
make        # compiles each .c to its own .o
make test   # builds and runs tests/test_bits
make clean  # removes every build product
```

`CC` defaults to `clang`. The build is not compiler-specific — `make CC=gcc test` works
identically, and both compile with no warnings under `-std=c11 -Wall -Wextra`.

## The four functions

```c
void     print_binary(uint32_t x, int width);
uint32_t get_field(uint32_t word, int pos, int width);
uint32_t set_field(uint32_t word, int pos, int width, uint32_t value);
int32_t  sign_extend(uint32_t value, int width);
```

- **`print_binary`** prints the lowest `width` bits of `x`, most significant bit first, in
  groups of four separated by a space.
- **`get_field`** returns bits `pos` through `pos + width - 1` of `word`, shifted down to
  bit 0.
- **`set_field`** returns `word` with bits `pos` through `pos + width - 1` replaced by the
  lowest `width` bits of `value`. Every other bit is unchanged.
- **`sign_extend`** interprets the lowest `width` bits of `value` as a two's complement
  number and returns it as an `int32_t`. `sign_extend(0xF8, 8)` returns `-8`.

### `print_binary` output conventions

These are choices, not requirements, so they are stated here:

- No trailing space and no trailing newline. The caller controls line breaks.
- When `width` is not a multiple of four, the short group lands at the **front**, which is how
  a binary number is read aloud. `print_binary(0x16, 5)` prints `1 0110`.

## Valid ranges

An argument pair is valid if and only if:

- `width` is in the range 1 to 32,
- `pos` is in the range 0 to 31, and
- `pos + width <= 32`.

`sign_extend` and `print_binary` take no `pos` argurment, so only the `width` 
rule applies to them. All four functions share a single `check_arguments` helper, 
so the rule is enforced in exactly one place and cannot drift between functions.

## Behavior outside the valid range

| Function | Result for invalid arguments |
|---|---|
| `print_binary` | prints nothing |
| `get_field` | returns `0` |
| `set_field` | returns `word` unchanged |
| `sign_extend` | returns `0` |

**Why this behavior.** These functions return raw bit patterns, and every possible return
value is a legitimate result of some valid call — `0`, `0xFFFFFFFF`, and `-1` are all answers
a caller might genuinely be owed. There is therefore no spare value available to serve as an
error code, and the signatures have no out-of-band channel to report one through.

The alternative would be to let invalid arguments fall through into the arithmetic, but that
is not an option: a shift count that is negative, or greater than or equal to the width of the
type, is undefined behavior in C. With `pos` and `width` declared `int`, both are reachable
from a caller's mistake.

So the library takes a **documented no-op**: an invalid call does nothing and says so by
leaving the caller's data alone. `set_field` returning `word` unchanged is the important case —
a bad call is guaranteed not to corrupt the word it was given. The cost of this choice is that
errors are silent, which is acceptable here because the valid range is small, fixed, and
checkable by the caller before the call.

Each of these cases is exercised in `tests/test_bits.c`.

## Behavior at the boundaries

- **`width = 32`.** The usual mask `(1u << width) - 1` is undefined behavior at `width = 32`,
  because C does not define a shift by the full width of the type. On x86 it silently computes
  `1u << 0`, giving a mask of `0` rather than all ones — wrong answer, no crash, no warning.
  This library builds the mask by shifting down instead, `0xFFFFFFFFu >> (32 - width)`, whose
  shift count is 0 to 31 for every valid width and so never reaches the undefined case.
  `get_field(0xABCDEF12, 0, 32)` returns `0xABCDEF12`.
- **`width = 1`.** The field is nothing but a sign bit, so `sign_extend(1, 1)` is `-1` and
  `sign_extend(0, 1)` is `0`.
- **`pos = 31`.** Only `width = 1` is legal there, by the `pos + width <= 32` rule.
- **A `value` too wide for its field.** `set_field` masks `value` to its low `width` bits
  before shifting it into place, so excess bits cannot bleed into neighboring fields.
  `set_field(0xFFFFFFFF, 8, 4, 0x300)` returns `0xFFFFF0FF` — the `0x3` is discarded.
- **Most negative value.** `sign_extend(0x80000000, 32)` is `INT32_MIN`, and
  `sign_extend(0x80, 8)` is `-128`.

## The thermostat status word

`status_unpack` decodes a 16-bit word with this layout:

| Bits | Field | Meaning |
|---|---|---|
| 0 | HEAT | 1 = heater on |
| 1 | COOL | 1 = compressor on |
| 2 | FAN | 1 = fan on |
| 3 | FAULT | 1 = fault detected |
| 6–4 | MODE | 0 = OFF, 1 = HEAT, 2 = COOL, 3 = AUTO, 4 = FAN_ONLY; 5 to 7 are invalid |
| 7 | reserved | must be 0 |
| 15–8 | SETPOINT | set point in °C, 8-bit two's complement (−128 to 127) |

Every position and width is a named constant in `status.h` (`MODE_POS`, `MODE_WIDTH`, and so
on), so the layout is described in one place and `status.c` contains no numeric literals.

```c
status_t s = status_unpack(0x1631);
// s.setpoint == 22, s.mode == MODE_AUTO, s.heat == 1,
// s.cool == 0, s.fan == 0, s.fault == 0, s.reserved == 0
```

### How an invalid mode is reported

MODE values 5 to 7 have no meaning. When one appears, `status_unpack` sets `s.mode` to
**`MODE_INVALID` (0xFF)**. A legitimate 3-bit field can never produce `0xFF`, so the sentinel
is unambiguous and a caller tests for it with a single comparison.

The trade-off is that the raw value is not preserved: a caller learns that the mode was
invalid, but not whether it was 5, 6, or 7. That is the price of reporting in band rather than
adding a validity flag to `status_t`, and it is the right trade here because an invalid mode is
not actionable data — there is nothing a caller can do with a 6 that it could not do knowing
only that the field was bad.

### Two other decisions

- **`setpoint` is an `int8_t`**, which is exactly the −128 to 127 range the table specifies.
  It is the only field that goes through `sign_extend`, because it is the only field the table
  describes as two's complement. The other members are `uint8_t`.
- **`reserved` is decoded and stored as-is, not validated.** The table says it must be 0, but
  `status_unpack` is a pure decoder — it reports what the word contained and lets the caller
  decide what a set reserved bit means.

## Tests

`tests/test_bits.c` holds 51 assertions covering all four library functions and
`status_unpack`. It prints `PASS` or `FAIL` for each test, then a summary line, and exits with
a nonzero code if any test failed.

Coverage includes the boundaries listed above, all five rules in `check_arguments`, and three
status words: the example `0x1631`, a word with a negative set point and an invalid mode
(`0xF85E`), and one with the most negative set point, the reserved bit set, and every flag on
(`0x808F`).

`print_binary` returns `void` and writes to standard output, so it cannot be asserted against
without capturing a file descriptor. It is checked visually instead: the test program prints
the actual output next to the expected string. This is a real gap in the suite rather than a
hidden one, and it is the only function not automatically verified.
