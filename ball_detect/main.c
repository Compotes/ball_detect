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
#define NOT_SEEING_VALUE 32000
#define QUANTITY_OF_SENSORS 4
#define ULTRASONIC_SENSOR 4
#define TIME_WITHOUT_PULSE 100000
#define TIMER_OVERFLOW 2000000
#define waitForTX() while (!(UCSR0A & 1<<UDRE0))

volatile int32_t not_seeing[QUANTITY_OF_SENSORS];
volatile int32_t ends_of_pulses[QUANTITY_OF_SENSORS];
volatile int32_t lenghts_of_pulses[QUANTITY_OF_SENSORS];
volatile int32_t starts_of_pulses[QUANTITY_OF_SENSORS];
volatile int32_t vision_result[QUANTITY_OF_SENSORS];
volatile int32_t sensor_values[QUANTITY_OF_SENSORS];
volatile int32_t counter[QUANTITY_OF_SENSORS];
volatile uint32_t now;
volatile uint8_t pinstate, ct, changed_bits, portb_history = 0xFF;
volatile uint8_t my_address, address_of_message = 0;

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
    PORTC |= ((1 << DIP_SWITCH_1) |
              (1 << DIP_SWITCH_2) |
              (1 << DIP_SWITCH_3) |
              (1 << DIP_SWITCH_4) |
              (1 << DIP_SWITCH_5) |
              (1 << DIP_SWITCH_6));

    //INTERRUPTS INIT
    DDRB = 0;
    PORTB |= ((1 << BALL_SENSOR_1) |
              (1 << BALL_SENSOR_2) |
              (1 << BALL_SENSOR_3) |
              (1 << ULTRASONIC_SENSOR));
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
    uint8_t read_char = UDR0,  message;
    if(read_char == my_address) {
        if((address_of_message % 2) == 0) {
            now = vision_result[address_of_message / 2];
            message = now & 0xFF;
            now >>= 8;
            now &= 0xFF;
        } else {
            message = now;
        }
        address_of_message = (address_of_message + 1) % 6;
        sendc(message);
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
                if(ends_of_pulses[ct] < starts_of_pulses[ct]) {
                    ends_of_pulses[ct] += TIMER_OVERFLOW;
                }
                lenghts_of_pulses[ct] = ends_of_pulses[ct]
                                      - starts_of_pulses[ct];
                not_seeing[ct] = 0;
            }
        }
    }
    if(changed_bits & (1 << ULTRASONIC_SENSOR)) {
    }
}

int main(void) {
    int i;
    setup();
    while (1) {
        // Fourth sensor is ultrasonic, so we don't want to coinsider it.
        for(i = 0; i < QUANTITY_OF_SENSORS - 1; i++) {
            not_seeing[i]++;
            if(not_seeing[i] > TIME_WITHOUT_PULSE) {
                lenghts_of_pulses[i] = NOT_SEEING_VALUE;
            }
            vision_result[i] = lenghts_of_pulses[i];
        }
    }
    return 0;
}
