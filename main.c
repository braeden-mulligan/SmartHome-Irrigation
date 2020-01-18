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

short m_status;
short m_level;
short m_initial;
bool m_change = false;

bool limp_mode = false;
void enter_limp_mode() {
	valve_status = valve_off();
	limp_mode = true;
}

void check_moisture_change(void) {
	short diff = m_initial - m_level; 
	if (abs(diff) > MOISTURE_DELTA) m_change = true;
}

//TODO: Debugging purposes.
char print_ready = '\0';
char msg[32];

void control_loop() {
	for (eternity) {
		if (print_ready == 'e') {
			sprintf(msg, "Error status: %d", m_status);
			log_append(msg);
		}
		if (print_ready == 's') {
			sprintf(msg, "Average moisture: %d", m_level);
			log_append(msg);
		};
		if (print_ready == 'm') {
			if (limp_mode) {
		 		log_append("Running in limp mode");
			}else {
				log_append("Running in normal mode");
			};
		};
		if (print_ready != '\0') log_print();
		print_ready = command_poll();

		m_status = moisture_check();
		if (m_status > 0) m_level = m_status;

		if (m_status < -1 || limp_mode) {
			//TODO: do something sensible here.
			//check error codes
			//exit limp_mode conditions.
			if (limp_mode) continue;
			enter_limp_mode();
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

		if (timer16_flag) {
			timer16_stop();
			if (!m_change) {
				//something fucky going on
				enter_limp_mode();
			}
			timer16_flag = false;
		};
		//if (!valve_status) sleep();
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
