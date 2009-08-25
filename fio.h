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

// extended version with "mask"-parameter to init

#ifndef __FIO_H 
#define __FIO_H

#include "types.h"

/* bit 0 in SCS register, port 0/1 are regular ports when bit 0 
is 0,  fast ports when bit 0 is 1. */
#define GPIOM                          0x00000001

/* see master definition file lpc230x.h for more details */
#define REGULAR_PORT_DIR_BASE          GPIO_BASE_ADDR + 0x08
#define REGULAR_PORT_DIR_INDEX         0x10

#define HS_PORT_DIR_BASE               FIO_BASE_ADDR + 0x00
#define HS_PORT_DIR_INDEX              0x20

#define HS_PORT_PIN_BASE               FIO_BASE_ADDR + 0x14
#define HS_PORT_PIN_INDEX              0x20

#define FAST_PORT          0x01
#define REGULAR_PORT       0x02


#define DIR_IN             0x00
#define DIR_OUT            0x01

#ifdef __cplusplus
extern "C"{
#endif

extern void GPIOInit( DWORD PortNum, DWORD PortType, DWORD PortDir, DWORD Mask);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* end __FIO_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/
