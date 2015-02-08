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
#define F_OSC F_CPU
#define ULTRASONIC_SENSOR 4
#define waitForTX() while (!(UCSR0A & 1<<UDRE0))

volatile int32_t ends_of_pulses[4];
volatile int32_t lenghts_of_pulses[4];
volatile int32_t starts_of_pulses[4]; 
volatile int32_t vision_result[3];
uint8_t changed_bits;
uint8_t pinstate, ct;
uint8_t portb_history = 0xFF;

void setup(void) {
	//USART INITIALIZATION
	UCSR0A = 0;
	UCSR0B = 0b10011000;
	UCSR0C = 0b00000110;
	UBRR0H = 0;
	UBRR0L = 25;
	
	//LED REGISTER SETUP
	DDRD |= 1 << 3;	
	
	//INTERRUPTS INIT
	DDRB = 0;
	PORTB |= ((1 << BALL_SENSOR_1) | (1 << BALL_SENSOR_2) | (1 << BALL_SENSOR_3) | (1 << ULTRASONIC_SENSOR)); 
	PCICR |= 1;
	PCMSK0 |= 0xF;

	//TIMER SETUP
	TCCR1A = 0;
	TCCR1B = 2;
	TCNT1 = 0;

	//TURN ON INTERRUPTS
	sei();
}

void sendc(uint8_t ch) {
  waitForTX();
  UDR0 = ch;
}

ISR(USART_RX_vect) {	
}

ISR(PCINT0_vect) {
	PORTD ^= 1 << 3;
	pinstate = PINB;
	changed_bits = pinstate ^ portb_history;
	portb_history = pinstate;  
	for (ct = 0; ct < 4; ct++) {
		if (changed_bits & (1 << ct)) { 
       		if (pinstate & (1 << ct)) { 
				starts_of_pulses[ct] = TCNT1; 
			} else {
				ends_of_pulses[ct] = TCNT1;
				lenghts_of_pulses[ct] = abs(ends_of_pulses[ct] - starts_of_pulses[ct]);
			}
		}
    }
	if(changed_bits & (1 << ULTRASONIC_SENSOR)) {
	}
}

int main(void) {
	uint8_t j, index = 0;
 	uint32_t small;
	setup();
	while (1) {
		small = 9999999;
		vision_result[0] = lenghts_of_pulses[0];
		vision_result[1] = lenghts_of_pulses[1];
		vision_result[2] = lenghts_of_pulses[2];
		for(j = 0; j < 3; j++) {
			if(small > vision_result[j]) {
				small = vision_result[j];
				index = j;
			}
		}
		sendc('0' + index);
		sendc('\r');
        sendc('\n');
		_delay_ms(200);
	}
	return 0;
}
