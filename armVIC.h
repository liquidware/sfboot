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
#ifndef INC_ARM_VIC_H
#define INC_ARM_VIC_H

/******************************************************************************
 *
 * MACRO Name: ISR_ENTRY()
 *
 * Description:
 *    This MACRO is used upon entry to an ISR.  The current version of
 *    the gcc compiler for ARM does not produce correct code for
 *    interrupt routines to operate properly with THUMB code.  The MACRO
 *    performs the following steps:
 *
 *    1 - Adjust address at which execution should resume after servicing
 *        ISR to compensate for IRQ entry
 *    2 - Save the non-banked registers r0-r12 and lr onto the IRQ stack.
 *    3 - Get the status of the interrupted program is in SPSR.
 *    4 - Push it onto the IRQ stack as well.
 *
 *****************************************************************************/
#define ISR_ENTRY() asm volatile(" sub   lr, lr,#4\n" \
                                 " stmfd sp!,{r0-r12,lr}\n" \
                                 " mrs   r1, spsr\n" \
                                 " stmfd sp!,{r1}")

/******************************************************************************
 *
 * MACRO Name: ISR_EXIT()
 *
 * Description:
 *    This MACRO is used to exit an ISR.  The current version of the gcc
 *    compiler for ARM does not produce correct code for interrupt
 *    routines to operate properly with THUMB code.  The MACRO performs
 *    the following steps:
 *
 *    1 - Recover SPSR value from stack       
 *    2 - and restore  its value                   
 *    3 - Pop the return address & the saved general registers from
 *        the IRQ stack & return
 *
 *****************************************************************************/
#define ISR_EXIT()  asm volatile(" ldmfd sp!,{r1}\n" \
                                 " msr   spsr_c,r1\n" \
                                 " ldmfd sp!,{r0-r12,pc}^")

/******************************************************************************
 *
 * Function Name: disableIRQ()
 *
 * Description:
 *    This function sets the IRQ disable bit in the status register
 *
 * Calling Sequence: 
 *    void
 *
 * Returns:
 *    previous value of CPSR
 *
 *****************************************************************************/
unsigned disableIRQ(void);

/******************************************************************************
 *
 * Function Name: enableIRQ()
 *
 * Description:
 *    This function clears the IRQ disable bit in the status register
 *
 * Calling Sequence: 
 *    void
 *
 * Returns:
 *    previous value of CPSR
 *
 *****************************************************************************/
unsigned enableIRQ(void);

/******************************************************************************
 *
 * Function Name: restoreIRQ()
 *
 * Description:
 *    This function restores the IRQ disable bit in the status register
 *    to the value contained within passed oldCPSR
 *
 * Calling Sequence: 
 *    void
 *
 * Returns:
 *    previous value of CPSR
 *
 *****************************************************************************/
unsigned restoreIRQ(unsigned oldCPSR);

/******************************************************************************
 *
 * Function Name: disableFIQ()
 *
 * Description:
 *    This function sets the FIQ disable bit in the status register
 *
 * Calling Sequence: 
 *    void
 *
 * Returns:
 *    previous value of CPSR
 *
 *****************************************************************************/
unsigned disableFIQ(void);

/******************************************************************************
 *
 * Function Name: enableFIQ()
 *
 * Description:
 *    This function clears the FIQ disable bit in the status register
 *
 * Calling Sequence: 
 *    void
 *
 * Returns:
 *    previous value of CPSR
 *
 *****************************************************************************/
unsigned enableFIQ(void);

/******************************************************************************
 *
 * Function Name: restoreIRQ()
 *
 * Description:
 *    This function restores the FIQ disable bit in the status register
 *    to the value contained within passed oldCPSR
 *
 * Calling Sequence: 
 *    void
 *
 * Returns:
 *    previous value of CPSR
 *
 *****************************************************************************/
unsigned restoreFIQ(unsigned oldCPSR);

#endif
