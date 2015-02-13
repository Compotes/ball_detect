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
volatile int32_t vision_result[3] = {0,0,0};
volatile uint8_t pinstate, ct, changed_bits, portb_history = 0xFF, my_address;

void setup(void) {
	//USART INITIALIZATION
	UCSR0A = 0;
	UCSR0B = 0b10011000;
	UCSR0C = 0b00000110;
	UBRR0H = 0;
	UBRR0L = 25;
	
	//LED REGISTER SETUP
	DDRD |= 1 << 3;	
	PORTD |= 1 << 3;
	//DIP-SWITCH INITIALIZATION
    DDRC = 0;
	PORTC |= \
		((1 << DIP_SWITCH_1) | (1 << DIP_SWITCH_2) | (1 << DIP_SWITCH_3) | \
		 (1 << DIP_SWITCH_4) | (1 << DIP_SWITCH_5) | (1 << DIP_SWITCH_6));
	
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
}

void sendc(uint8_t ch) {
  waitForTX();
  UDR0 = ch;
}

ISR(USART_RX_vect) {
	PORTD ^= 1 << 3;
	uint32_t modulo, position;
	uint8_t read_char = UDR0, i, j;
	if(read_char == my_address) {
		for(j = 2; j <= 0; j--) {	
			modulo = vision_result[j];
			position = 1000000;
			for(i = 0;i < 7;i++) {
				sendc('0' + ((modulo / position) % 10));
				modulo %= position;
				position /= 10;
			}
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
				lenghts_of_pulses[ct] = abs(ends_of_pulses[ct] - starts_of_pulses[ct]);
			}
		}
    }
	if(changed_bits & (1 << ULTRASONIC_SENSOR)) {
	}
}

int main(void) {
	setup();
	while (1) {
		cli();
		vision_result[0] = lenghts_of_pulses[0];
		vision_result[1] = lenghts_of_pulses[1];
		vision_result[2] = lenghts_of_pulses[2];
		sei();
	}
	return 0;
}
