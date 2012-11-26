/*
 * Copyright 2012 Stefan Stanacar
 *
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 */


#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <stdlib.h>

#include "naluca.h" // main header
#include "serial.h" // routines used in serial comm
#include "move.h" // move related routines

int main() {

	char sstatus, fstatus; //holds sensor status
	cli();	//disable global interrupts
	init();	//setup basic stuff like data direction for ports, adc, timers,etc
	autonom = 0;
	sei();	//enable global interrupts

	serialWrite("\n\rWelcome!\n\r");
	serialWrite("To switch between manual/autonomous modes press: 'm'\n\r");
	serialWrite("In manual mode use:\n\r w/a/s/d for direction and 'z' to stop.\n\r");
	serialWrite("l - flip LED state; r - show sensors status.\n\r");
	serialWrite("v - change speed.\n\r");
	serialWrite("Reference value for the field sensor: ");
	serialWriteInt(color);
	serialWrite("\n\r");

	PORTD |= (1 << LED); // turn led on to signal we are ready

	for (;;) {
		if (set_new_speed) {
			cli();
			stop();
			serialWrite("\n\rOld Speed is: ");
			serialWriteInt(cspeed);
			serialWrite("\n\rNew speed (enter a number between 10 - 99 ): ");
			char ch = serialRead();
			if ((ch >= '1') && (ch <= '9')) {
				char new_speed[2];
				new_speed[0] = ch;
				ch = serialRead();
				if ((ch >= '0') && (ch <= '9')) {
					new_speed[1] = ch;
					wdt_enable(0);
					//enable watchdog with 500ms timeout, just in case input is garbage
					cspeed = atoi(new_speed);
					wdt_disable();
					serialWrite("\n\rNew speed will be: ");
					serialWriteInt(cspeed);
				} else
					serialWrite(
							"\n\rYou entered invalid data, I'll use the old speed.");

			} else
				serialWrite(
						"\n\rYou entered invalid data, I'll use the old speed.");
			set_new_speed = 0;
			sei();
		}
		if (autonom) {

			sstatus = get_front_sensors();
			do_move(sstatus);

			if (get_field_sensor_status()) { //hmm seems we hit checkpoint, but it could be a line
				_delay_ms(10); //delay a little bit and try again
				if (get_field_sensor_status()) {
					cli();
					stop();
					PORTD |= (1 << LED);
					serialWrite("\n\r !!! CHECKPOINT !!!\n\r");
					dump_sensors_values();
					_delay_ms(1000);
					serialWrite("Values sent, moving on...\n\r");
					_delay_ms(500);
					sstatus = get_front_sensors();
					do_move(sstatus);
					_delay_ms(500);
					fstatus = get_field_sensor_status();
					sei();
					while (fstatus && autonom) {
						fstatus = get_field_sensor_status();
						sstatus = get_front_sensors();
						do_move(sstatus);
					}

				}
			}

			else
				PORTD &= ~(1 << LED);

		}
	}
	return 0;
}

void init() {
	wdt_disable();
	//disable watchdog

	//set data directions - sensors
	DDRA &= ~((1 << PORTA0) | (1 << PORTA1) | (1 << PORTA2));
	DDRB &= ~((1 << PORTB0) | (1 << PORTB1));
	DDRD &= ~(1 << PORTD2); //set data dir input for button

	SET_DD_PWM; //set data direction for pwm outputs
	SET_DD_IN;  //and direction/inputs to driver

	DDRD |= (1 << LED); //set dd out for led
	PORTD &= ~(1 << LED); //and set it off

	//enable pull-ups
	PORTA |= (1 << PORTA2);
	PORTB |= (1 << PORTB0) | (1 << PORTB1);
	PORTD |= (1 << PORTD2);
	PORTC = 0xFF;

	usart_init(BAUD_PRESCALE);

	//init timers for phase correct PWM; prescaler 1, icr1 = top = 100 => fpwm = fclk/2 * 1 * 100 = 36.84 khz
	TCCR1A |= (1 << WGM11) | (1 << COM1A1) | (1 << COM1B1);
	TCCR1B |= (1 << WGM13) | (1 << CS10);
	ICR1 = MAX_SPEED;

	move(0,0);  //move forward with 0 speed, our default state.

	//enable ADC and set prescaler to 64
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1);
	ADMUX = (1 << REFS0); //set AVCC as voltage reference

	calibrate_field_sensor(); //get a calibration for black

}

void usart_init(unsigned int ubrr) {
	//usart init stuff
	UBRRH = (unsigned char) (ubrr >> 8); //set baud rate
	UBRRL = (unsigned char) ubrr;
	UCSRB |= (1 << RXEN) | (1 << TXEN); //enable hw
	UCSRC |= (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1); //8 bit size

	UCSRB |= (1 << RXCIE); // enable the USART Recieve Complete interrupt (USART_RXC)

}

void do_move(char sstatus) {

	switch (sstatus) {
	case 0:   //clear, move fw
		move(cspeed, cspeed);
		break;
	case 1:   //left trigerred, move right
		move(tspeed, -tspeed);
		break;
	case 2:  //right trig, move left
		move(-tspeed, tspeed);
		break;
	case 7:   //all trig
	case 3:   //left and right trig
	case 4:    //middle trig, move back then right
		move(-cspeed, -cspeed);
		_delay_ms(100);
		move(tspeed, -tspeed);
		_delay_ms(30);
		break;
	case 5:     //left and middle trig
		move(-cspeed, -cspeed);
		_delay_ms(80);
		move(tspeed, -tspeed);
		_delay_ms(20);
		break;
	case 6:    //right and middle trig
		move(-cspeed, -cspeed);
		_delay_ms(80);
		move(-tspeed, tspeed);
		_delay_ms(20);
		break;
	}

}

int read_adc(char adc_input) {
	ADMUX = adc_input | (1 << REFS0);
	// start the AD conversion
	ADCSRA |= 0x40;
	// wait for the AD conversion to complete
	while ((ADCSRA & 0x10) == 0)
		;
	ADCSRA |= 0x10;
	return ADCW ;
}


char get_field_sensor_status() {

	int fsensor = get_field_sensor_value();

	if ((fsensor >= (color - color/3)) && (fsensor <= (color + color / 3))) {

		return 1;
	}
	return 0;
}

int get_field_sensor_value() {
	return read_adc(1);  //field sensor is on ADC1
}

void calibrate_field_sensor() {
	int value;
	char i;

	get_field_sensor_value(); //dummy read
    value = 0;
	for (i = 0; i < 10; i++) {
		 get_field_sensor_value();
		_delay_ms(10);
	}
	for (i = 0; i < 100; i++) {
			 value += get_field_sensor_value();
			_delay_ms(10);
		}
	value /= 100;
	for (i = 0; i < 100; i++) {
				 value += get_field_sensor_value();
				_delay_ms(10);
	}
	value /= 100;

	color = value;
}

char get_front_sensors() {

	char val;

	val = 0;
	if (!(PINB & (1 << PINB0)))   //if left sensor sees smth set val to 1
		val += 1;
	if (!(PINB & (1 << PINB1))) //if right sensor sees smth add 2
		val += 2;
	if (!(PINA & (1 << PINA2))) //middle sensor
		val += 4;
	/*
	 0 sensors clear, 1 left sees something, 2 right sees smth, 3 left+right
	 4 middle sensor, 5 left+middle, 6 right+middle, 7 left+right+middle

	 */
	return val;

}

void dump_sensors_values() {
	serialWrite("Front sensors status: ");
	serialWriteInt(get_front_sensors());
	serialWrite("\n\rField sensor status: ");
	serialWriteInt(get_field_sensor_status());
	serialWrite(" (0 - inactive, 1 - active) with actual value: ");
	serialWriteInt(get_field_sensor_value());
	serialWrite("\n\r");

}

void interpret_input(char input) {

	if (input == 'm') {
		autonom = 1 - autonom;
		stop();
		if (autonom)
			serialWrite("Autonomous mode\n\r");
		else
			serialWrite("Manual mode\n\r");
	}
	if (input == 'r')
		dump_sensors_values();
	if (input == 'v')
		set_new_speed = 1;
	if (!autonom) {
		switch (input) {
		case 'e':
			stop();
			break;
		case 'w':
			move(cspeed, cspeed);
			break;
		case 's':
			move(-cspeed, -cspeed);
			break;
		case 'a':
			move(tspeed / 2, tspeed);
			break;
		case 'd':
			move(tspeed, tspeed / 2);
			break;
		case 'l':
			PORTD ^= (1 << LED); //flip led status
			break;
		}
	}
}

ISR(USART_RXC_vect) {
	char data;
	data = UDR;
	interpret_input(data);
}

