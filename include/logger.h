/*
Author: Braeden Mulligan
		braeden.mulligan@gmail.com
*/

#include <stdint.h>

//#include "serial.h"
#include "uart.h"

// Consider ATmega328p only has 2KiB of memory.
#define LOG_BUFFER_SIZE 256

//char log_buffer[LOG_BUFFER_SIZE]; 

void logger_init();

/*
void log_clear();

void log_append(char* info);

void log_print();
*/

void build_report(char command, short* error_code, bool* limp, short* moisture_values, short* timer, bool* valve, short* open_count);

char command_poll();
