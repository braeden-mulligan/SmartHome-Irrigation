/*
Author: Braeden Mulligan
		braeden.mulligan@gmail.com
*/

#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
//#include <util/delay.h>

#include "lib/serial.h"
#include "lib/logger.h"

//--- Analogue to Digital.
//TODO: Tweak for 3.3V
void ADC_convert() {
	ADCSRA |= (1 << ADSC);
}

void ADC_init() {
	ADMUX = (1 << REFS0) | (1 << MUX0) | (1 << MUX2);
	ADCSRA = (1 << ADEN) | (1 << ADIE); // Clock scaling for ADC
	ADCSRA |= (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2); // Trigger interrupt upon conversion.
	DIDR0 = (1 << ADC5D);

	ADC_convert();
}

ISR(ADC_vect) {
	//delta = delta_min + (0.33 * ( (float) ADC / 1024.0));
}
//---

//--- General 16 bit Timer.
//TODO: Repurpose for timed moisture checks.
void TIMER16_init() {
	TCCR1B = (1 << WGM12); 
	OCR1A = 785; 
	TIMSK1 = (1 << OCIE1A); 
	sei(); 
	TCCR1B |= (1 << CS10) | (1 << CS12);
}

//ISR(TIMER1_COMPA_vect) {
//}


int main(void) {

	UART_init();
	TIMER8_init();

	char status_message[UART_BUFFER_SIZE] = "Test status report.\n\r";
	while (true) {
		button_poll(status_message);
	//	Read/control moisture
	//	sleep
	}

}

