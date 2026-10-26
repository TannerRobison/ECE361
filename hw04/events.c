// events.c: counts the events named on standard input, one per line.
//
//   ./events < data/door.txt
//
// Blank lines and lines starting with # are skipped. Every other line is
// added to one tally, which is printed at the end, followed by
// "names: N, lines skipped: S", where S counts the lines tally_add refused.
//
// ECE 361, Fall 2026, HW4. Tanner Robison.

#include <stdio.h>
#include <string.h>

#include "tally.h"

// Far more than the TALLY_LEN characters plus '\n' and '\0' a name needs, so
// that a line too long for the buffer is also far too long to be a name.
#define LINE_SIZE 128

int main(void)
{
	tally_t *t = tally_create();
	if (t == NULL) {
		fprintf(stderr, "events: no tally available\n");
		return 1;
	}

	char line[LINE_SIZE];
	int skipped = 0;

	while (fgets(line, sizeof line, stdin) != NULL) {
		size_t end = strcspn(line, "\n"); // index of the '\n', if any
		if (line[end] == '\n') {
			line[end] = '\0';
		} else {
			// No '\n': this is the last line of a file that does not end
			// in one, or the line did not fit in line[]. Throw away the
			// rest of it, so that it is not read as a line of its own;
			// what did fit is too long, and tally_add will refuse it.
			int c;
			while ((c = getchar()) != '\n' && c != EOF) {
			}
		}

		if (line[0] == '\0' || line[0] == '#') {
			continue; // a blank line or a comment, not an event
		}
		if (tally_add(t, line) < 0) {
			skipped++; // too long, or a new name in a full tally
		}
	}

	tally_print(t, stdout);
	printf("names: %d, lines skipped: %d\n", tally_size(t), skipped);

	tally_destroy(t);
	return 0;
}
