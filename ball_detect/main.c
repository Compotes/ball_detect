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
volatile int pulse = 0;
volatile int pulses[4]; // averages of pulses
volatile int counters[4]; // counters of lenghts of pulses
volatile int helper[4]; // lenghts of pulses

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
        if(PINB & (1 << BALL_SENSOR_1)) {
			helper[0] = TCNT1H; 
	    	if(counters[0] < 10) counters[0]++; 
			else {
        		counters[0] = 0;
				pulses[0] = pulses[0]/10;
        	}  			    	 	
		} else {
			pulse = TCNT1H;
			if(pulse < helper[0]) {
				pulses[0] += pulse+1000000-helper[0];
				helper[0] = 0;
			}
		}
	} else if(changed_bits & (1 << BALL_SENSOR_2)) {
		if(PINB & (1 << BALL_SENSOR_2)) {
            helper[1] = TCNT1H;
            if(counters[1] < 10) counters[1]++;
            else {
                counters[1] = 0;
                pulses[1] = pulses[1]/10;
            }
        } else {
            pulse = TCNT1H;
            if(pulse < helper[1]) {
                pulses[1] += pulse+1000000-helper[1];
                helper[1] = 0;
            }
        }
	 	
    } else if(changed_bits & (1 << BALL_SENSOR_3)) {
		if(PINB & (1 << BALL_SENSOR_3)) {
            helper[2] = TCNT1H;
            if(counters[2] < 10) counters[2]++;
            else {
                counters[2] = 0;
                pulses[2] = pulses[2]/10;
            }
        } else {
            pulse = TCNT1H;
            if(pulse < helper[2]) {
                pulses[2] += pulse+1000000-helper[2];
                helper[2] = 0;
            }
        }
        
    } else if(changed_bits & (1 << ULTRASONIC_SENSOR)) {
		
	}
}

int main(void) {
	setup();
	return 0;
}
