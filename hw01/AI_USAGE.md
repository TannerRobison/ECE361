# AI Usage — Homework 1

Tanner Robison

## Tools used

**Claude Code (Claude Opus 5)**, run from the terminal in this directory.

## What I used it for

I used it mostly as a reviewer rather than a writer, because I have to explain every line of
this in the Shine.

Written by me:

- All four functions in `bits.c` (`print_binary`, `get_field`, `set_field`, `sign_extend`),
  including the `check_arguments` helper and the out-of-range policy.
- `bits.h`, `status.h`'s `status_t` struct, and the overall structure of `status_unpack`.

Written by Claude at my direction, after I had decided what it should do:

- The `Makefile`.
- The `test()` helper in `tests/test_bits.c` and the bodies of the test cases.
- The remaining field assignments in `status_unpack` and the position/width macros in
  `status.h`.

Used for review throughout: I asked it to read `bits.c` after each function and tell me
whether the implementation matched the spec. It caught several real bugs, including a loop
counter I had changed from `int32_t` to `uint32_t`, which made `i >= 0` always true and would
have looped forever.

## Something the model got wrong

I asked it to finish `status_unpack` following the struct I had already written. Instead of
just filling in the function, it rewrote `status.h` — adding a `mode_valid` member to
`status_t`, changing the `setpoint` type, and inserting a block of macros I had not asked for.

I found it by reading the diff before accepting the edit, and rejected it. The problem was not
that the suggestions were bad — I ended up wanting the macros and the `int8_t` anyway — but
that a struct member I had not designed would have been something I could not explain as my
own. I asked for the macros separately afterward, and reported the invalid mode with the
`MODE_INVALID` sentinel instead, which fits the struct I already had.

The lesson I took from it is that the model will quietly expand the scope of a request if the
larger change looks better to it, so the diff is worth reading every time rather than
accepting on the strength of the explanation.

## Something I had to fix in my own code

My first `sign_extend` computed `~value + 1` when the sign bit was set. That is *negation*,
not sign extension — it gave `-248` for `sign_extend(0xF8, 8)` instead of `-8`. I found it by
writing out the arithmetic by hand: `~0xF8 + 1` is `0xFFFFFF08`, which is `-248`. The correct
operation is to copy the sign bit upward, `value | ~mask`, which leaves the low bits alone and
fills everything above the field with ones.
