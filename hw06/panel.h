#ifndef PANEL_H
#define PANEL_H

#include <stdint.h>

// returns the switches that went from off to on between before and now,
//  one bit per switch
uint32_t panel_pressed(uint32_t now, uint32_t before);

// returns the switches that went from on to off between before and now,
//  one bit per switch
uint32_t panel_released(uint32_t now, uint32_t before);

// turns LED n on (on != 0) or off (on == 0) by read-modify-write of the LED
//  register, so that every other LED keeps its state.
//  Returns 0 on success, -1 if n is not 0 to 31, or the iom361 return code
//  of a failed register access
int panel_led(uint32_t *base, int n, int on);

// toggles LED n by read-modify-write of the LED register, so that every
//  other LED keeps its state. Returns the same codes as panel_led
int panel_toggle(uint32_t *base, int n);

#endif
