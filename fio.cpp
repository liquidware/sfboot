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

#include "LPC23xx.h"
#include "types.h"
#include "fio.h"

/*****************************************************************************
** Function name:   GPIOInit
**
** Descriptions:    initialize GPIO port
**
** parameters:      port number and port type, Fast I/O or
**                  regular GPIO, direction, Mask
** Returned value:  None
**
*****************************************************************************/
void GPIOInit( DWORD PortNum, DWORD PortType, DWORD PortDir, DWORD Mask )
{
	if ( (PortType == REGULAR_PORT) && ((PortNum == 0) || (PortNum == 1)) )
	{
		SCS &= ~GPIOM;    /* set GPIOx to use regular I/O */
		if ( PortDir == DIR_OUT )
		{
			(*(volatile unsigned long *)(REGULAR_PORT_DIR_BASE
				+ PortNum * REGULAR_PORT_DIR_INDEX)) |= Mask;
		}
		else
		{
			(*(volatile unsigned long *)(REGULAR_PORT_DIR_BASE
				+ PortNum * REGULAR_PORT_DIR_INDEX)) &= ~Mask;
		}
	}
	else if ( PortType == FAST_PORT )
	{
		if ( (PortNum == 0) || (PortNum == 1) )
		{
			SCS |= GPIOM; /* set GPIOx to use Fast I/O */
		}
		if ( PortDir == DIR_OUT )
		{
			(*(volatile unsigned long *)(HS_PORT_DIR_BASE
				+ PortNum * HS_PORT_DIR_INDEX)) |= Mask;
		}
		else
		{
			(*(volatile unsigned long *)(HS_PORT_DIR_BASE
				+ PortNum * HS_PORT_DIR_INDEX)) &= ~Mask;
		}
	}
	return;
}

/*********************************************************************************
**                            End Of File
*********************************************************************************/
