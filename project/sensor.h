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
//  and puts them in appropriate reading_t *reading members along with the time.
//  Returns 0 on success, -1 on failure. *reading is unchanged on failure
int read_sensor(uint32_t *base, reading_t *reading);

#endif
