/*
 * tests/test_hw06.c: unit tests for the sensor, panel, and scenario modules.
 *
 * Prints PASS or FAIL for each test and a summary, and exits nonzero if any
 * test failed, so that "make test" stops on a failure.
 *
 * ECE 361, Fall 2026, HW6. Tanner Robison.
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "../iom361_r4.h"
#include "../panel.h"
#include "../scenario.h"
#include "../sensor.h"

#define T0 ((time_t) 1791273600)     /* 2026-10-06 08:00:00 UTC */

static int total = 0;
static int fails = 0;

static void test_int(const char *name, long expected, long actual) {
    total++;
    if (expected == actual) {
        printf("PASS  %s\n", name);
    } else {
        fails++;
        printf("FAIL  %s: expected %ld, got %ld\n", name, expected, actual);
    }
}

static void test_hex(const char *name, uint32_t expected, uint32_t actual) {
    total++;
    if (expected == actual) {
        printf("PASS  %s\n", name);
    } else {
        fails++;
        printf("FAIL  %s: expected 0x%x, got 0x%x\n", name,
               (unsigned) expected, (unsigned) actual);
    }
}

/* the readings are rounded to 0.1, so they must equal the float exactly */
static void test_float(const char *name, float expected, float actual) {
    total++;
    if (expected == actual) {
        printf("PASS  %s\n", name);
    } else {
        fails++;
        printf("FAIL  %s: expected %.4f, got %.4f\n", name, (double) expected,
               (double) actual);
    }
}

static void test_true(const char *name, bool ok) {
    total++;
    if (ok) {
        printf("PASS  %s\n", name);
    } else {
        fails++;
        printf("FAIL  %s\n", name);
    }
}

/* loads text as a scenario through a temporary file; returns scenario_load's
   result, or -2 if the temporary file can't be made */
static int load_text(const char *text, scenario_t *s) {
    FILE *fp = tmpfile();
    if (fp == NULL)
        return -2;
    fputs(text, fp);
    rewind(fp);
    int r = scenario_load(fp, s);
    fclose(fp);
    return r;
}

static uint32_t leds(uint32_t *base) {
    return iom361_readReg(base, LEDS_REG, NULL);
}

static void test_sensor(uint32_t *base) {
    reading_t r;

    printf("\nsensor_read\n");

    _iom361_set_sensor1(91.0f, 40.0f);
    test_int("91.0 C: returns 0", 0, sensor_read(base, T0, &r));
    test_float("91.0 C reads back as 91.0", 91.0f, r.temp_c);
    test_float("40.0 %RH reads back as 40.0", 40.0f, r.rh_pct);
    test_int("the timestamp is now", (long) T0, (long) r.t);

    /* values just above and below a threshold must round to it exactly */
    _iom361_set_sensor1(37.2f, 52.0f);
    sensor_read(base, T0 + 5, &r);
    test_float("37.2 C reads back as 37.2", 37.2f, r.temp_c);
    test_float("52.0 %RH reads back as 52.0", 52.0f, r.rh_pct);
    test_int("the timestamp is now + 5", (long) (T0 + 5), (long) r.t);

    _iom361_set_sensor1(-50.0f, 0.0f);
    sensor_read(base, T0, &r);
    test_float("-50.0 C, the bottom of the range", -50.0f, r.temp_c);
    test_float("0.0 %RH, the bottom of the range", 0.0f, r.rh_pct);

    /* a failed read: base is not the I/O block, so iom361 returns 1 */
    reading_t before = {T0 + 99, 12.3f, 45.6f};
    reading_t after = before;
    uint32_t not_the_base[8] = {0};
    int rc = sensor_read(not_the_base, T0, &after);
    test_int("a wrong base returns iom361's code 1", 1, rc);
    test_true("a failed read changes nothing",
              memcmp(&before, &after, sizeof before) == 0);

    after = before;
    test_int("a NULL base returns -1", -1, sensor_read(NULL, T0, &after));
    test_true("a NULL base changes nothing",
              memcmp(&before, &after, sizeof before) == 0);
    test_int("a NULL reading returns -1", -1, sensor_read(base, T0, NULL));
}

static void test_edges(void) {
    printf("\npanel_pressed and panel_released\n");

    test_hex("a press: pressed", 0x1, panel_pressed(0x1, 0x0));
    test_hex("a press: released", 0x0, panel_released(0x1, 0x0));
    test_hex("a release: pressed", 0x0, panel_pressed(0x0, 0x1));
    test_hex("a release: released", 0x1, panel_released(0x0, 0x1));
    test_hex("a held switch: pressed", 0x0, panel_pressed(0x1, 0x1));
    test_hex("a held switch: released", 0x0, panel_released(0x1, 0x1));
    test_hex("nothing on: pressed", 0x0, panel_pressed(0x0, 0x0));

    /* 0101 -> 0110: switch 1 pressed, switch 0 released, switch 2 held */
    test_hex("mixed 0x5 -> 0x6: pressed", 0x2, panel_pressed(0x6, 0x5));
    test_hex("mixed 0x5 -> 0x6: released", 0x1, panel_released(0x6, 0x5));
    test_hex("switch 31: pressed", 0x80000000u, panel_pressed(0x80000000u, 0x0));
}

static void test_leds(uint32_t *base) {
    printf("\npanel_led and panel_toggle\n");

    iom361_writeReg(base, LEDS_REG, 0xA, NULL);              /* 1010 */
    test_int("panel_led(0, on) returns 0", 0, panel_led(base, 0, 1));
    test_hex("LED 0 on keeps LEDs 1 and 3", 0xB, leds(base));
    test_int("panel_led(3, off) returns 0", 0, panel_led(base, 3, 0));
    test_hex("LED 3 off keeps LEDs 0 and 1", 0x3, leds(base));
    panel_led(base, 1, 1);
    test_hex("LED 1 on when it is already on", 0x3, leds(base));
    panel_led(base, 2, 0);
    test_hex("LED 2 off when it is already off", 0x3, leds(base));

    iom361_writeReg(base, LEDS_REG, 0xA, NULL);              /* 1010 */
    test_int("panel_toggle(0) returns 0", 0, panel_toggle(base, 0));
    test_hex("toggle LED 0 keeps LEDs 1 and 3", 0xB, leds(base));
    panel_toggle(base, 1);
    test_hex("toggle LED 1 keeps LEDs 0 and 3", 0x9, leds(base));
    panel_toggle(base, 1);
    test_hex("toggle LED 1 twice is back where it was", 0xB, leds(base));

    iom361_writeReg(base, LEDS_REG, 0x5, NULL);
    test_int("panel_led, n = 32, returns -1", -1, panel_led(base, 32, 1));
    test_int("panel_led, n = -1, returns -1", -1, panel_led(base, -1, 1));
    test_int("panel_toggle, n = 32, returns -1", -1, panel_toggle(base, 32));
    test_int("panel_toggle, n = -1, returns -1", -1, panel_toggle(base, -1));
    test_hex("a bad n changes no LED", 0x5, leds(base));
    test_int("panel_led, NULL base, returns -1", -1, panel_led(NULL, 0, 1));
    test_int("panel_toggle, NULL base, returns -1", -1, panel_toggle(NULL, 0));

    uint32_t not_the_base[8] = {0};
    test_int("panel_led, wrong base, returns iom361's code 1", 1,
             panel_led(not_the_base, 0, 1));
    test_int("panel_toggle, wrong base, returns iom361's code 1", 1,
             panel_toggle(not_the_base, 0));
}

static void test_scenario(void) {
    static scenario_t s;          /* about 16 KB, kept off the stack */
    const scenario_line_t *line;

    printf("\nscenario_load and scenario_at\n");

    /* the example from the project sheet, with comments and blank lines */
    const char *good =
        "# a comment before the start line\n"
        "\n"
        "start 1791273600                # 2026-10-06 08:00:00 UTC\n"
        "# tick  temp   rh    switches\n"
        "0       37.5   52    0x0\n"
        "   \n"
        "5       37.5   52    0x1      # EGGS_IN pressed\n"
        "6       37.5   52    0x0      # and released\n"
        "\n"
        "60      38.5   52.5  0xF\n";
    test_int("a load with comments and blank lines returns 0", 0,
             load_text(good, &s));
    test_int("the start time", (long) T0, (long) s.start);
    test_int("4 data lines", 4, s.count);
    test_int("the last tick is 60", 60, scenario_last_tick(&s));
    test_float("line 2: temp", 37.5f, s.lines[1].temp_c);
    test_hex("line 2: switches in hex", 0x1, s.lines[1].switches);
    test_float("line 4: rh", 52.5f, s.lines[3].rh_pct);
    test_hex("line 4: switches 0xF", 0xF, s.lines[3].switches);

    /* values hold between lines */
    line = scenario_at(&s, 0);
    test_true("tick 0: the line for tick 0", line != NULL && line->tick == 0);
    line = scenario_at(&s, 4);
    test_true("tick 4 holds the line for tick 0", line != NULL && line->tick == 0);
    line = scenario_at(&s, 5);
    test_true("tick 5: the line for tick 5",
              line != NULL && line->tick == 5 && line->switches == 0x1);
    line = scenario_at(&s, 59);
    test_true("tick 59 holds the line for tick 6",
              line != NULL && line->tick == 6 && line->switches == 0x0);
    line = scenario_at(&s, 60);
    test_true("tick 60: the line for tick 60",
              line != NULL && line->tick == 60 && line->temp_c == 38.5f);
    line = scenario_at(&s, 1000);
    test_true("after the last line, it holds",
              line != NULL && line->tick == 60);

    test_int("a first line at tick 3 loads", 0,
             load_text("start 0\n3 20 40 0x0\n", &s));
    test_true("before the first line there is none", scenario_at(&s, 2) == NULL);
    test_true("at the first line there is one", scenario_at(&s, 3) != NULL);

    /* wrong files: the number of the first wrong line */
    test_int("no start line: the first data line, 2", 2,
             load_text("# no start\n0 37.5 52 0x0\n", &s));
    test_int("only comments: the line after the last, 3", 3,
             load_text("# only\n# comments\n", &s));
    test_int("an empty file: line 1", 1, load_text("", &s));
    test_int("start but no data lines: the line after the last, 2", 2,
             load_text("start 1791273600\n", &s));
    test_int("a start without a time: line 1", 1,
             load_text("start\n0 37.5 52 0x0\n", &s));
    test_int("a missing field: line 3", 3,
             load_text("start 1791273600\n0 37.5 52 0x0\n5 37.5 0x1\n", &s));
    test_int("a missing field after a comment: line 4", 4,
             load_text("start 0\n0 20 40 0x0\n# ok\n5 20 40   # no switches\n", &s));
    test_int("an extra field: line 2", 2,
             load_text("start 0\n0 20 40 0x0 7\n", &s));
    test_int("a field that is not a number: line 2", 2,
             load_text("start 0\n0 warm 40 0x0\n", &s));
    test_int("the same tick twice: line 3", 3,
             load_text("start 0\n0 20 40 0x0\n0 21 40 0x0\n", &s));
    test_int("a tick that goes back: line 4", 4,
             load_text("start 0\n0 20 40 0x0\n10 20 40 0x0\n9 20 40 0x0\n", &s));
    test_int("a negative tick: line 2", 2,
             load_text("start 0\n-1 20 40 0x0\n", &s));
    test_int("a second start line: line 3", 3,
             load_text("start 0\n0 20 40 0x0\nstart 5\n", &s));

    /* 1000 data lines fit; the 1001st is wrong */
    static char big[1002 * 20];
    char *p = big;
    p += sprintf(p, "start 0\n");
    for (int i = 0; i < SCENARIO_MAX_LINES; i++)
        p += sprintf(p, "%d 20 40 0x0\n", i);
    test_int("1000 data lines load", 0, load_text(big, &s));
    test_int("1000 data lines are counted", 1000, s.count);
    sprintf(p, "%d 20 40 0x0\n", SCENARIO_MAX_LINES);
    test_int("the 1001st data line, line 1002, is wrong", 1002,
             load_text(big, &s));
}

int main(void) {
    int rc = 0;
    uint32_t *base = iom361_initialize(4, 4, &rc);
    if (base == NULL || rc != 0) {
        printf("FAIL  iom361_initialize: %d\n", rc);
        return 1;
    }
    iom361_set_display(false);

    test_sensor(base);
    test_edges();
    test_leds(base);
    test_scenario();

    printf("\n%d tests, %d passed, %d failed\n", total, total - fails, fails);
    return fails == 0 ? 0 : 1;
}
