/*
Author: Braeden Mulligan
		braeden.mulligan@gmail.com
*/

#include <stdbool.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "lib/serial.h"
#include "lib/logger.h"

//--- Analogue to Digital.
void ADC_convert(uint8_t sensor_id) {
	ADMUX &= ~(0x07); //Reset MUX select bits.
	ADMUX |= sensor_id;
	ADCSRA |= (1 << ADSC);
}

void ADC_init() {
	ADMUX = (1 << REFS0);
	ADCSRA |= (1 << ADEN) | (1 << ADIE); 
	ADCSRA |= (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2); 
	DIDR0 = (1 << ADC0D);
	DIDR0 = (1 << ADC0D);

	ADC_convert(0);
}

volatile short sensor_read = 0;
volatile bool read_done = false;
ISR(ADC_vect) {
	sensor_read = ADC;
	read_done = true;
}
//---

//--- General 16 bit Timer.
//TODO: Repurpose for timed moisture checks.
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
*/
//---

//--- Control logic.
#define MOISTURE_TARGET 400
#define MOISTURE_THRESHOLD 55
#define SENSOR_COUNT 2

bool valve_status;

bool valve_off() {
	PORTB &= ~_BV(DDB0);
	return false;
}

bool valve_on() {
	PORTB |= _BV(DDB0);
	return true;
}

void blink_LED() {
	PORTB |= _BV(DDB5);
	_delay_ms(500);
	PORTB &= ~_BV(DDB5);
	_delay_ms(500);
}

//TODO: Blink LED via 8 bit timer.
//TODO: Cause system stoppage for now. 
void error_report(char* buffer, char* message){
	valve_status = valve_off();
	sprintf(buffer, message);
	serial_put(buffer);
	TIMER8_halt();
	while (true) {
		UART_write();
		for (int i = 0; i < 5; ++i) {
			blink_LED();
		}
	}
}

short initial_read = 0;
short m_target = MOISTURE_TARGET;
short m_low = MOISTURE_TARGET - MOISTURE_THRESHOLD;
short m_high = MOISTURE_TARGET + MOISTURE_THRESHOLD;
// Experimentally determined range of sensor readings.
// Consider moisture values outside of these bounds to be garbage.
short m_min = 300;
short m_max = 600;


int main(void) {
	// Set pin 8 output - off.
	DDRB |= _BV(DDB0);
	valve_status = valve_off();
	// Set internal LED off.
	DDRB |= _BV(DDB5);
	PORTB &= ~_BV(DDB5);

	char status_message[UART_BUFFER_SIZE / 4]; // Append up to 4 logs to serial bufer.

	// Sensor read failure count.
	short garbage_pile = 0; 
	short garbage_limit = 1; // Allow 1 mis-read.

	ADC_init();
	UART_init();
	TIMER8_init();

	short sensor_array[SENSOR_COUNT];
	uint8_t sensor_id = 0;
	uint8_t toggle_count = 0;
	while (true) {
		// Clear logs that were not printed. 
		tx_buffer_erase(); 

		if (read_done) {
			sensor_array[sensor_id] = sensor_read;
			sensor_id = (++sensor_id) % SENSOR_COUNT;

			//TODO: Factor in all sensors
			// Make sure there enough good "consecutive" reads.
			if (sensor_array[0] < m_min || sensor_array[0] > m_max) {
				garbage_pile++;
			}else {
				garbage_pile--;
				if (garbage_pile < 0) garbage_pile = 0;
			};
			//sprintf(status_message, "Read attempts: %d\n\r", read_waits);
			//serial_put(status_message);
			read_done = false;
			
		}else { 
			ADC_convert(sensor_id);
		};

		if (garbage_pile > garbage_limit) {
			valve_status = valve_off();
			error_report(status_message, "Consecutive reads fail\n\r");
		}else {
			if (sensor_array[0] > m_high && !valve_status) {
				valve_status = valve_on();
				initial_read = sensor_read;
				toggle_count++;
				//start timer
			};

			if (valve_status && sensor_array[0] < m_low){
				valve_status = valve_off();
				//TODO: observe for hysteresis in moisture value; overshoot?
				//stop timer
			};
		};
			
		/*
		if (timer_trigger) {
			// If soil not getting any water:
			if (|initial_read - sensor_read| < some_delta) {
				valve_status = valve_off()
				do error stuff;
			};
		};
		*/

		for (uint8_t i = 0; i < SENSOR_COUNT; ++i) {
			sprintf(status_message, "Sensor%d status: %d\n\r",i, sensor_array[i]);
			serial_put(status_message);
		};
		sprintf(status_message, "Valve switch count: %d\n\r", toggle_count);
		serial_put(status_message);
		button_poll();
	}
}

