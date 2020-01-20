#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "error.h"
#include "hardware.h"

// Experimentally determined range of sensor readings.
// Consider moisture values outside of these bounds to be garbage.
#define SENSOR_MIN 280
#define SENSOR_MAX 620
#define SENSOR_COUNT 2

// Onboard LED controls.
void LED_on() {
	PORTB |= _BV(DDB5);
}

void LED_off() {
	PORTB &= ~_BV(DDB5);
}

// Compiler expects constant for _delay_ms() argument.
// Specify the blink period in tenths of a second.
void LED_blink(uint16_t period_tenths) {
	LED_on();
	for (uint16_t i = 0; i < period_tenths; ++i) {
		_delay_ms(50);
	}
	LED_off();
	for (uint16_t i = 0; i < period_tenths; ++i) {
		_delay_ms(50);
	}
}

uint8_t sensor_id = 0;
short read_failures[SENSOR_COUNT] = {0};
// This should be < SHORT_MAX / 2.
short read_failure_limit = 0;

bool valve_on() {
	PORTB |= _BV(DDB0);
	return true;
}

bool valve_off() {
	PORTB &= ~_BV(DDB0);
	return false;
}

void ADC_init() {
	ADMUX = (1 << REFS0);
	ADCSRA |= (1 << ADEN);// | (1 << ADIE); 
	ADCSRA |= (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2); 
	DIDR0 = (1 << ADC0D);
	DIDR0 = (1 << ADC1D);
}

short ADC_convert(uint8_t sensor_id) {
	ADMUX &= ~(0x7); //Reset MUX select bits.
	ADMUX |= sensor_id;
	ADCSRA |= (1 << ADSC);
	while (ADCSRA & (1 << ADSC)) { };
	return (ADC);
}

/*
ISR(ADC_vect) {
	sensor_read = ADC;
	read_done = true;
}
*/

void hardware_init(short failure_limit) {
	ADC_init();
	// Set valve control pin 8 output - off.
	PORTB &= ~_BV(DDB0);
	DDRB |= _BV(DDB0);
	read_failure_limit = failure_limit;
	for (uint8_t i = 0; i < SENSOR_COUNT; ++i) {
		sensor_array[i] = 0;
	}

	ADC_convert(sensor_id);
}

// Add offset per sensor as readings vary slightly under similar conditions.
// Offsets are determined experimentally.
short calibrated_read(uint8_t id, short sensor_val){
	switch (id) {
		case 0:
			return sensor_val;// + 20;
		case 1:
			return sensor_val;
		default:
			return sensor_val;
	};
}

// Make sure there enough good "consecutive" reads.
bool sensor_verify(uint8_t id, short adc_val) {
	short max_failures = 2 * read_failure_limit;

	if (read_failure_limit < 0) return true;
	if ((adc_val < SENSOR_MIN) || (adc_val > SENSOR_MAX)) {
		read_failures[id]++;
		if (read_failures[id] > max_failures) read_failures[id] = max_failures;
		return false;
	}else {
		read_failures[id]--;
		if (read_failures[id] < 0) read_failures[id] = 0;
		return true;
	};
}

short sensor_average(short* array) {
	short sum = 0;
	short d = 0;
	for (uint8_t i = 0; i < SENSOR_COUNT; ++i) {
		if ((array[i] > SENSOR_MIN) && (array[i] < SENSOR_MAX)) {
			sum += array[i];
			++d;
		};
	}
	if (d) {
		return (sum / d);
	}else {
		return SENSOR_NO_DATA;
	};
}

short sensor_status() {
	uint8_t count = 0;
	for (uint8_t i = 0; i < SENSOR_COUNT; ++i) {
		if (read_failures[i] > read_failure_limit) ++count;
	}
	if (count == 1) return SENSOR_BAD_READS;
	if (count > 1) return MULTIPLE_SENSOR_BAD_READS;
	return NO_ERROR;
}

short moisture_check() {
	short sensor_val = ADC_convert(sensor_id);
	short sensor_condition = 0;

	if (!sensor_verify(sensor_id, sensor_val)) {
		// If one sensor is failing, check all of them.
		sensor_condition = sensor_status();
		if (!sensor_condition) sensor_condition = WARNING;
	};
	sensor_array[sensor_id] = calibrated_read(sensor_id, sensor_val);
	sensor_id = (++sensor_id) % SENSOR_COUNT;

	if (sensor_condition) return sensor_condition;
	return sensor_average(sensor_array);
}

