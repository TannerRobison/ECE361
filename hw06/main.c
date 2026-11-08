// main.c: ECE 361, Fall 2026, HW6. Tanner Robison.
//
// ./replay scenario.txt
//
// Replays a scenario file through iom361 and prints one trace line per tick.
// The device is simple: a press of switch k toggles LED k.

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "iom361_r4.h"
#include "panel.h"
#include "scenario.h"
#include "sensor.h"

#define NUM_SWITCHES 4
#define NUM_LEDS     4

// static: about 16 KB, kept off the stack
static scenario_t scenario;

int main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "usage: %s scenario.txt\n", argv[0]);
		return 1;
	}

	// load the scenario
	FILE *fp = fopen(argv[1], "r");
	if (fp == NULL) {
		perror(argv[1]);
		return 1;
	}
	int bad_line = scenario_load(fp, &scenario);
	fclose(fp);
	if (bad_line != 0) {
		fprintf(stderr, "%s: line %d: not a valid scenario line\n",
			argv[1], bad_line);
		return 1;
	}

	// set up the I/O module: 4 switches, 4 LEDs, no LED display
	int rc = 0;
	uint32_t *base = iom361_initialize(NUM_SWITCHES, NUM_LEDS, &rc);
	if (base == NULL || rc != 0) {
		fprintf(stderr, "iom361_initialize failed: %d\n", rc);
		return 1;
	}
	iom361_set_display(false);

	// the switches start at 0 after iom361_initialize
	uint32_t before = 0;

	long last = scenario_last_tick(&scenario);
	for (long tick = 0; tick <= last; tick++) {
		// set the world from the line in force. Before the first line
		//  there is none, and the world keeps what it has
		const scenario_line_t *line = scenario_at(&scenario, tick);
		if (line != NULL) {
			_iom361_set_sensor1(line->temp_c, line->rh_pct);
			_iom361_set_switches(line->switches);
		}

		// the clock
		time_t now = scenario.start + (time_t)tick;

		// read the sensor and the switches through the registers
		reading_t r;
		rc = sensor_read(base, now, &r);
		if (rc != 0) {
			fprintf(stderr, "tick %ld: sensor_read failed: %d\n", tick, rc);
			return 1;
		}
		uint32_t switches = iom361_readReg(base, SWITCHES_REG, &rc);
		if (rc != 0) {
			fprintf(stderr, "tick %ld: switch read failed: %d\n", tick, rc);
			return 1;
		}

		// a press of switch k toggles LED k
		uint32_t pressed = panel_pressed(switches, before);
		for (int k = 0; k < NUM_LEDS; k++) {
			if (pressed & (1u << k)) {
				rc = panel_toggle(base, k);
				if (rc != 0) {
					fprintf(stderr, "tick %ld: panel_toggle(%d) failed: %d\n",
						tick, k, rc);
					return 1;
				}
			}
		}
		before = switches;

		uint32_t leds = iom361_readReg(base, LEDS_REG, &rc);
		if (rc != 0) {
			fprintf(stderr, "tick %ld: LED read failed: %d\n", tick, rc);
			return 1;
		}

		// the time in UTC, so a trace is the same on every computer
		char when[32];
		struct tm *tm = gmtime(&r.t);
		if (tm == NULL
		    || strftime(when, sizeof when, "%Y-%m-%d %H:%M:%S", tm) == 0) {
			fprintf(stderr, "tick %ld: can't format the time\n", tick);
			return 1;
		}

		printf("%s temp=%.1f rh=%.1f switches=0x%" PRIx32
		       " pressed=0x%" PRIx32 " leds=0x%" PRIx32 "\n",
		       when, r.temp_c, r.rh_pct, switches, pressed, leds);
	}

	return 0;
}
