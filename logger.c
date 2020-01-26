/*
Author: Braeden Mulligan
		braeden.mulligan@gmail.com

*/

#include <stdbool.h>
#include <util/delay.h>
#include <stdio.h>

#include "error.h"
#include "hardware.h"
#include "logger.h"
#include "serial.h"

/*
short log_tail = 0;
bool log_full = false;

void log_clear() {
	log_tail = 0;
	log_full = false;
	log_buffer[0] = '\0';
	log_buffer[LOG_BUFFER_SIZE - 1] = '\0';
}

// Simply overwrite buffer for now.
//TODO:
void log_append(char* info) {
	for (short i = 0; info[i] != '\0'; ++i) {
		if (log_tail >= LOG_BUFFER_SIZE - 3) {
			log_tail = 0;
			log_full = true;
		};
		log_buffer[log_tail] = info[i];
		++log_tail;
	}
	log_buffer[log_tail] = '\r';
	log_buffer[log_tail + 1] = '\n';
	log_buffer[log_tail + 2] = '\0';
	log_tail += 3;
}

// Flush log buffer.
void log_print() {
	if (log_full) log_tail = LOG_BUFFER_SIZE - 1;
	for (short i = 0; i < log_tail; ++i) {
		//if (!(i % TX_BUFFER_SIZE)) serial_write(log_buffer + i);
		if (i > 0 && !(i % TX_BUFFER_SIZE - 1)) _delay_ms(50);
		serial_putc(log_buffer[i]);
		serial_print();
	}
	log_clear();
}
*/

void build_report(char command, short* error_code, bool* limp, short* moisture_values, short* timer, bool* valve) {
	char msg[32];
	short failures;
	switch (command) {
		case '\0':
			return;
		case 'c':
			//serial_write("AT+GMR\r\n");
			break;
		case 'e':
			sprintf(msg, "Error status: %d", (*error_code));
			serial_write(msg);
			failures = sensor_status();
			if (failures == SENSOR_BAD_READS) serial_write(", Single sensor error.");
			if (failures == MULTIPLE_SENSOR_BAD_READS) serial_write(", Multiple sensor error.");
			serial_write("\r\n");
			break;
		case 'm':
			if (*limp) {
		 		serial_write("Limp mode active.\r\n");
			}else {
				serial_write("Normal mode active.\r\n");
			};
			break;
		case 's':
			for (uint8_t i = 0; i < SENSOR_COUNT; ++i) {
				sprintf(msg, "Sensor %d: %d\r\n", i, moisture_values[i]);
				serial_write(msg);
			}
			sprintf(msg, "Average: %d\r\n", moisture_values[SENSOR_COUNT]);
			serial_write(msg);
			break;
		case 't':
			if ((*timer) < 0) {
				serial_write("Timer stopped.\r\n");
			}else {
				sprintf(msg, "Timer running %ds\r\n", (*timer));
				serial_write(msg);
			};
			break;
		case 'v':
			if (*valve) {
		 		serial_write("Valve open.\r\n");
			}else {
				serial_write("Valve closed.\r\n");
			};
			break;
		default:
			return;
	}
}

char command_poll() {
	while (rx_available()) {
		char c = serial_getc(false);
		if (c >= 'a' || c <= 'Z') {
			return c;
		};
	}
	return '\0';
}

void logger_init() {
	UART_init(true, true);
}

