#include <ctype.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "scenario.h"

#define LINE_LEN 256

// returns 1 if line holds only whitespace
static int is_blank(const char *line)
{
	for (; *line != '\0'; line++) {
		if (!isspace((unsigned char)*line)) {
			return 0;
		}
	}
	return 1;
}

int scenario_load(FILE *fp, scenario_t *s)
{
	if (fp == NULL || s == NULL) {
		return -1;
	}

	char buf[LINE_LEN];
	int lineno = 0;
	int have_start = 0;
	s->count = 0;

	while (fgets(buf, sizeof buf, fp) != NULL) {
		lineno++;

		// a line that didn't fit in buf has no newline (unless it's the last)
		if (strchr(buf, '\n') == NULL && !feof(fp)) {
			return lineno;
		}

		// cut off a comment, then skip what's left if it's blank
		char *hash = strchr(buf, '#');
		if (hash != NULL) {
			*hash = '\0';
		}
		if (is_blank(buf)) {
			continue;
		}

		// %n stores how many characters were used, so buf[used] must be
		//  the end of the line: anything left over is an extra field
		int used = 0;

		// the first line that is not a comment gives the start time
		if (!have_start) {
			long long start;
			if (sscanf(buf, " start %lld %n", &start, &used) != 1
			    || buf[used] != '\0') {
				return lineno;
			}
			s->start = (time_t)start;
			have_start = 1;
			continue;
		}

		// every line after it: tick temp rh switches
		if (s->count == SCENARIO_MAX_LINES) {
			return lineno;
		}
		scenario_line_t ln;
		if (sscanf(buf, " %ld %f %f %" SCNx32 " %n",
			   &ln.tick, &ln.temp_c, &ln.rh_pct, &ln.switches, &used) != 4
		    || buf[used] != '\0') {
			return lineno; // a missing, extra, or malformed field
		}

		// ticks start at 0 or later and must increase
		if (ln.tick < 0) {
			return lineno;
		}
		if (s->count > 0 && ln.tick <= s->lines[s->count - 1].tick) {
			return lineno;
		}

		s->lines[s->count] = ln;
		s->count++;
	}

	// no start line, or nothing after it
	if (!have_start || s->count == 0) {
		return lineno + 1;
	}

	return 0;
}

const scenario_line_t *scenario_at(const scenario_t *s, long tick)
{
	if (s == NULL) {
		return NULL;
	}

	// lines are in increasing tick order: walk forward until the next line
	//  starts after tick. The last one passed is in force
	const scenario_line_t *in_force = NULL;
	for (int i = 0; i < s->count; i++) {
		if (s->lines[i].tick > tick) {
			break;
		}
		in_force = &s->lines[i];
	}
	return in_force;
}

long scenario_last_tick(const scenario_t *s)
{
	if (s == NULL || s->count == 0) {
		return -1;
	}
	return s->lines[s->count - 1].tick;
}
