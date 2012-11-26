/*
 * naluca.h
 *
 * Copyright 2012 Stefan Stanacar
 *
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 */
#ifndef NALUCA_H_
#define NALUCA_H_

#define F_CPU 7372800UL
#define USART_BAUDRATE 9600  //our serial comm baud rate
#define BAUD_PRESCALE ((F_CPU/16/USART_BAUDRATE) - 1) //fancy way of selecting baud for usart later
#define LED PORTD6

/* Global Variables.
 *
 */

int color = 200; // it's a good start value for a dark color before calibration
char cspeed = 20; //cruise speed
char tspeed = 30; //turn speed
volatile char set_new_speed = 0; //flag used when we want to change the speed from user input
volatile char autonom; //flag for autonomous/manual mode

/*
 * Function definitions
 */

void usart_init(unsigned int ubrr);
void init();
int read_adc(char adc_input);
char get_field_sensor_status();
int get_field_sensor_value();
void calibrate_field_sensor();
char get_front_sensors();
void do_move(char sstatus);
void dump_sensors_values();
void interpret_input(char input);

#endif /* NALUCA_H_ */
