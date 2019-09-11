/*
Author: Braeden Mulligan
		braeden.mulligan@gmail.com
*/

#include <stdbool.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "lib/logger.h"

#define ABS(x) x > 0 ? x : -x
// Experimentally determined range of sensor readings.
// Consider moisture values outside of these bounds to be garbage.
#define SENSOR_MIN 280
#define SENSOR_MAX 620

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

	//ADC_convert(0);
}

volatile short sensor_read = 0;
volatile bool read_done = false;

ISR(ADC_vect) {
	sensor_read = ADC;
	read_done = true;
}

// Make sure there enough good "consecutive" reads.
void ADC_verify(short* failures) {
	if (sensor_read < SENSOR_MIN || sensor_read > SENSOR_MAX) {
		(*failures)++;
	}else {
		(*failures)--;
		if ((*failures) < 0) (*failures) = 0;
	};
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

void TIMER16_halt(){
}
void TIMER16_start(short period){
}
*/
//---

//--- Control logic.
#define MOISTURE_TARGET 400
#define MOISTURE_THRESHOLD 55
#define MOISTURE_DELTA 3 // Account for jitter between readings.
#define SELF_ADJUST 5
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

short m_target = MOISTURE_TARGET;
short m_damp = MOISTURE_TARGET - MOISTURE_THRESHOLD;
short m_dry = MOISTURE_TARGET + MOISTURE_THRESHOLD;

// Add offset per sensor as readings vary slightly under similar conditions.
// Offsets are determined experimentally.
short calibrated_read(uint8_t id, short sensor_val){
	switch (id) {
		case 0:
			return sensor_val + 20;
		case 1:
			return sensor_val;
		default:
			return sensor_val;
	};
}

short sensor_average(short* array) {
	short sum = 0;
	for (int i = 0; i < SENSOR_COUNT; ++i){
		if (array[i] > SENSOR_MIN && array[i] < SENSOR_MAX) {
			sum += array[i];
		};
	}
	return (sum / SENSOR_COUNT);
}

/*
uint8_t timer_wait_count = 0;
uint8_t timer_wait_max = 10;
*/
void moisture_control() {
	// Sensor read failure count.
	short garbage_pile = 0; 
	short garbage_limit = 3; // Allow 3 mis-reads.

	short sensor_array[SENSOR_COUNT];
	uint8_t sensor_id = 1;

	short m_avg = 0;

	while (true) {
		// Clear logs that were not printed. 
		log_clear();

		if (read_done) {
			// Recalculate average after all sensors have been read.
			if (!sensor_id) m_avg = sensor_average(sensor_array);
			sensor_array[sensor_id] = calibrated_read(sensor_id, sensor_read);
			sensor_id = (++sensor_id) % SENSOR_COUNT;
			ADC_verify(&garbage_pile);
			read_done = false;
		}else { 
			ADC_convert(sensor_id);
		};

		if (garbage_pile > garbage_limit) {
			valve_status = valve_off();
			log_error("Consecutive reads fail\n\r");
		}else {
			if (m_avg > m_dry && !valve_status) {
				valve_status = valve_on();
				//TODO: start timer()
			};

			if (valve_status && m_avg < m_damp){
				valve_status = valve_off();
				//TODO: observe for hysteresis in moisture value; overshoot?
				//stop timer()
			};
		};
			
		for (uint8_t i = 0; i < SENSOR_COUNT; ++i) {
			sprintf(log_buffer, "Sensor%d status: %d\n\r",i, sensor_array[i]);
			log_append(log_buffer);
		};

		button_poll();
	}
/*
// start timer() {
if timer trips:
	self adjust,
	stop timer
	valve off
	return

if damp < min_damp
	stop timer
	return;

while abs(initial_read - sensor_read) > delta
	reset timer
		if (timer_trigger) {
			// count++
			if count > x, adjust threshold low
			if (|initial_read - sensor_read| < some_delta) {
				valve_status = valve_off()
				do error stuff;
			};
		};
*/
}

int main(void) {
	// Set pin 8 output - off.
	DDRB |= _BV(DDB0);
	valve_status = valve_off();
	// Set internal LED off.
	DDRB |= _BV(DDB5);
	PORTB &= ~_BV(DDB5);

/*
	UART_init();
	TIMER8_init();
*/
	LOGGER_init();
	ADC_init();

	moisture_control();
}

