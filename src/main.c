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

#include "error.h"
#include "hardware.h"
#include "logger.h"
#include "timer.h"

#define eternity ;;
#define abs(x) x > 0 ? x : -x

#define MOISTURE_TARGET 400
#define MOISTURE_THRESHOLD 50
#define MOISTURE_DELTA 5 // Account for jitter between readings.

short m_target = MOISTURE_TARGET;
short m_damp = MOISTURE_TARGET - MOISTURE_THRESHOLD;
short m_dry = MOISTURE_TARGET + MOISTURE_THRESHOLD;

bool valve_state = false;

short m_status;
short m_level;
short m_initial;
bool m_change = true;

short timer_elapsed = -1;

void record_time(void) {
	++timer_elapsed;
}

void timer_set() {
	m_initial = m_level;
	timer_elapsed = 0;
	timer16_start();
}

void timer_halt() {
	timer_elapsed = -1;
	timer16_stop();
}

bool limp_mode = false;

void enter_limp_mode() {
	valve_state = valve_off();
	LED_on();
	limp_mode = true;
}

void exit_limp_mode() {
	limp_mode = false;
	//TODO: Comment out to see if limp mode was ever entered. Only for debugging.
	LED_off();
}

bool error_check() {
	if (limp_mode) {
		if ((sensor_status() < NO_ERROR) || !m_change) return true;
		if (m_status >= NO_ERROR) {
			exit_limp_mode();
			return false;
		};
	};

	switch (m_status) {
		case WARNING:
			return false;
		case SENSOR_NO_DATA:
			return false;
		case UNKNOWN_ERROR:
			break;
		case SENSOR_BAD_READS:
			break;
		case MULTIPLE_SENSOR_BAD_READS:
			break;
		default:
//TODO: maybe dangerous if all possible errors not already handled?
			return false;
	}

	enter_limp_mode();
	return true;
}

void control_loop() {
	short open_count = 0;
	short moistures[SENSOR_COUNT + 1];
	for (eternity) {
		moistures[0] = sensor_array[0];
		moistures[1] = sensor_array[1];
		moistures[2] = m_level;
		build_report(command_poll(), &m_status, &limp_mode, moistures, &timer_elapsed, &valve_state, &open_count);

		if (timer_elapsed >= 120) {
			timer_halt();
			// If moisture doesn't change quick enough, proably not getting water,
			// or keeps watering past saturation.
			short diff = m_initial - m_level; 
			if ((abs(diff)) > MOISTURE_DELTA) {
				m_change = true;
			}else {
				m_change = false;
			};

			if (m_change) {
				timer_set();
			}else {
				enter_limp_mode();
			};
		};

		m_status = moisture_check();
		if (m_status > NO_ERROR) m_level = m_status;
		// WARNING is okay but indicates potentially imminent failure.
		//if (m_status < WARNING || limp_mode) {
		if (error_check()) {
			continue;
		}else {
			if (m_level > m_dry && !valve_state) {
				valve_state = valve_on();
				++open_count;
				timer_set();
			};
			if (valve_state && m_level < m_damp){
				valve_state = valve_off();
				timer_halt();
				//timer16_flag = false;
			};	
		};
		//if (!valve_state) low_power_sleep();
	}
}

int main(void) {
	// Set internal LED off.
	DDRB |= _BV(DDB5);
	PORTB &= ~_BV(DDB5);

	logger_init();
	hardware_init(3);
	timer16_init(1, &record_time);

	control_loop();

	return 0;
}
