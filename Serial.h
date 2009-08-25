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
#ifndef SERIAL_H
#include <stdint.h>

#define UART_BUFFSIZE 64

#define IER_RBR   0x01
#define IER_THRE	0x02
#define IER_RLS   0x04

#define IIR_PEND	0x01
#define IIR_RLS   0x03
#define IIR_RDA   0x02
#define IIR_CTI   0x06
#define IIR_THRE	0x01

#define LSR_RDR   0x01
#define LSR_OE		0x02
#define LSR_PE		0x04
#define LSR_FE		0x08
#define LSR_BI		0x10
#define LSR_THRE	0x20
#define LSR_TEMT	0x40
#define LSR_RXFE	0x80

void SerialBegin(uint8_t portNum, uint32_t baud);
void SerialStr(uint8_t portNum, char * str);
int SerialWrite(uint8_t portNum, int ch);
int SerialAvailable(uint8_t portNum);
int SerialRead(uint8_t portNum);
void SerialFlush(uint8_t portNum);
int  SerialBusyRead(uint8_t portNum);
#endif
