/******************************************************************************/
/* SERIAL.C: Low Level Serial Routines                                        */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <LPC23xx.H>                     /* LPC23xx definitions               */
#include "Serial.h"

#define UART0                            /* Use UART 0 for printf             */

/* If UART 0 is used for printf                                               */
#ifdef UART0
  #define UxFDR  U0FDR
  #define UxLCR  U0LCR
  #define UxDLL  U0DLL
  #define UxDLM  U0DLM
  #define UxLSR  U0LSR
  #define UxTHR  U0THR
  #define UxRBR  U0RBR
/* If UART 1 is used for printf                                               */
#elif defined(UART1)
  #define UxFDR  U1FDR
  #define UxLCR  U1LCR
  #define UxDLL  U1DLL
  #define UxDLM  U1DLM
  #define UxLSR  U1LSR
  #define UxTHR  U1THR
  #define UxRBR  U1RBR
#endif

/* Initialize Serial Interface       */
void SerialBegin(int baud)  
{               
  #ifdef UART0
    PINSEL0 |= 0x00000050;               /* Enable TxD0 and RxD0              */
  #elif defined (UART1)
    PINSEL0 |= 0x40000000;               /* Enable TxD1                       */
    PINSEL1 |= 0x00000001;               /* Enable RxD1                       */
  #endif
  UxFDR    = 0;                          /* Fractional divider not used       */
  UxLCR    = 0x83;                       /* 8 bits, no Parity, 1 Stop bit     */

  // UART Baud Rate Divisor Calculation
  //   Make sure PCLK is 16 times greater 
  //   than the desired baud rate
  // 
  // CCLK  = PLL Freq / 4           = 288Mhz / 4 = 72Mhz
  // PCLK  = CCLK / 4               = 72MHz / 4 = 18Mhz
  // UxDLL = (PCLK / 16) * (1/Baud) = (18Mhz / 16) * (1/9600) = 117 
  //  UxDLL    = 117;                         /* Set the UART baud rate divisor, 9600 */
  UxDLL    = 20;                         /* Set the UART baud rate divisor, 57600 */
  UxDLM    = 0;                          /* High divisor latch = 0            */
  UxLCR    = 0x03;                       /* DLAB = 0                          */
}


/* Write character to Serial Port    */
/* Implementation of putchar (also used by printf function to output data)    */
int SerialWrite(int ch)  
{                 

  while (!(UxLSR & 0x20));

  return (UxTHR = ch);
}

/* Write string to Serial Port    */
void SerialStr(char * str)
{
   while (*str) {
   	SerialWrite(*str);
   	str++;
   	}
} 

/* Read character from Serial Port   */
int  SerialRead(void)
{                     
  while (!(UxLSR & 0x01));

  return (UxRBR);
}
