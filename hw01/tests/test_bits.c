#include <stdint.h>
#include <stdio.h>

#include "bits.h"
#include "status.h"

static int total = 0;
static int fails = 0;

static void test(char *name, uint32_t expected, uint32_t experimental)
{
	total++;
	if (expected == experimental) {
		printf("PASS  %s\n", name);
	} else {
		fails++;
		printf("FAIL  %s: expected %08X, got %08X\n", name, expected,
		       experimental);
	}
}

int main(void)
{
	uint32_t x = 0xABCDEF12;

	// print binary returns void so these have to be checked visually
	printf("\nrunning print_binary\n");
	printf("  (0xABCDEF12, 32)  ");
	print_binary(x, 32);
	printf("  want 1010 1011 1100 1101 1110 1111 0001 0010\n");
	printf("  (0x1, 1)          ");
	print_binary(0x1, 1);
	printf("  want 1\n");

	// checking get_field
	printf("\nrunning get_field\n");
	test("get_field(0xABCDEF12, 4, 12)", 0xEF1, get_field(x, 4, 12));
	test("get_field(0xABCDEF12, 0, 1)", 0x0, get_field(x, 0, 1));
	test("get_field(0xABCDEF12, 1, 1)", 0x1, get_field(x, 1, 1));
	test("get_field(0xABCDEF12, 31, 1)", 0x1, get_field(x, 31, 1));
	test("get_field(0x7FFFFFFF, 31, 1)", 0x0,
	     get_field(0x7FFFFFFFu, 31, 1));
	test("get_field(0xABCDEF12, 0, 32)", 0xABCDEF12, get_field(x, 0, 32));

	// checking set_field
	printf("\nrunning set_field\n");
	test("set_field(0xFFFFFFFF, 8, 4, 0x300)", 0xFFFFF0FF,
	     set_field(0xFFFFFFFFu, 8, 4, 0x300));
	test("set_field(0x0, 4, 4, 0xFF)", 0x000000F0,
	     set_field(0x0u, 4, 4, 0xFF));
	test("set_field(0x0, 31, 1, 1)", 0x80000000,
	     set_field(0x0u, 31, 1, 0x1));
	test("set_field(0xFFFFFFFF, 31, 1, 0)", 0x7FFFFFFF,
	     set_field(0xFFFFFFFFu, 31, 1, 0x0));
	test("set_field(0x0, 0, 32, 0xABCDEF12)", 0xABCDEF12,
	     set_field(0x0u, 0, 32, 0xABCDEF12));

	// checking sign_extend
	printf("\nrunning sign_extend\n");
	test("sign_extend(0xF8, 8)", -8, sign_extend(0xF8, 8));
	test("sign_extend(0x80, 8)", -128, sign_extend(0x80, 8));
	test("sign_extend(0x1, 1)", -1, sign_extend(0x1, 1));
	test("sign_extend(0x0, 1)", 0, sign_extend(0x0, 1));
	test("sign_extend(0x80000000, 32)", INT32_MIN,
	     sign_extend(0x80000000u, 32));
	test("sign_extend(0x7FFFFFFF, 32)", INT32_MAX,
	     sign_extend(0x7FFFFFFFu, 32));

	// out of bounds tests, functions should return 0
	printf("\nrunning out of range\n");
	test("get_field(0xABCDEF12, 0, 0)", 0x0, get_field(x, 0, 0));
	test("get_field(0xABCDEF12, 0, 33)", 0x0, get_field(x, 0, 33));
	test("get_field(0xABCDEF12, -1, 8)", 0x0, get_field(x, -1, 8));
	test("get_field(0xABCDEF12, 32, 1)", 0x0, get_field(x, 32, 1));
	test("get_field(0xABCDEF12, 28, 8)", 0x0, get_field(x, 28, 8));
	test("set_field(0xABCDEF12, 0, 0, 0xFF)", 0xABCDEF12,
	     set_field(x, 0, 0, 0xFF));
	test("set_field(0xABCDEF12, 0, 33, 0xFF)", 0xABCDEF12,
	     set_field(x, 0, 33, 0xFF));
	test("set_field(0xABCDEF12, -1, 8, 0xFF)", 0xABCDEF12,
	     set_field(x, -1, 8, 0xFF));
	test("set_field(0xABCDEF12, 32, 1, 0x1)", 0xABCDEF12,
	     set_field(x, 32, 1, 0x1));
	test("set_field(0xABCDEF12, 28, 8, 0xFF)", 0xABCDEF12,
	     set_field(x, 28, 8, 0xFF));
	test("sign_extend(0xFF, 0)", 0x0, sign_extend(0xFF, 0));
	test("sign_extend(0xFF, 33)", 0x0, sign_extend(0xFF, 33));
	test("sign_extend(0xFF, -1)", 0x0, sign_extend(0xFF, -1));

	// checking status_unpack, the example word from the assignment
	printf("\nrunning status_unpack\n");
	status_t s = status_unpack(0x1631);
	test("0x1631 heat", 1, s.heat);
	test("0x1631 cool", 0, s.cool);
	test("0x1631 fan", 0, s.fan);
	test("0x1631 fault", 0, s.fault);
	test("0x1631 mode", MODE_AUTO, s.mode);
	test("0x1631 reserved", 0, s.reserved);
	test("0x1631 setpoint", 22, s.setpoint);

	// negative set point and an invalid mode of 5
	s = status_unpack(0xF85E);
	test("0xF85E heat", 0, s.heat);
	test("0xF85E cool", 1, s.cool);
	test("0xF85E fan", 1, s.fan);
	test("0xF85E fault", 1, s.fault);
	test("0xF85E mode", MODE_INVALID, s.mode);
	test("0xF85E reserved", 0, s.reserved);
	test("0xF85E setpoint", -8, s.setpoint);

	// most negative set point, reserved bit set, every flag on
	s = status_unpack(0x808F);
	test("0x808F heat", 1, s.heat);
	test("0x808F cool", 1, s.cool);
	test("0x808F fan", 1, s.fan);
	test("0x808F fault", 1, s.fault);
	test("0x808F mode", MODE_OFF, s.mode);
	test("0x808F reserved", 1, s.reserved);
	test("0x808F setpoint", -128, s.setpoint);

	// diagnostics
	printf("\n%d tests, %d failed\n", total, fails);

	return fails != 0;
}
