#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include <avr/interrupt.h>
#include <avr/io.h>
#include <inttypes.h>
#include <util/delay.h>

#define BALL_SENSOR_1 0
#define BALL_SENSOR_2 1
#define BALL_SENSOR_3 2
#define DIP_SWITCH_1 0
#define DIP_SWITCH_2 1
#define DIP_SWITCH_3 2 
#define DIP_SWITCH_4 3
#define DIP_SWITCH_5 4
#define DIP_SWITCH_6 5
#define F_OSC F_CPU
#define ULTRASONIC_SENSOR 4
#define waitForTX() while (!(UCSR0A & 1<<UDRE0))

volatile int32_t ends_of_pulses[4];
volatile int32_t lenghts_of_pulses[4];
volatile int32_t starts_of_pulses[4]; 
volatile int32_t vision_result[3];
volatile int32_t sensor_values[3];
volatile int32_t counter[3];
volatile int32_t min[3] = {9999999, 9999999, 9999999};
volatile uint32_t now;
volatile uint8_t pinstate, ct, changed_bits, portb_history = 0xFF, my_address, address_of_message = 1;

void setup(void) {
	//USART INITIALIZATION
	UCSR0A = 0;
	UCSR0B = 0b10011000;
	UCSR0C = 0b00000110;
	UBRR0H = 0;
	UBRR0L = 25;
	
	//LED REGISTER SETUP
	DDRD |= 1 << 3;	

	//DIP-SWITCH INITIALIZATION
	DDRC = 0;
	PORTC |= ((1 << DIP_SWITCH_1)
		| (1 << DIP_SWITCH_2)
		| (1 << DIP_SWITCH_3)
		| (1 << DIP_SWITCH_4)
		| (1 << DIP_SWITCH_5)
		| (1 << DIP_SWITCH_6));	
	
	//INTERRUPTS INIT
	DDRB = 0;
	PORTB |= ((1 << BALL_SENSOR_1)
		| (1 << BALL_SENSOR_2)
		| (1 << BALL_SENSOR_3)
		| (1 << ULTRASONIC_SENSOR)); 
	PCICR |= 1;
	PCMSK0 |= 0xF;

	//TIMER SETUP
	TCCR1A = 0;
	TCCR1B = 2;
	TCNT1 = 0;

	//DETECT ADRESS
	if((PINC & (1 << DIP_SWITCH_1)) == 0) {
		my_address = 1;
	} else if((PINC & (1 << DIP_SWITCH_2)) == 0) {
		my_address = 2;
	} else if((PINC & (1 << DIP_SWITCH_3)) == 0) {
		my_address = 3;
	} else if((PINC & (1 << DIP_SWITCH_4)) == 0) {
		my_address = 4;
	}
	
	//TURN ON INTERRUPTS
	sei();
}

void sendc(uint8_t ch) {
	waitForTX();
	UDR0 = ch;
}

ISR(USART_RX_vect) {	
	PORTD ^= 1 << 3;
	uint32_t small = 9999999, index = 0;
	uint8_t read_char = UDR0, j,  message = 0;
	if(read_char == my_address) {
		if(address_of_message == 1) {
			for(j = 0; j < 3; j++) {	
				if(small > vision_result[j]) { 
					small = vision_result[j];
					now = vision_result[j];
					index = j;
				}
			}
			message |= (index << 6);
			message |= (now & 0b111111);
			now >>= 6;
			address_of_message = 2;
			sendc(message);
		} else {
			address_of_message = 1;
			message = now;
			sendc(message);	
		}
	} 		
}

ISR(PCINT0_vect) {
	pinstate = PINB;
	changed_bits = pinstate ^ portb_history;
	portb_history = pinstate;  
	for (ct = 0; ct < 4; ct++) {
		if (changed_bits & (1 << ct)) { 
			if (pinstate & (1 << ct)) { 
				starts_of_pulses[ct] = TCNT1; 
			} else {
				ends_of_pulses[ct] = TCNT1;
				lenghts_of_pulses[ct] = abs(ends_of_pulses[ct] -
											starts_of_pulses[ct]);
				if (counter[ct] <= 9) {
					if (lenghts_of_pulses[ct] < min[ct]) {
                    	min[ct] = lenghts_of_pulses[ct];
                	}
					sensor_values[ct] += lenghts_of_pulses[ct];
					counter[ct]++;
				} else {
					vision_result[ct] = (sensor_values[ct] - min[ct]) / 9;
					sensor_values[ct] = 0;
					counter[ct] = 0;
					min[ct] = 9999999;
				}
			}
		} 
	}
	if(changed_bits & (1 << ULTRASONIC_SENSOR)) {
	}
}

int main(void) {
	setup();
	while (1) {
		//vision_result[0] = lenghts_of_pulses[0];
		//vision_result[1] = lenghts_of_pulses[1];
		//vision_result[2] = lenghts_of_pulses[2];
	}
	return 0;
}
