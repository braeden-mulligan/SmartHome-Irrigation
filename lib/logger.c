/*
Author: Braeden Mulligan
		braeden.mulligan@gmail.com

This file contains subroutines to offer basic UI for debugging.
It polls for a button press and will print messages over serial accordingly.
*/

#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "logger.h"

//--- Button input polling.
void TIMER8_init() {
	// Initialize internal LED off.
	DDRB |= _BV(DDB5);
	PORTB &= ~_BV(DDB5);

	// Set pin 12 input.
	DDRB &= ~_BV(DDB4);
	PORTB |= _BV(DDB4);

	//TCCR0A |= (1 << WGM01);
	TIMSK0 |= (1 << OCIE0A); 
	//OCR0A = 13;
	sei(); 
	TCCR0B |= (1 << CS02) | (1 << CS00);
}

void TIMER8_halt() {
	TIMSK0 &= ~(1 << OCIE0A);
}

uint8_t timer8_count = 0;
volatile bool pressed = false;
volatile bool held = false;
ISR(TIMER0_COMPA_vect) {
	if (timer8_count < 3) { // Scale timer to poll every ~50ms.
		++timer8_count;
	}else {
		if (!(PINB & _BV(DDB4))) {
			PORTB |= _BV(DDB5);
			if (!held) {
				pressed = true;
				held = true;
			};
		}else {
			PORTB &= ~_BV(DDB5);
			held = false;
		};
		timer8_count = 0;
	};
}

void log_clear() {
	tx_buffer_erase(); 
}

void log_append(char* info){
	serial_put(info);
}

void blink_LED() {
	PORTB |= _BV(DDB5);
	_delay_ms(500);
	PORTB &= ~_BV(DDB5);
	_delay_ms(500);
}

//TODO: Blink LED via 8 bit timer. Cause system stoppage for now. 
void log_error(char* info){
	TIMER8_halt();
	serial_put(info);
	while (true) {
		UART_write();
		for (int i = 0; i < 5; ++i) {
			blink_LED();
		}
	}
}

// Check if debug button was pressed to flush serial buffer.
void button_poll() {
	if (pressed) {
		pressed = false;
		UART_write();
	};
}

void LOGGER_init() {
	UART_init();
	TIMER8_init();
}

