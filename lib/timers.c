#include <stdbool.h>
#include <avr/interrupt.h>

void TIMER16_init(short period, short period_scale) {
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

void TIMER16_halt(){
}

void TIMER16_start(){
}


short timer8_period_scale = 0;
short timer8_period_count = 0;

void TIMER8_init(short period_scale) {
	timer8_period_scale = period_scale;
	timer8_flag = false;
	//TCCR0A |= (1 << WGM01);
	TIMSK0 |= (1 << OCIE0A); 
	//OCR0A = 13;
	sei(); 
	TCCR0B |= (1 << CS02) | (1 << CS00);
}

void TIMER8_halt() {
	TIMSK0 &= ~(1 << OCIE0A);
}

void TIMER8_restart() {
	timer8_flag = false;
}

ISR(TIMER0_COMPA_vect) {
	if (timer8_period_count < timer8_period_scale)
		++timer8_period_count;
	}else {
		timer8_flag = true;
		timer8_period_count = 0;
	};
}
