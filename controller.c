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
//volatile bool read_done = false;
ISR(ADC_vect) {
	sensor_read = ADC;
	//read_done = true;
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
//---

//--- Control logic.
#define MOISTURE_TARGET 450
#define MOISTURE_THRESHOLD 50

bool valve_status;

bool valve_off() {
	PORTB &= ~_BV(DDB0);
	return false;
}

bool valve_on() {
	PORTB |= _BV(DDB0);
	return true;
}

short initial_read = 0;
short m_target = MOISTURE_TARGET;
short m_low = MOISTURE_TARGET - MOISTURE_THRESHOLD;
short m_high = MOISTURE_TARGET + MOISTURE_THRESHOLD;

// Experimentally determined sensor reading range.
// Consider moisture values outside of these bounds to be garbage.
short m_min = 300;
short m_max = 600;


int main(void) {
	// Set pin 8 output - off.
	DDRB |= _BV(DDB0);
	valve_status = valve_off();

	char status_message[UART_BUFFER_SIZE / 4]; // Append up to 4 logs to serial bufer.

	// Take average of last 3 readings.
	//short sensor_avg[3] = {m_target, m_target, m_target};
	short garbage_pile = 0; // Sensor read failure count.
	short garbage_threshold = 1; // Complain if failure count exceeds this.

	ADC_init();
	UART_init();
	TIMER8_init();

	while (true) {
		// Clear logs that were not printed. 
		tx_buffer_erase(); 

		ADC_convert();

		//if (read_done) {
		if (sensor_read < m_min || sensor_read > m_max) {
			garbage_pile++;
		}else {
			garbage_pile--;
			if (garbage_pile < 0) garbage_pile = 0;
		};

		if (garbage_pile > garbage_threshold) {
			valve_status = valve_off();
			do error stuff;
		}else {
			if (sensor_read < m_low && !valve_status) {
				valve_status = valve_on();
				initial_read = sensor_read;
				start timer
			};

			if (valve_status && sensor_read > m_high){
				valve_status = valve_off();
				stop timer
			};
		};
			
		if (timer triggered) {
			// If soil not getting any water:
			if (|initial_read - sensor_read| < some_delta) {
				valve_status = valve_off()
				do error stuff;
			};
		};

		sprintf(status_message, "Sensor result: %d\n\r", sensor_read);
		serial_put(status_message)
		button_poll();
	}
}

