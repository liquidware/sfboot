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

#ifndef  _SBL_IAP_H
#define  _SBL_IAP_H

#include "types.h"

extern const unsigned sector_start_map[];
extern const unsigned sector_end_map[];
extern const unsigned crp;

//unsigned write_flash(unsigned int * dst, unsigned char * src, unsigned no_of_bytes);
unsigned write_flash(unsigned int * dst, unsigned char * src, unsigned no_of_bytes, bool eraseSectorAllowed);
void execute_user_code(void);
BOOL user_code_present(void);
void erase_user_flash(void);
void check_isp_entry_pin(void);
void erase_user_flash(void);

typedef enum
{
PREPARE_SECTOR_FOR_WRITE=50,
COPY_RAM_TO_FLASH=51,
ERASE_SECTOR=52,
BLANK_CHECK_SECTOR=53,
READ_PART_ID=54,
READ_BOOT_VER=55,
COMPARE=56,
REINVOKE_ISP=57
}IAP_Command_Code;

#define CMD_SUCCESS 0
#define IAP_ADDRESS 0x7FFFFFF1

#endif /* _SBL_IAP_H */
