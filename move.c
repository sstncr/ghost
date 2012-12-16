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

//This file holds all routines relevant to movement


#include "move.h"
#include <avr/io.h>
#include <stdlib.h>


/*
 speed1 is for left engine, speed2 is for right engine
 */

void speed(int speed1, int speed2) {
	if (abs(speed1) > MAX_SPEED)
		speed1 = MAX_SPEED;
	if (abs(speed2) > MAX_SPEED)
		speed2 = MAX_SPEED;
	OCR1A = abs(speed1);
	OCR1B = abs(speed2);
}

void move_forward(int speed1, int speed2) {
	MOVE_FORWARD;
	speed(speed1, speed2);
}

void move_back(int speed1, int speed2) {
	MOVE_BACK;
	speed(speed1, speed2);
}

//dir = 0 rotate to right
//else rotate to left
void rotate(int s1, int s2, char dir) {
	if (dir == 0) {
		ROTATE_RIGHT;
		speed(s1, s2);
	} else {
		ROTATE_LEFT;
		speed(s1, s2);
	}
}

void stop() {
	speed(0, 0);
}

//interpret parameters and call the right move routine
void move(int speed1, int speed2) {
	if ((speed1 >= 0) && (speed2 >= 0))
		move_forward(speed1, speed2);
	else if ((speed1 <= 0) && (speed2 >= 0))
		rotate(speed1, speed2, 1);
	else if ((speed1 >= 0) && (speed2 <= 0))
		rotate(speed1, speed2, 0);
	else
		move_back(speed1, speed2);
}

