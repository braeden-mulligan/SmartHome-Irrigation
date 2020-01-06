/*
Author: Braeden Mulligan
		braeden.mulligan@gmail.com
*/

#include <stdbool.h>
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>

#include "hardware.h"
#include "logger.h"

#define eternity ;;
#define ABS(x) x > 0 ? x : -x

//--- Control logic.
#define MOISTURE_TARGET 400
#define MOISTURE_THRESHOLD 55
#define MOISTURE_DELTA 3 // Account for jitter between readings.
#define SELF_ADJUST 5
#define SENSOR_COUNT 2


short m_target = MOISTURE_TARGET;
short m_damp = MOISTURE_TARGET - MOISTURE_THRESHOLD;
short m_dry = MOISTURE_TARGET + MOISTURE_THRESHOLD;

bool valve_status = false;

void control_loop() {
	short m_level;

	for (eternity) {
		log_clear();
		m_level = moisture_check();

		if (m_level == 0) {
		//if (m_level is error value) {
			//TODO: do something sensible here.
			valve_status = valve_off();
			//TODO: log_error()
			_delay_ms(10000);
		}else {
			if (m_level > m_dry && !valve_status) {
				valve_status = valve_on();
				//TODO: start timer()
				};
			if (valve_status && m_level < m_damp){
				valve_status = valve_off();
				//TODO: observe for hysteresis in moisture value; overshoot?
				//stop timer()
			};	
		}

		for (uint8_t i = 0; i < SENSOR_COUNT; ++i) {
			sprintf(log_buffer, "Sensor%d status: %d\n\r",i, sensor_array[i]);
			log_append(log_buffer);
		};

		button_poll();
	}
/*
// start timer() {
if timer trips:
	self adjust,
	stop timer
	valve off
	return
if damp < min_damp
	stop timer
	return;
while abs(initial_read - sensor_read) > delta
	reset timer
		if (timer_trigger) {
			// count++
			if count > x, adjust threshold low
			if (|initial_read - sensor_read| < some_delta) {
				valve_status = valve_off()
				do error stuff;
			};
		};
*/
}

int main(void) {
	// Set internal LED off.
	DDRB |= _BV(DDB5);
	PORTB &= ~_BV(DDB5);

/*
	UART_init();
	TIMER8_init();
*/
	logger_init();
	hardware_init(3);

	control_loop();
	return 0;
}
