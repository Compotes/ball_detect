#include <avr/io.h>
#include <stdint.h> 
#include <util/delay.h>
#include <avr/interrupt.h>
#define F_OSC F_CPU

#define BALL_SENSOR_1 0
#define BALL_SENSOR_2 1
#define BALL_SENSOR_3 2
#define ULTRASONIC_SENSOR 4

volatile uint8_t portb_history = 0xFF; 


void setup(void) {
	//USART INITIALIZATION
	UCSR0A = 0;
	UCSR0B = 0b10011000;
	UCSR0C = 0b00000110;
	UBRR0H = 0;
	UBRR0L = 12;
	//INTERRUPTS INIT
	DDRB = 0;
	PORTB |= ((1 << BALL_SENSOR_1) | (1 << BALL_SENSOR_2) | (1 << BALL_SENSOR_3) | (1 << ULTRASONIC_SENSOR)); 
	PCICR |= 1;
	PCMSK0 |= 1;

	//TIMER SETUP
	TCCR1A = 0;
	TCCR1B |= 1;
	TCNT1H = 0;

	//TURN ON INTERRUPTS
	sei();
}

ISR(USART_RX_vect) {
	
}

ISR(PCINT0_vect) {
	uint8_t changed_bits;
	
	changed_bits = PINB ^ portb_history;
	portb_history = PINB;
	
	if(changed_bits & (1 << BALL_SENSOR_1)) {
			    	 	
    } else if(changed_bits & (1 << BALL_SENSOR_2)) {
	    	 	
    } else if(changed_bits & (1 << BALL_SENSOR_3)) {
	        
    } else if(changed_bits & (1 << ULTRASONIC_SENSOR)) {
		
	}
k88888888
}

int main(void) {
	setup();
	return 0;
}
