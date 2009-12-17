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
#ifndef __TYPES_H__
#define __TYPES_H__

#ifndef NULL
#define NULL    ((void *)0)
#endif

#ifndef FALSE
#define FALSE   (0)
#endif

#ifndef TRUE
#define TRUE    (1)
#endif

typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;
typedef unsigned int   BOOL;

typedef volatile signed char vint8_t;
typedef volatile unsigned char vuint8_t;

typedef volatile signed short vint16_t;
typedef volatile unsigned short vuint16_t;

typedef volatile signed int vint32_t;
typedef volatile unsigned int vuint32_t;

typedef volatile signed long long vint64_t;
typedef volatile unsigned long long vuint64_t;

#endif  /* __TYPES_H__ */
