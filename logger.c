/*
Author: Braeden Mulligan
		braeden.mulligan@gmail.com

*/

#include <stdbool.h>
#include <util/delay.h>

#include "hardware.h"
#include "logger.h"
#include "serial.h"

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

