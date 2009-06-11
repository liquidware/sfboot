
#ifndef WIRING_H
#define WIRING_H

#include <stdint.h>
#include "wiring_pins.h"

#define INPUT  0
#define OUTPUT 1

#define HIGH 0x1
#define LOW  0x0

void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t val);
int digitalRead(uint8_t pin);

#endif