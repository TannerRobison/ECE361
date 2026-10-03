#include "bits.h"
#include "status.h"

status_t status_unpack(uint16_t word)
{
	status_t thermostat;
	thermostat.heat = get_field(word, HEAT_POS, HEAT_WIDTH);
	thermostat.cool = get_field(word, COOL_POS, COOL_WIDTH);
	thermostat.fan = get_field(word, FAN_POS, FAN_WIDTH);
	thermostat.fault = get_field(word, FAULT_POS, FAULT_WIDTH);
	thermostat.mode = get_field(word, MODE_POS, MODE_WIDTH);
	if (thermostat.mode > MODE_FAN_ONLY) {
		thermostat.mode = MODE_INVALID;
	}
	thermostat.reserved = get_field(word, RESERVED_POS, RESERVED_WIDTH);
	thermostat.setpoint =
		sign_extend(get_field(word, SETPOINT_POS, SETPOINT_WIDTH),
			    SETPOINT_WIDTH);
	return thermostat;
}
