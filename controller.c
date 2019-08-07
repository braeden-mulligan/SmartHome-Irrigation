/*
Author: Braeden Mulligan
		braeden.mulligan@gmail.com
*/

#include <stdbool.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
//#include <util/delay.h>

#include "lib/serial.h"
#include "lib/logger.h"

//--- Analogue to Digital.
void ADC_convert() {
	ADCSRA |= (1 << ADSC);
}

void ADC_init() {
	ADMUX = (1 << REFS0);
	ADCSRA |= (1 << ADEN) | (1 << ADIE); // Trigger interrupt upon conversion.
	ADCSRA |= (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2); // Clock scaling for ADC.
	DIDR0 = (1 << ADC0D);

	ADC_convert();
}

volatile short sensor_read = 0;
ISR(ADC_vect) {
	sensor_read = ADC;
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
	ADC_init();

	char status_message[UART_BUFFER_SIZE];
	while (true) {
		sprintf(status_message, "Sensor result: %d\n\r", sensor_read);
		ADC_convert();
		button_poll(status_message); //TODO: Bug in serial writing. Looping behaviour upon (UART_BUFFER_SIZE / 4) number of writes.
	}

}

