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

#ifndef MOVE_H_
#define MOVE_H_

// control port for motors input and pwm
#define MOTOR_PORT PORTC
//inputs for left motor
#define LEFT_MOTOR_IN1 PORTC0
#define LEFT_MOTOR_IN2 PORTC1
//inputs for right motor
#define RIGHT_MOTOR_IN1 PORTC3
#define RIGHT_MOTOR_IN2 PORTC2
//pwm pins
#define LEFT_MOTOR_PWM PORTD5
#define RIGHT_MOTOR_PWM PORTD4

// data direction macros
#define SET_DD_IN DDRC |= (1 << LEFT_MOTOR_IN1) | (1<<LEFT_MOTOR_IN2) | (1 << RIGHT_MOTOR_IN1) | (1 << RIGHT_MOTOR_IN2)
#define SET_DD_PWM DDRD |= (1<<LEFT_MOTOR_PWM) | (1<<RIGHT_MOTOR_PWM)

//move directions macros
#define MOVE_FORWARD { MOTOR_PORT |= (1<<LEFT_MOTOR_IN1) | (1<<RIGHT_MOTOR_IN1); \
		MOTOR_PORT &= ~((1<<LEFT_MOTOR_IN2) | (1<<RIGHT_MOTOR_IN2)); }

#define MOVE_BACK { MOTOR_PORT |= (1<<LEFT_MOTOR_IN2) | (1<<RIGHT_MOTOR_IN2); \
		MOTOR_PORT &= ~((1<<LEFT_MOTOR_IN1) | (1<<RIGHT_MOTOR_IN1)); }

#define ROTATE_LEFT { MOTOR_PORT |= (1<<LEFT_MOTOR_IN2) | (1<<RIGHT_MOTOR_IN1); \
		MOTOR_PORT &= ~((1<<LEFT_MOTOR_IN1) | (1<<RIGHT_MOTOR_IN2)); }

#define ROTATE_RIGHT { MOTOR_PORT |= (1<<LEFT_MOTOR_IN1) | (1<<RIGHT_MOTOR_IN2); \
		MOTOR_PORT &= ~((1<<LEFT_MOTOR_IN2) | (1<<RIGHT_MOTOR_IN1)); }

//pwm max speed, used to control TOP
#define MAX_SPEED 100

void speed(int speed1, int speed2);
void move_forward(int speed1, int speed2);
void move_back(int speed1, int speed2);
void rotate(int s1, int s2, char dir);
void stop();
void move(int speed1, int speed2);

#endif /* MOVE_H_ */
