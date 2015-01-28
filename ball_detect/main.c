#include <avr/io.h>
#include <stdint.h> 
#include <util/delay.h>
#include <avr/interrupt.h>
#include <inttypes.h>
#define F_OSC F_CPU

#define BALL_SENSOR_1 0
#define BALL_SENSOR_2 1
#define BALL_SENSOR_3 2
#define ULTRASONIC_SENSOR 4

volatile int32_t pulse = 0;
volatile int32_t pulses[4]; // averages of pulses
volatile int32_t counters[4]; // counters of lenghts of pulses
volatile int32_t helper[4]; // lenghts of pulses

void setup(void) {
	//USART INITIALIZATION
	//UCSR0A = 0;
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
	TCCR1B = 2 << CS10;
	TCNT1H = 0;

	//TURN ON INTERRUPTS
	sei();
	
	DDRD |= 1 << 3;
}

ISR(USART_RX_vect) {
	
}

ISR(PCINT0_vect) {
	static uint8_t portb_history = 0xFF;
	uint8_t changed_bits;
	uint8_t pinstate;

	pinstate = PINB;
	PORTD ^= 1 << 3;
	
	changed_bits = pinstate ^ portb_history;
	portb_history = pinstate;
	
#define ct pinstate
	for (ct = 0;ct < 4;ct++) {
		if (changed_bits & (1 << ct)) {
       		if (pinstate & (1 << ct)) {
				helper[ct] = TCNT1H; 
		   		if(counters[ct] < 10) 
					counters[ct]++; 
				else {
	        		counters[ct] = 0;
					pulses[ct] = pulses[ct]/10;
   	     		}  			    	 	
			} else {
				pulse = TCNT1H;
				if(pulse < helper[ct]) {
					pulses[ct] += pulse+1000000-helper[ct];
					helper[ct] = 0;
				} else {
					pulses[ct] += pulse - helper[ct];
				}
			}
		}
	}
    
	if(changed_bits & (1 << ULTRASONIC_SENSOR)) {
		
	}
#undef ct
}

#define waitForTX() while (!(UCSR0A & 1<<UDRE0))

void sendc(uint8_t ch) {
  waitForTX();
  UDR0 = ch;
}

int main(void) {
	uint8_t i;
	setup();

	i = 0;
	while (1) {
		sendc(i + '0');
		sendc(' ');
		sendc('0' + ((pulses[i]/2 / 10000) % 10));
		sendc('0' + ((pulses[i]/2 /  1000) % 10));
		sendc('0' + ((pulses[i]/2 /   100) % 10));
		sendc('0' + ((pulses[i]/2 /    10) % 10));
		sendc('0' + ((pulses[i]/2 /     1) % 10));
		sendc('\r');
		sendc('\n');

		i = (i + 1) & 0x3;
	}

	return 0;
}
