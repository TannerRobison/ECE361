#include <stdint.h>
#include <stdio.h>

#include "bits.h"

static int check_arguments(int pos, int width)
{
	if (width < 1 || width > 32) {
		return -1;
	}
	if (pos < 0 || pos > 31) {
		return -1;
	}
	if ((pos + width) > 32) {
		return -1;
	}
	return 0;
}

void print_binary(uint32_t x, int width)
{
	if (check_arguments(0, width)) {
		return;
	}
	for (int32_t i = width - 1; i >= 0; i--) {
		uint8_t bit_is_set = (x & (1u << i)) != 0;
		printf("%d", bit_is_set);
		if (i % 4 == 0 && i > 0) {
			printf(" ");
		}
	}
}

uint32_t get_field(uint32_t word, int pos, int width)
{
	if (check_arguments(pos, width)) {
		return 0;
	}
	uint32_t mask = 0xFFFFFFFFu >> (32 - width);
	return (word >> pos) & mask;
}

uint32_t set_field(uint32_t word, int pos, int width, uint32_t value)
{
	if (check_arguments(pos, width)) {
		return word;
	}
	uint32_t mask = 0xFFFFFFFFu >> (32 - width);
	return (word & ~(mask << pos)) | ((value & mask) << pos);
}

int32_t sign_extend(uint32_t value, int width)
{
	if (check_arguments(0, width) < 0) {
		return 0;
	}

	uint32_t mask = 0xFFFFFFFFu >> (32 - width);
	value &= mask;

	uint8_t is_negative = (value & (1u << (width - 1))) != 0;
	if (is_negative) {
		return value | ~mask;
	} else {
		return value;
	}
}
