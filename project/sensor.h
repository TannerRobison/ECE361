#ifndef SENSOR_H
#define SENSOR_H

#include <stdint.h>
#include <time.h>

typedef struct {
	time_t time;
	float temp;
	float humidity;
} reading_t;

// reads the temperature and humidity registers, converts to Celsius and %RH,
//  rounded to 0.1, and puts them in *reading along with the timestamp now.
//  Returns 0 on success, -1 on failure. *reading is unchanged on failure
int sensor_read(uint32_t *base, time_t now, reading_t *reading);

#endif
