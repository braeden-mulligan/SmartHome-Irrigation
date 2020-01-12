#include <stdint.h>

#define SENSOR_COUNT 2

short sensor_array[SENSOR_COUNT];

void hardware_init(short failure_limit);

void LED_on();
void LED_off();
void LED_blink(uint16_t period_tenths);

bool valve_on();
bool valve_off();

short moisture_check();
