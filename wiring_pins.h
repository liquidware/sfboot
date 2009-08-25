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
#ifndef WIRING_PINS_H
#define WIRING_PINS_H

#define kPinCount 43 /* The total number of pins in the PinTable */

typedef enum {
   eRXTX,
   eDIGITAL,
   eANALOG,
   eNumPinTypes,
} PIN_TYPE_T; 

typedef enum {
   eP0,
   eP1,
   eP2,
   eP3,
   eP4,
} PORTID_T;

typedef struct {
   uint8_t     PinId;
   PORTID_T    PortNum;
   uint8_t     PortPin;
   PIN_TYPE_T  PinIsA;
} PIN_DESC_T;

extern PIN_DESC_T PinTable[];


#endif