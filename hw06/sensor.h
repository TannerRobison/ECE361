#ifndef SENSOR_H
#define SENSOR_H

#include <stdint.h>
#include <time.h>

typedef struct {
	time_t t;
	float temp_c;
	float rh_pct;
} reading_t;

// reads the temperature and humidity registers, keeps bits 19..0 of each,
//  converts to Celsius and %RH rounded to 0.1, and stores them in *r with
//  the timestamp now. Returns 0 on success, or the iom361 return code of the
//  failed read (-1 if base or r is NULL). *r is unchanged on failure
int sensor_read(uint32_t *base, time_t now, reading_t *r);

#endif
