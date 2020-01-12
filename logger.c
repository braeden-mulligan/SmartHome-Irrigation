/*
Author: Braeden Mulligan
		braeden.mulligan@gmail.com

*/

#include <stdbool.h>

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
void log_append(char* info){
	for (short i = 0; info[i] != '\0'; ++i) {
		if (log_tail >= LOG_BUFFER_SIZE - 2) {
			log_tail = 0;
			log_full = true;
		};
		log_buffer[log_tail] = info[i];
		++log_tail;
	}
	log_buffer[log_tail] = '\0';
	++log_tail;
}

// Flush log buffer.
void log_print() {
	if (log_full) log_tail = LOG_BUFFER_SIZE - 1;
	for (short i = 0; i < log_tail; ++i) {
		//if (!(i % TX_BUFFER_SIZE)) serial_write(log_buffer + i);
		serial_putc(log_buffer[i]);
		serial_print();
	}
	log_clear();
}

void command_poll() {
	while (rx_available()) {
		char c = serial_getc(false);
		if (c >= 'a' || c <= 'Z') {
			log_print();
			for (short i = 0; i < 3; ++i) {
				LED_blink(5);
			}
		};
	}
}

void logger_init() {
	UART_init(true, true);
}

