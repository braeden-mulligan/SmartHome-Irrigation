/*
Author: Braeden Mulligan
		braeden.mulligan@gmail.com
*/

#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "logger.h"
#include "serial.h"

//--- Button input polling for debugging.
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

uint8_t timer8_count = 0;
volatile bool pressed = false;
volatile bool held = false;
ISR(TIMER0_COMPA_vect) {
	if (timer8_count < 3) { // Scale timer to poll every 50ms.
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

// Read and print to serial the 'log' buffer.
void button_poll(char* log) {
	if (pressed) {
		pressed = false;
		serial_write(log);
	};
}
//---
