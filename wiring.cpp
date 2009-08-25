/*****************************************************************************
   Illuminato  X Machina - Secondary Bootloader
   Copyright (C) 2009 Christopher Ladden

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
   USA
 ******************************************************************************/
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
