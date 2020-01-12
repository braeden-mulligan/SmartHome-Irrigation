/*
Author: Braeden Mulligan
		braeden.mulligan@gmail.com
*/

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>

#include "hardware.h"
#include "logger.h"
#include "timer.h"

#define eternity ;;
#define abs(x) x > 0 ? x : -x

#define MOISTURE_TARGET 400
#define MOISTURE_THRESHOLD 55
#define MOISTURE_DELTA 3 // Account for jitter between readings.
#define SELF_ADJUST 5

short m_target = MOISTURE_TARGET;
short m_damp = MOISTURE_TARGET - MOISTURE_THRESHOLD;
short m_dry = MOISTURE_TARGET + MOISTURE_THRESHOLD;

bool valve_status = false;

void control_loop() {
	short m_level;

	for (eternity) {
		log_clear();
		m_level = moisture_check();

		if (m_level < 0) {
		//if (m_level is error value) {
			//TODO: do something sensible here.
			valve_status = valve_off();
			//TODO: log_error()
			_delay_ms(1000);
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

		log_append("Debugging info:\r\n");
		char tmp_buffer[32];
		for (uint8_t i = 0; i < SENSOR_COUNT; ++i) {
			sprintf(tmp_buffer, "Sensor%d status: %d\r\n", i, sensor_array[i]);
			log_append(tmp_buffer);
		};
		command_poll();
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
	reset timer count
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

char buf[32];
volatile bool timer_running = true;
int count = 0;
void test_func(void) {
	sprintf(buf, "Count is %d.\r\n", count);
	serial_write(buf);
	count++;
	if (count > 2) {
		timer16_stop();
		serial_write("Stopping timer.\r\n");
		count = 0;
		timer_running = false;
	};
}
	
int main(void) {
	// Set internal LED off.
	DDRB |= _BV(DDB5);
	PORTB &= ~_BV(DDB5);

	logger_init();
	hardware_init(3);

	timer16_init(1, &test_func);
	timer16_start();
	while (true) {
		if (!timer_running) {
			_delay_ms(2000);
			serial_write("Restarting timer.\r\n");
			timer16_start();
			timer_running = true;
		};
	}

	control_loop();
	return 0;
}
