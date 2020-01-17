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

short m_level;
short m_initial;
bool moisture_change = false;

//TODO: placeholder for better error handling.
bool limp_mode = false;

void check_for_change(void) {
	short diff = m_initial - m_level; 
	if (abs(diff) > MOISTURE_DELTA) moisture_change = true;
}

void control_loop() {
	for (eternity) {
		if (limp_mode) {
			//LED_blink(1000);
			continue;
		}; 

		//log_clear();
		m_level = moisture_check();
		if (m_level > 0) {
			if (m_level > m_dry && !valve_status) {
				valve_status = valve_on();
				m_initial = m_level;
				moisture_change = false;
				timerx_start();
			};
			if (valve_status && m_level < m_damp){
				valve_status = valve_off();
				timerx_stop();
				//TODO: observe for hysteresis in moisture value; overshoot?
			};	
		}else {
			//TODO: do something sensible here.
			//check error case
			valve_status = valve_off();
			limp_mode = true;
		};

		if (timerx_flag) {
			timerx_stop();
			if (!moisture_change) {
				//something fucky going on
				limp_mode = true;
			};
			timerx_flag = false;
		};

/*
		log_append("Debugging info:\r\n");
		char tmp_buffer[32];
		for (uint8_t i = 0; i < SENSOR_COUNT; ++i) {
			sprintf(tmp_buffer, "Sensor%d status: %d\r\n", i, sensor_array[i]);
			log_append(tmp_buffer);
		};
		command_poll();
		if (!valve_status) sleep();
*/
	}
}
	
int main(void) {
	// Set internal LED off.
	DDRB |= _BV(DDB5);
	PORTB &= ~_BV(DDB5);

	logger_init();
	hardware_init(3);

	control_loop();
	return 0;
}
