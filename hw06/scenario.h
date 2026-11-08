#ifndef SCENARIO_H
#define SCENARIO_H

#include <stdint.h>
#include <stdio.h>
#include <time.h>

// A scenario is the script of what the world does: the start time, then
//  lines that say "from this tick on, the sensor reads this and the switches
//  are in this position". For example:
//
//      start 1791273600        # 2026-10-06 08:00:00 UTC
//      # tick  temp   rh    switches
//      0       37.5   52    0x0
//      5       37.5   52    0x1    # switch 0 pressed
//      6       37.5   52    0x0    # and released
//
//  Between lines the values hold: ticks 1 to 4 use the line for tick 0.

#define SCENARIO_MAX_LINES 1000

// one data line of a scenario file
typedef struct {
	long tick;          // seconds since the start
	float temp_c;       // temperature, degrees C
	float rh_pct;       // relative humidity, %
	uint32_t switches;  // the switch register
} scenario_line_t;

// a whole scenario, in a fixed-size array: no malloc
typedef struct {
	time_t start;                                // Unix timestamp of tick 0
	int count;                                   // data lines used
	scenario_line_t lines[SCENARIO_MAX_LINES];   // in increasing tick order
} scenario_t;

// reads a scenario from fp into *s. Comments (# to the end of the line) and
//  blank lines are skipped. The first other line must be "start <timestamp>";
//  every line after it must be "tick temp rh switches", with the switches in
//  hexadecimal and the ticks increasing.
//  Returns 0, or the number (from 1) of the first wrong line. A missing start
//  line or no data lines is reported as the line after the last one.
//  On failure the contents of *s are undefined
int scenario_load(FILE *fp, scenario_t *s);

// returns the line in force at tick: the last line whose tick is <= tick,
//  or NULL if tick comes before the first line
const scenario_line_t *scenario_at(const scenario_t *s, long tick);

// returns the tick of the last line, or -1 if there are no lines
long scenario_last_tick(const scenario_t *s);

#endif
