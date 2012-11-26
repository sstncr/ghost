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

#ifndef SERIAL_H_
#define SERIAL_H_

void serialWriteChar(char data);
char serialRead();
void serialWrite(const char *data);
void serialWriteInt(int data);

#endif /* SERIAL_H_ */
