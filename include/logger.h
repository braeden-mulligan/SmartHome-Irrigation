/*
Author: Braeden Mulligan
		braeden.mulligan@gmail.com
*/

#include "serial.h"

// Guarantee 4 logs can be queued.
char log_buffer[TX_BUFFER_SIZE / 4]; 

void logger_init();

void log_clear();

void log_append(char* info);

void log_error(char* info);

void button_poll();
