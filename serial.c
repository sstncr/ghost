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


// This file holds all routines relevant to serial communication

#include <avr/io.h>
#include "serial.h"
#include <stdlib.h>

//serial write one char
void serialWriteChar(char data) {
	while (!(UCSRA & (1 << UDRE)))
		;

	UDR = data;
}

//serial read one char
char serialRead() {

	while (!(UCSRA & (1 << RXC)))
		;
	return UDR ;
}

//serial write string
void serialWrite(const char *data) {

	while (*data) {
		serialWriteChar(*data);
		data++;
	}

}

void serialWriteInt(int data) {
	char sdata[6];
	itoa(data, sdata, 10);
	serialWrite(sdata);

}
