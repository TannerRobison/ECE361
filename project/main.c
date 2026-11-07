#include <stdint.h>
#include <stdio.h>

#include "iom361_r4.h"

int main(void)
{
	int rc;
	uint32_t *base = iom361_initialize(3, 3, &rc); /* 3 switches, 3 LEDs */
	if (base == NULL) {
		fprintf(stderr, "iom361_initialize failed: %d\n", rc);
		return 1;
	}

	_iom361_set_sensor1(37.5f, 52.0f); /* the "world": 37.5 C, 52 %RH */
	_iom361_set_switches(0x1);	   /* the "world": EGGS_IN on */

	uint32_t raw = iom361_readReg(base, TEMP_REG, &rc);
	double temp = raw / 1048576.0 * 200.0 - 50.0; /* AHT20 formula */
	uint32_t sw = iom361_readReg(base, SWITCHES_REG, &rc);

	if (sw & 0x1u) /* EGGS_IN is switch 0 */
		iom361_writeReg(base, LEDS_REG, 0x1u,
				&rc); /* HEATER is LED 0 */
	iom361_writeReg(base, RGB_LED_REG, build_rgb_reg(1, 255, 0, 0),
			&rc); /* red */

	printf("raw 0x%05X = %.4f C, switches 0x%X\n", (unsigned)raw, temp,
	       (unsigned)sw);
	return 0;
}
