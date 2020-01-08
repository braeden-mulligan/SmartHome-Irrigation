#include <avr/interrupt.h>

/*
void TIMER16_init() {
	TCCR1B = (1 << WGM12); 
	OCR1A = 785; 
	TIMSK1 = (1 << OCIE1A); 
	sei(); 
	TCCR1B |= (1 << CS10) | (1 << CS12);
}

ISR(TIMER1_COMPA_vect) {
}

void TIMER16_halt(){
}
void TIMER16_start(short period){
}
*/

void TIMER8_init() {
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
