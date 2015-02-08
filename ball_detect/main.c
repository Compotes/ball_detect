#include <avr/io.h>
#include <stdlib.h>
#include <stdint.h> 
#include <util/delay.h>
#include <avr/interrupt.h>
#include <inttypes.h>
#include <math.h>
#define F_OSC F_CPU

#define BALL_SENSOR_1 0
#define BALL_SENSOR_2 1
#define BALL_SENSOR_3 2
#define ULTRASONIC_SENSOR 4

volatile int32_t pulse[4];
volatile int32_t pulses[4]; // averages of pulses
volatile int32_t helper[4]; // lenghts of pulses
volatile int32_t result[3]; //results of ball sensor visions :)
uint8_t changed_bits;
uint8_t pinstate;
uint8_t portb_history = 0xFF;

void setup(void) {
	//USART INITIALIZATION
	UCSR0A = 0;
	UCSR0B = 0b10011000;
	UCSR0C = 0b00000110;
	UBRR0H = 0;
	UBRR0L = 25;
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
	
	DDRD |= 1 << 3;
}

#define waitForTX() while (!(UCSR0A & 1<<UDRE0))

void sendc(uint8_t ch) {
  waitForTX();
  UDR0 = ch;
}



ISR(USART_RX_vect) {
	
}

ISR(PCINT0_vect) {
	pinstate = PINB;
	PORTD ^= 1 << 3;
	changed_bits = pinstate ^ portb_history;
	portb_history = pinstate;  
	int ct = 0;
	for (ct = 0;ct < 4;ct++) {
		if (changed_bits & (1 << ct)) { 
       		if (pinstate & (1 << ct)) { 
				helper[ct] = TCNT1; 
			} else {
				pulse[ct] = TCNT1;
				pulses[ct] = abs(pulse[ct]-helper[ct]);
			}
		}
    }
	if(changed_bits & (1 << ULTRASONIC_SENSOR)) {
		
	}

}

int main(void) {
	uint8_t j, index = 0;
 	uint32_t  small;
	setup();
	while (1) {
		small = 9999999;
		result[0] = pulses[0];
		result[1] = pulses[1];
		result[2] = pulses[2];
		for(j = 0;j < 3;j++) {
			if(small > result[j]) {
				small = result[j];
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
