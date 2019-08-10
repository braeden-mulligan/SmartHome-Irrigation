/*
Author: Braeden Mulligan
		braeden.mulligan@gmail.com
*/

#include <stdbool.h>
#include <avr/io.h>

#include "serial.h"

void UART_init() {
	UBRR0H = (BAUD_EQ >> 8);
	UBRR0L = BAUD_EQ;
	UCSR0B = (1 << TXEN0);
	UCSR0C = (1 << UCSZ00) | (1 << UCSZ01);
}

char tx_buffer[UART_BUFFER_SIZE];
uint8_t tx_cursor_send = 0;
uint8_t tx_cursor_put = 0;

// Return true if buffer is about to be overwritten on next call.
bool tx_buffer_append(char c) {
	if (tx_cursor_put >= UART_BUFFER_SIZE) tx_cursor_put = 0;
	//tx_cursor_put %= UART_BUFFER_SIZE;
	tx_buffer[tx_cursor_put] = c;
	++tx_cursor_put;
	if (tx_cursor_put == tx_cursor_send) return true;
	return false;
}

void tx_buffer_erase() {
	tx_cursor_send = 0;
	tx_cursor_put = 0;
	for (uint8_t j = 0; j < UART_BUFFER_SIZE; ++j) {
		tx_buffer[0] = '\0';
	}
}


void UART_transmit(char byte) {
	while (!(UCSR0A & (1 << UDRE0))) {}
	UDR0 = byte;
}

void UART_write() {
	do { 
		if (tx_cursor_send >= UART_BUFFER_SIZE) tx_cursor_send = 0;
		UART_transmit(tx_buffer[tx_cursor_send]);
		++tx_cursor_send;
		//tx_cursor_send %= UART_BUFFER_SIZE;
	}while (tx_cursor_send != tx_cursor_put);
}

// Call this function as wrapper for loading serial buffer.
// Checks long strings for buffer overflow. 
void serial_put(char* text) {
	uint8_t i = 0;
	bool warning = false;
	while (text[i] != '\0') {
		if (warning) { 
			UART_write();
			//UART_transmit('\n'); UART_transmit('\r');
			char error[UART_BUFFER_SIZE] = "* ERROR: Buffer overflow. *\n\r";
			for (uint8_t j = 0; j < UART_BUFFER_SIZE; ++j) {
				tx_buffer_append(error[j]);
			}
			UART_write();
			return;
		};
		warning = tx_buffer_append(text[i]);
		++i;
	}
}



