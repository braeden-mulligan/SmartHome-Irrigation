/*
Author: Braeden Mulligan
        braeden.mulligan@gmail.com
*/

#include <stdbool.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include <util/delay.h>

#include "serial.h"


void UART_init(bool tx, bool rx) {
	cli();
	UBRR0H = (BAUD_PRESCALE >> 8);
	UBRR0L = BAUD_PRESCALE;

	UCSR0C = (1 << UCSZ00) | (1 << UCSZ01);
	if (tx) UCSR0B = (1 << TXEN0) | (1 << TXCIE0);
	if (rx) UCSR0B |= (1 << RXEN0) | (1 << RXCIE0);
	sei();
}

char tx_buffer[TX_BUFFER_SIZE];
char rx_buffer[RX_BUFFER_SIZE];

volatile uint8_t tx_head = 0;
volatile uint8_t tx_tail = 0;
volatile uint8_t rx_head = 0;
volatile uint8_t rx_tail = 0;


/*
	**
	* UART transmit functions.
	**
*/

volatile bool tx_empty = true;

#if TX_INTERRUPT
ISR(USART_TX_vect) {
	if (tx_head != tx_tail || !tx_empty) {
		UDR0 = tx_buffer[tx_head];
		++tx_head;
		if (tx_head >= TX_BUFFER_SIZE) tx_head = 0;
		if (tx_head == tx_tail) tx_empty = true;
	};
}
#else
void UART_transmit(char byte) {
	while (!(UCSR0A & (1 << UDRE0))) {}
	UDR0 = byte;
}
#endif

void serial_print() {
	if (tx_empty) return;
	#if TX_INTERRUPT
	if (UCSR0A & (1 << UDRE0)) UDR0 = 0;
	//UCSR0B |= (1 << UDRIE0);
	#else
	do { 
		UART_transmit(tx_buffer[tx_head]);
		++tx_head;
		if (tx_head >= TX_BUFFER_SIZE) tx_head = 0;
	}while (tx_head != tx_tail);
	tx_empty = true;
	#endif
}

void tx_buffer_erase() {
	tx_head = 0;
	tx_tail = 0;
	tx_empty = true;
}

// TODO: error enum for argument
// Flush TX buffer and report error.
void UART_error() {
	serial_print();
	#if TX_INTERRUPT
	// Slight delay to give existing buffer time to flush before overwriting.
	_delay_ms(100);
	#endif
	tx_buffer_erase();

	strcpy(tx_buffer, "\n\rERROR: Buffer overflow.\n\r");
	for (uint8_t j = 0; tx_buffer[j] != '\0'; ++j) {
		++tx_tail;
	}
	tx_empty = false;
	serial_print();
	#if TX_INTERRUPT
	_delay_ms(100);
	#endif
}

// Return true if buffer is about to be overwritten on next call.
bool tx_buffer_putc(char c) {
	tx_buffer[tx_tail] = c;
	++tx_tail;
	tx_empty = false;
	if (tx_tail >= TX_BUFFER_SIZE) tx_tail = 0;
	if (tx_tail == tx_head) return true;
	return false;
}

// <tx_buffer_putc()> will silently overwrite buffer. Merge into this wrapper
//   if we always way overflows to be reported. 
void serial_putc(char byte) {
	if (tx_tail == tx_head && !tx_empty) {
		UART_error();
		return;
	}else{
		tx_buffer_putc(byte);
	}
}

void serial_puts(char* text) {
	bool warning = false;
	for (uint8_t i = 0; text[i] != '\0'; ++i) {
		if (warning) { 
			UART_error();
			return;
		};
		warning = tx_buffer_putc(text[i]);
	}
}

void serial_write(char* text) {
	serial_puts(text);
	serial_print();
}

/* 
	**
	* UART Receive functions.
	**
*/

// No feedback given if receive buffer is full. 
volatile bool rx_full = false;

ISR(USART_RX_vect) {
	if (!rx_full) {
		rx_buffer[rx_tail] = UDR0;
		++rx_tail;
		if (rx_tail >= RX_BUFFER_SIZE) rx_tail = 0;
		if (rx_tail == rx_head) rx_full = true;
	} else {
		char trash = UDR0;
	};
}

uint8_t rx_available() {
	if (rx_full) {
		return RX_BUFFER_SIZE;
	}else {
		if (rx_tail < rx_head) return rx_tail + (RX_BUFFER_SIZE - rx_head);
		return (rx_tail - rx_head);
	};
}

void rx_buffer_erase() {
	rx_head = 0;
	rx_tail = 0;
	rx_full = false;
}

char serial_getc(bool peek) {
	char byte = '\0';
	if ((rx_head != rx_tail) || rx_full) {
		byte = rx_buffer[rx_head];	
		if (!peek) {
			++rx_head;
			rx_full = false;
		};
	};
	return byte;
}

void serial_gets(char* read_string, short n) {
	if (n < 1 || n > RX_BUFFER_SIZE) n = RX_BUFFER_SIZE;
	for (short i = 0; i < n; ++i) {
		read_string[i] = serial_getc(false);
		if (read_string[i] == '\0') return;
	}
}

