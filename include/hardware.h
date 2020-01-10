#include <stdint.h>

#define SENSOR_COUNT 2

short sensor_array[SENSOR_COUNT];

void hardware_init(short failure_limits);

void blink_LED(short period_tenths);

bool valve_off();
bool valve_on();

short moisture_check();
