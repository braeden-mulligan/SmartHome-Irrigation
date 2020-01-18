#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "timer.h"

#define timer8_MAX_MS 10
#define timer16_MAX_S 4

uint16_t timer8_cycle_count = 0;
uint16_t timer8_period_scale = 0;

void (* volatile timer8_subroutine)(void) = NULL;

// Above <period_ms> of 10 ms we only get timer increments every multiple of 10.
//   eg. 76ms truncates to 70ms, 214ms truncates to 210ms, etc.
// This keeps ISR overhead low by default given current timer implementation.
// <custom_subroutine> must be used to handle more precise intervals down to 
//   ~1ms resolution if needed.
void timer8_init(uint16_t period_ms, void (* volatile custom_subroutine)(void)) {
	// This gives us approximate number of clock ticks for 1ms with current prescaler. 
	uint8_t clk_count;
	if (period_ms > timer8_MAX_MS) {
		timer8_period_scale = period_ms / timer8_MAX_MS - 1;
		// This gives us approximate number of clock ticks for 1ms with current prescaler. 
		clk_count = timer8_MAX_MS * 15;
	}else {
		clk_count = (uint8_t)(period_ms * 15);
	};

	timer8_flag = false;
	cli();
	timer8_subroutine = custom_subroutine;

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
		if (timer8_subroutine != NULL) timer8_subroutine();
		timer8_cycle_count = 0;
	};
}


uint16_t timer16_cycle_count = 0;
uint16_t timer16_period_scale = 0;

void (* volatile timer16_subroutine)(void) = NULL;

void timer16_init(uint16_t period_s, void (* volatile custom_subroutine)(void)) {
	// Count clock ticks to 1 second.
	uint16_t clk_count = 15625;
	timer16_period_scale = period_s - 1;

	timer16_flag = false;
	cli();
	timer16_subroutine = custom_subroutine;

	TCCR1B |= (1 << WGM12); 
	OCR1A = clk_count;
	TIMSK1 |= (1 << OCIE1A); 
	sei(); 
}

void timer16_start(){
	TCCR1B |= (1 << CS10) | (1 << CS12);
}

void timer16_stop(){
	TCCR1B &= ~((1 << CS10) | (1 << CS12));
	TCNT1 = 0;
}

ISR(TIMER1_COMPA_vect) {
	if (timer16_cycle_count < timer16_period_scale) {
		++timer16_cycle_count;
	}else {
		timer16_flag = true;
		if (timer16_subroutine != NULL) timer16_subroutine();
		timer16_cycle_count = 0;
	};
}

