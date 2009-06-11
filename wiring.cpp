#include <LPC23xx.H>
#include <stdint.h>
#include "wiring.h"
#include "wiring_pins.h"
#include "fio.h"

void pinMode(uint8_t pin, uint8_t mode) {

   GPIOInit( PinTable[pin].PortNum, FAST_PORT, mode, (1 << PinTable[pin].PortPin) );

}

void digitalWrite(uint8_t pin, uint8_t val) {

uint32_t mask;

mask = (1 << PinTable[pin].PortPin);

		if ( val )
		{
			(*(volatile unsigned long *)(HS_PORT_PIN_BASE
				+ PinTable[pin].PortNum * HS_PORT_PIN_INDEX)) |= mask;
		}
		else
		{
			(*(volatile unsigned long *)(HS_PORT_PIN_BASE
				+ PinTable[pin].PortNum * HS_PORT_PIN_INDEX)) &= ~mask;
		}
} 

int digitalRead(uint8_t pin) {

int val;

   val = (*(volatile unsigned long *)(HS_PORT_PIN_BASE
         + PinTable[pin].PortNum * HS_PORT_PIN_INDEX) >> PinTable[pin].PortPin) & 1;

return val;
}
