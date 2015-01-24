#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#define F_OSC F_CPU

void setup(void) {
	//USART INITIALIZATION
	UCSR0A = 0;
	UCSR0B = 0b10011000;
	UCSR0C = 0b00000110;
	UBRR0H = 0;
	UBRR0L = 12;
	sei();
}

ISR(USART_RX_vect) {

}

int main(void) {
	setup();
	return 0;
}
