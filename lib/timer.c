#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "timer.h"

#define timer8_MAX_MS 10

/*
void timer16_init(short period, short period_scale) {
	timer16_period_scale = period_scale;
	timer16_flag = false;
	TCCR1B = (1 << WGM12); 
	//OCR1A = 785; 
	OCR1A = period; 
	TIMSK1 = (1 << OCIE1A); 
	sei(); 
	TCCR1B |= (1 << CS10) | (1 << CS12);
}

ISR(TIMER1_COMPA_vect) {
	if (timer16_period_count < timer16_period_scale)
		++timer16_period_count;
	}else {
		timer16_flag = true;
		timer16_period_count = 0;
	};
}

void timer16_halt(){
}

void timer16_start(){
}
*/

short timer8_cycle_count = 0;
short timer8_period_scale = 0;

// This function seems to be of limited capability.
void (* volatile timer_routine)(void) = NULL;

// Above 10 ms we only get timer increments in multiples of 10. eg. 76ms becomes 70ms.
void timer8_init(short period_ms, void (* volatile custom_routine)(void)) {
	// This gives us approximate number of clock ticks for 1ms with current prescaler. 
	uint8_t clk_count;
	if (period_ms > timer8_MAX_MS) {
		timer8_period_scale = period_ms / timer8_MAX_MS - 1;
		// This gives us approximate number of clock ticks for 1ms with current prescaler. 
		clk_count = timer8_MAX_MS * 15;
	}else {
		clk_count = (uint8_t)period_ms;
	};

	timer8_flag = false;
	cli();
	timer_routine = custom_routine;

	OCR0A = clk_count;
	TCCR0A |= (1 << WGM01);
	TIMSK0 |= (1 << OCIE0A); 
	sei(); 
}

void timer8_start() {
	TCCR0B |= (1 << CS02) | (1 << CS00);
}

void timer8_stop() {
	TCCR0B &= ~((1 << CS02) | (1 << CS00));
	TCNT0 = 0;
}

ISR(TIMER0_COMPA_vect) {
	if (timer8_cycle_count < timer8_period_scale) {
		++timer8_cycle_count;
	}else {
		timer8_flag = true;
		if (timer_routine != NULL) timer_routine();
		timer8_cycle_count = 0;
	};
}
