#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

#include "iom361_r4.h"
#include "sensor.h"

// the sensor values are 20-bit unsigned numbers in bits[19:0]
#define SENSOR_MASK 0xFFFFFu

int sensor_read(uint32_t *base, time_t now, reading_t *r)
{
	// input validation
	if (base == NULL || r == NULL) {
		return -1;
	}

	// read the temperature and humidity registers before touching *r,
	//  so a failed read leaves it unchanged
	int rc = 0;
	uint32_t raw_temp = iom361_readReg(base, TEMP_REG, &rc);
	if (rc != 0) {
		return rc;
	}
	uint32_t raw_humidity = iom361_readReg(base, HUMID_REG, &rc);
	if (rc != 0) {
		return rc;
	}

	// keep only bits 19..0
	raw_temp &= SENSOR_MASK;
	raw_humidity &= SENSOR_MASK;

	// convert to appropriate values, and store
	// TEMPERATURE: ST / 2^20 * 200 - 50
	// 1048576.0 is 2^20. Using floating-point ensures no integer
	// truncation.
	double temp_c = ((double)raw_temp / 1048576.0) * 200.0 - 50.0;
	r->temp_c = (float)(round(temp_c * 10.0) / 10.0);

	// HUMIDITY: SRH / 2^20 * 100
	double rh_pct = ((double)raw_humidity / 1048576.0) * 100.0;
	r->rh_pct = (float)(round(rh_pct * 10.0) / 10.0);

	r->t = now;

	return 0;
}
