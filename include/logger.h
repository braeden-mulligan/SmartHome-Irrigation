/*
Author: Braeden Mulligan
		braeden.mulligan@gmail.com
*/

#include "serial.h"

// Consider ATmega328p only has 2KiB of memory.
#define LOG_BUFFER_SIZE 256

char log_buffer[LOG_BUFFER_SIZE]; 

void logger_init();

void log_clear();

void log_append(char* info);

void log_print();

char command_poll();
