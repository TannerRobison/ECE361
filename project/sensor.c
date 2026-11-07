#include <math.h>
#include <stdint.h>
#include <time.h>

#include "iom361_r4.h"
#include "sensor.h"

// should I put them here or in main??
//  probably in main tbh

int read_sensor(uint32_t *base, reading_t *reading)
{
	// input validation
	if (base == NULL || reading == NULL) {
		return -1;
	}

	// read the temperature and humidity registers
	int rc = 0;
	uint32_t raw_temp = iom361_readReg(base, TEMP_REG, &rc);
	if (rc != 0) {
		return -1;
	}
	uint32_t raw_humidity = iom361_readReg(base, HUMID_REG, &rc);
	if (rc != 0) {
		return -1;
	}

	// convert to appropriate values, and store
	// TEMPERATURE: ST / 2^20 * 200 - 50
	// 1048576.0 is 2^20. Using floating-point ensures no integer
	// truncation.
	reading->temp = ((double)raw_temp / 1048576.0) * 200.0 - 50.0;
	reading->temp = roundf(reading->temp * 10.0f) / 10.0f;

	// HUMIDITY: SRH / 2^20 * 100
	reading->humidity = ((double)raw_humidity / 1048576.0) * 100.0;
	reading->humidity = roundf(reading->humidity * 10.0f) / 10.0f;

	// get the time
	time(&reading->time);

	return 0;
}
