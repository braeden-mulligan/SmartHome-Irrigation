#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "hardware.h"

// Experimentally determined range of sensor readings.
// Consider moisture values outside of these bounds to be garbage.
#define SENSOR_MIN 280
#define SENSOR_MAX 620

uint8_t sensor_id = 0;

volatile short sensor_read = 0;
volatile bool read_done = false;
short read_failures = 0;
short read_failure_limit = 0;

bool valve_off() {
	PORTB &= ~_BV(DDB0);
	return false;
}

bool valve_on() {
	PORTB |= _BV(DDB0);
	return true;
}

void ADC_init() {
	ADMUX = (1 << REFS0);
	ADCSRA |= (1 << ADEN) | (1 << ADIE); 
	ADCSRA |= (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2); 
	DIDR0 = (1 << ADC0D);
	DIDR0 = (1 << ADC0D);
	//ADC_convert(0);
}

void hardware_init(short failure_limit) {
	ADC_init();
	// Set valve control pin 8 output - off.
	DDRB |= _BV(DDB0);
	PORTB &= ~_BV(DDB0);
	read_failure_limit = failure_limit;
	for (uint8_t i = 0; i < SENSOR_COUNT; ++i) {
		sensor_array[i] = 0;
	}
}


ISR(ADC_vect) {
	sensor_read = ADC;
	read_done = true;
}

void ADC_convert(uint8_t sensor_id) {
	ADMUX &= ~(0x07); //Reset MUX select bits.
	ADMUX |= sensor_id;
	ADCSRA |= (1 << ADSC);
}

// Make sure there enough good "consecutive" reads.
//TODO: per-sensor error checking.
bool ADC_verify(uint8_t id) {
	if (sensor_read < SENSOR_MIN || sensor_read > SENSOR_MAX) {
		read_failures++;
		return false;
	}else {
		read_failures--;
		if (read_failures < 0) read_failures = 0;
		return true;
	};
}

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
	short d = 0;
	for (int i = 0; i < SENSOR_COUNT; ++i) {
		if (array[i] > SENSOR_MIN && array[i] < SENSOR_MAX) {
			sum += array[i];
			++d;
		}
	}
	if (d) {
		return (sum / d);
	}else {
		return -1;
	}
}

short moisture_check() {
	if (read_done) {
		ADC_verify(sensor_id);
		sensor_array[sensor_id] = calibrated_read(sensor_id, sensor_read);
		sensor_id = (++sensor_id) % SENSOR_COUNT;
		read_done = false;
		ADC_convert(sensor_id);
	};
	return sensor_average(sensor_array);
	//TODO: return failure values
}

