#include <stddef.h>
#include <stdint.h>

#include "iom361_r4.h"
#include "panel.h"

uint32_t panel_pressed(uint32_t now, uint32_t before)
{
	// XOR leaves a 1 in every bit that changed, either way.
	//  A changed bit that is on now went from off to on
	uint32_t changed = now ^ before;
	return changed & now;
}

uint32_t panel_released(uint32_t now, uint32_t before)
{
	// a changed bit that was on before went from on to off
	uint32_t changed = now ^ before;
	return changed & before;
}

int panel_led(uint32_t *base, int n, int on)
{
	// argument check
	if (n < 0 || n > 31 || base == NULL) {
		return -1;
	}

	// create a mask
	uint32_t mask = (1u << n);

	// read
	int rc = 0;
	uint32_t led_panel = iom361_readReg(base, LEDS_REG, &rc);
	if (rc != 0) {
		return rc;
	}

	// modify
	if (on != 0) { // turn led on
		led_panel |= mask;
	} else { // turn led off
		led_panel &= ~mask;
	}

	// write
	iom361_writeReg(base, LEDS_REG, led_panel, &rc);
	if (rc != 0) {
		return rc;
	}

	return 0;
}

int panel_toggle(uint32_t *base, int n)
{
	// argument check
	if (n < 0 || n > 31 || base == NULL) {
		return -1;
	}

	// create a mask
	uint32_t mask = (1u << n);

	// read
	int rc = 0;
	uint32_t led_panel = iom361_readReg(base, LEDS_REG, &rc);
	if (rc != 0) {
		return rc;
	}

	// modify
	led_panel ^= mask;

	// write
	iom361_writeReg(base, LEDS_REG, led_panel, &rc);
	if (rc != 0) {
		return rc;
	}

	return 0;
}
