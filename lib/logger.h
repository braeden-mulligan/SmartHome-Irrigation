/*
Author: Braeden Mulligan
		braeden.mulligan@gmail.com
*/

#include "serial.h"

// Guarantee 4 logs can be queued.
char log_buffer[UART_BUFFER_SIZE / 4]; 

void LOGGER_init();

void log_clear();

void log_append(char* info);

void log_error(char* info);

void button_poll();
