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
#define MOISTURE_THRESHOLD 55
#define MOISTURE_DELTA 3 // Account for jitter between readings.
#define SELF_ADJUST 5

short m_target = MOISTURE_TARGET;
short m_damp = MOISTURE_TARGET - MOISTURE_THRESHOLD;
short m_dry = MOISTURE_TARGET + MOISTURE_THRESHOLD;

bool valve_status = false;

short m_status;
short m_level;
short m_initial;
bool m_change = false;

bool limp_mode = false;
void enter_limp_mode() {
	valve_status = valve_off();
	limp_mode = true;
}

bool error_check(short m_status) {
	if (limp_mode) {
		if (sensor_status() < WARNING) return true;
		limp_mode = false;
		return false;
	};

	switch (m_status) {
		case ADC_NOT_READY:
			return false;
		case SENSOR_NO_DATA:
			return false;
		case SENSOR_BAD_READS:
			break;
		case MULTIPLE_SENSOR_BAD_READS:
			break;
		default:
			break;
	}
	enter_limp_mode();
	return true;
}

void check_moisture_change(void) {
	short diff = m_initial - m_level; 
	if ((abs(diff)) > MOISTURE_DELTA) m_change = true;
}

//TODO: Debugging purposes.
char print_ready = '\0';
char msg[32];

void build_report(char command) {
	short failures;
	switch (command) {
		case '\0':
			return;
		case 'e':
			sprintf(msg, "Error status: %d", m_status);
			serial_write(msg);
			failures = sensor_status();
			if (failures == SENSOR_BAD_READS) serial_write(", Single sensor error");
			if (failures == MULTIPLE_SENSOR_BAD_READS) serial_write(", Multiple sensor error");
			serial_write("\r\n");
			break;
		case 's':
			sprintf(msg, "Sensor 0 moisture: %d\r\n", sensor_array[0]);
			serial_write(msg);
			sprintf(msg, "Sensor 1 moisture: %d\r\n", sensor_array[1]);
			serial_write(msg);
			_delay_ms(10);
			sprintf(msg, "Average moisture: %d\r\n", m_level);
			serial_write(msg);
			break;
		case 'm':
			if (limp_mode) {
		 		serial_write("Running in limp mode");
			}else {
				serial_write("Running in normal mode");
			};
			break;
		default:
			return;
	}
};

void control_loop() {
	for (eternity) {
		build_report(command_poll());

		//TODO: maybe this can go in ISR?
		if (timer16_flag) {
			timer16_stop();
			if (!m_change) enter_limp_mode();
			timer16_flag = false;
		};

		m_status = moisture_check();
		// WARNING indicates potentially imminent failure.
		if (m_status > WARNING) m_level = m_status;

		if (m_status < WARNING || limp_mode) {
			if (error_check(m_status)) continue;
		}else {
			if (m_level > m_dry && !valve_status) {
				valve_status = valve_on();
				m_initial = m_level;
				m_change = false;
				timer16_start();
			};
			if (valve_status && m_level < m_damp){
				valve_status = valve_off();
				timer16_stop();
				timer16_flag = false;
				//TODO: observe for hysteresis in moisture value; overshoot?
			};	
		};
		//if (!valve_status) low_power_sleep();
	}
}

	
	
int main(void) {
	// Set internal LED off.
	DDRB |= _BV(DDB5);
	PORTB &= ~_BV(DDB5);

	logger_init();
	hardware_init(3);
	timer16_init(30, &check_moisture_change);

	control_loop();
	return 0;
}
