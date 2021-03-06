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

#include <stdlib.h>
#include <stdint.h>
#include <LPC23xx.H>                     /* LPC23xx definitions               */
#include "Serial.h"
#include "types.h"
#include "armVIC.h"
#include "irq.h"
#include "sys_config.h"

//**************************************
//* TYPES
//**************************************

/* The names for our UARTs */
typedef enum {
   UART0,
   UART1,
   UART2,
   UART3,
   NUM_UARTS
} UART_NAME_T;

/* UART Register Description */
typedef struct {

   union {
      vuint32_t RBR; 
      vuint32_t DLL;
      vuint32_t THR;
   } R1;

   union {
      vuint32_t DLM;
      vuint32_t R;
      struct {
         vuint32_t RBR    :1;
         vuint32_t THRE   :1;
         vuint32_t RLS    :1;
         vuint32_t Spare2 :29;
      }IER;
   } R2;

   union {
      vuint32_t IIR;
      vuint32_t FCR;
   } R3;

   vuint32_t LCR;
   vuint32_t MCR;
   vuint32_t LSR;
   vuint32_t MSR;
   vuint32_t SCR;
   vuint64_t ACR;
   vuint16_t FDR;
} UART_REG_T;

/* UART Pins Description */
typedef struct {
   /* TX */
      vuint32_t *TxSelReg;
      uint32_t   TxSelValue;
      uint32_t   TxMask;
   /* RX */
      vuint32_t *RxSelReg;
      uint32_t   RxSelValue;
      uint32_t   RxMask;
} UART_PIN_T;


/* UART interrupt Description */
typedef struct {
   uint32_t *  IntFunc;
   uint8_t     IntNumber;
   uint8_t     Priority;
} INT_DESC_T;

/* High-level UART structure */
typedef struct {
	vuint8_t RxBuffer[UART_BUFFSIZE];
	vuint32_t RxHead;
	vuint32_t RxTail;
	vuint32_t Status;
	vuint16_t TxEmpy;
} vUART_T;

//**************************************
//* LOCAL FUNCTION PROTOTYPES
//**************************************

void SerialISR(void);
void Serial0ISR(void) __attribute__((naked));
void Serial1ISR(void) __attribute__((naked));
void Serial2ISR(void) __attribute__((naked));
void Serial3ISR(void) __attribute__((naked));


//**************************************
//* LOCAL VARIABLES
//**************************************

/* The quad UART High-level data */
volatile vUART_T uartData[NUM_UARTS];


/* The quad UART Interrupts */
volatile INT_DESC_T uartInts[NUM_UARTS] = {
                                 {
                                   /* IntFunc   */ (uint32_t *)Serial0ISR,
                                   /* IntNumber */ UART0_INT,
                                   /* Priority  */ HIGHEST_PRIORITY,
                                 },
                                 {
                                   /* IntFunc   */ (uint32_t *)Serial1ISR,
                                   /* IntNumber */ UART1_INT,
                                   /* Priority  */ HIGHEST_PRIORITY,
                                 },
                                 {
                                   /* IntFunc   */ (uint32_t *)Serial2ISR,
                                   /* IntNumber */ UART2_INT,
                                   /* Priority  */ HIGHEST_PRIORITY,
                                 },
                                 {
                                   /* IntFunc   */ (uint32_t *)Serial3ISR,
                                   /* IntNumber */ UART3_INT,
                                   /* Priority  */ HIGHEST_PRIORITY,
                                 },

};


/* The quad UART pins */
volatile UART_PIN_T uartPins[NUM_UARTS] = {
                          {          
                                      /* TxSelReg    */ (vuint32_t *)PINSEL_BASE_ADDR,
                                      /* TxSelValue  */ (1UL<<4),
                                      /* TxMask      */ (3UL<<4),
                                      /* RxSelReg    */ (vuint32_t *)PINSEL_BASE_ADDR,
                                      /* RxSelValue  */ (1UL<<6),
                                      /* RxMask      */ (3UL<<6), 
                          },
                          {          
                                      /* TxSelReg    */ (vuint32_t *)PINSEL_BASE_ADDR,
                                      /* TxSelValue  */ (1UL<<30),
                                      /* TxMask      */ (3UL<<30),
                                      /* RxSelReg    */ (vuint32_t *)(PINSEL_BASE_ADDR + 0x04),
                                      /* RxSelValue  */ (1UL<<0),
                                      /* RxMask      */ (3UL<<0), 
                          },
                          {          
                                      /* TxSelReg    */ (vuint32_t *)PINSEL_BASE_ADDR,
                                      /* TxSelValue  */ (1UL<<20),
                                      /* TxMask      */ (3UL<<20),
                                      /* RxSelReg    */ (vuint32_t *)PINSEL_BASE_ADDR,
                                      /* RxSelValue  */ (1UL<<22),
                                      /* RxMask      */ (3UL<<22), 
                          },
                          {          
                                      /* TxSelReg    */ (vuint32_t *)PINSEL_BASE_ADDR,
                                      /* TxSelValue  */ (2UL<<0),
                                      /* TxMask      */ (3UL<<0),
                                      /* RxSelReg    */ (vuint32_t *)PINSEL_BASE_ADDR,
                                      /* RxSelValue  */ (2UL<<2),
                                      /* RxMask      */ (2UL<<2), 
                          }
};

/* The quad UART registers */
volatile UART_REG_T *uartReg[NUM_UARTS] = {
                                             (volatile UART_REG_T*)UART0_BASE_ADDR,
                                             (volatile UART_REG_T*)UART1_BASE_ADDR,
                                             (volatile UART_REG_T*)UART2_BASE_ADDR,
                                             (volatile UART_REG_T*)UART3_BASE_ADDR,
                                           };

//**************************************
//* LOCAL FUNCTIONS
//**************************************

//***************************************
//* The Serial Interrupt Service routine
//* used for all 4 uarts.                     
void SerialISR(uint8_t portNum) {
   BYTE IIRValue;
   volatile UART_REG_T* uart = uartReg[portNum];

   IIRValue = uart->R3.IIR;
   IIRValue >>= 1;        /* skip pending bit in IIR */
   IIRValue &= 0x07;      /* check bit 1~3, interrupt identification */

   /* Receive Data Available */
   if ( IIRValue == IIR_RDA ) {

      vuint32_t index;
      vuint32_t *head;

      head = &uartData[portNum].RxHead;
      index = (*head + 1) % UART_BUFFSIZE;

      if (index != uartData[portNum].RxTail) {

         /* Store the received character into the buffer */
         uartData[portNum].RxBuffer[*head] = uart->R1.RBR;
         *head = index;

      }
   }
   else if ( IIRValue == IIR_CTI ) { /* Character timeout indicator */

      /* Character Time-out indicator */
      uartData[portNum].Status |= 0x100;     /* Bit 9 as the CTI error */

   }
   else if ( IIRValue == IIR_THRE ) {   /* THRE, transmit holding register empty */
	  
	  BYTE LSRValue;

      /* THRE interrupt */
      LSRValue = uart->LSR;   		/* Check status in the LSR to see if
                                 		valid data in U0THR or not */
      if ( LSRValue & LSR_THRE ) {
         uartData[portNum].TxEmpy = 1;
      }
      else {
         uartData[portNum].TxEmpy = 0;
      }
   }
}

void Serial0ISR(void) {
   
   /* Perform proper ISR entry so thumb-interwork works properly */
   ISR_ENTRY();

   /* Handle the UART Interrupt */
   SerialISR(UART0);

   /* Clear this interrupt from the VIC */
   VICVectAddr = 0x00000000;  
   
   /* Recover registers and return */           
   ISR_EXIT();                           

}

void Serial1ISR(void) {
   
   /* Perform proper ISR entry so thumb-interwork works properly */
   ISR_ENTRY();

   /* Handle the UART Interrupt */
   SerialISR(UART1);

   /* Clear this interrupt from the VIC */
   VICVectAddr = 0x00000000;  
   
   /* Recover registers and return */           
   ISR_EXIT();   

}

void Serial2ISR(void) {
   
   /* Perform proper ISR entry so thumb-interwork works properly */
   ISR_ENTRY();

   /* Handle the UART Interrupt */
   SerialISR(UART2);

   /* Clear this interrupt from the VIC */
   VICVectAddr = 0x00000000;  
   
   /* Recover registers and return */           
   ISR_EXIT();   

}

void Serial3ISR(void) {

   /* Perform proper ISR entry so thumb-interwork works properly */
   ISR_ENTRY();

   /* Handle the UART Interrupt */
   SerialISR(UART3);

   /* Clear this interrupt from the VIC */
   VICVectAddr = 0x00000000;  
   
   /* Recover registers and return */           
   ISR_EXIT();   

}

//**************************************
//* GLOBAL FUNCTIONS
//**************************************

/* NOTE: This findDivisorAndPrescale is code by
 *
 *  "Jan Vanek" <j3vanek@googlemail.com>
 *
 * who generously posted it to the LPC2000 mailing list on Thu, 12 Mar 2009.
 */

uint32_t findDivisorAndPrescale(uint32_t frequency, uint32_t baudRate, bool allowSmallerBaudRate, uint32_t& mulVal, uint32_t& divAddVal) {
  // The FDR is set to prescale the UART_CLK with factor M/(M+D),
  // where both M and D are in <0, 15>. This is to achieve smaller
  // error when calculating the divisor (the UnDL).
  //                UART_CLK      M
  // UARTbaudrate = --------- x -----
  //                16 x UnDL   (M+D)

  uint32_t divisor = frequency / (16 * baudRate);
  uint32_t baudRateError = (frequency / (16 * divisor)) - baudRate;

  if (allowSmallerBaudRate) {
    uint32_t otherDivisor = divisor + 1;
    uint32_t otherBaudRateError = baudRate - (frequency / (16 * otherDivisor));
    if (baudRateError > otherBaudRateError) {
      divisor = otherDivisor;
      baudRateError = otherBaudRateError;
    }
  }
  mulVal = 1;
  divAddVal = 0;
  for (uint32_t mv = 0; ++mv < 16;) {
    for (uint32_t dav = 0; ++dav < 16;)  {  // ACKLEY: SHOULDN'T THIS BE: '++dav < mv' (according to manual 2.12 pg 426???
      uint32_t prescaledClock = (frequency * mv) / (16 * (mv + dav));
      uint32_t newDivisor = prescaledClock / baudRate;
      if (newDivisor != 0) {
        uint32_t newBaudRateError = (prescaledClock / newDivisor) - baudRate;
        if (baudRateError > newBaudRateError) {
          divisor = newDivisor;
          mulVal = mv;
          divAddVal = dav;
          baudRateError = newBaudRateError;
        }
      }
      if (allowSmallerBaudRate) {
        ++newDivisor;
        uint32_t newBaudRateError = baudRate - (prescaledClock / newDivisor);
        if (baudRateError > newBaudRateError) {
          divisor = newDivisor;
          mulVal = mv;
          divAddVal = dav;
          baudRateError = newBaudRateError;
        }
      }
    }
  }
  // uint32_t calculatedBaudRate = (frequency * mulVal) / ((16 * divisor) * (mulVal + divAddVal));
  return divisor;
}


//*****************************************
//* Initialize Serial Interface
void SerialBegin(uint8_t portNum, uint32_t baud) {

   PCONP = 0xFFFFFFFF; //everything powered on            

  ////
  // Determine slowest sufficient PCLK for this baud rate

  uint32_t pclk;
  uint8_t pclkCode;
  if (CCLK/4/16/4 >= baud) {
    pclk = CCLK/4;
    pclkCode = 0;
  } else if (CCLK/2/16/4 >= baud) {
    pclk = CCLK/2;
    pclkCode = 2;
  } else {
    pclk = CCLK;    // Yah.  Right.  This'll work.
    pclkCode = 1;
  }

  uint32_t mulVal;
  uint32_t divAddVal;
  uint32_t Fdiv;

  Fdiv = findDivisorAndPrescale(pclk, baud, true, mulVal, divAddVal);

   /* Setup the baud rate */
   uartReg[portNum]->LCR = 0x83;           /* 8 bits, no Parity, 1 Stop bit, enable access */
   uartReg[portNum]->FDR = (divAddVal<<0)|(mulVal <<4); /* Set the fractional divisor */
   uartReg[portNum]->R2.DLM = Fdiv >> 8;
   uartReg[portNum]->R1.DLL = Fdiv & 0xFF;
   uartReg[portNum]->LCR = 0x03;           /* Disable access */
   uartReg[portNum]->R3.FCR = 0x07;        /* Enable and reset TX and RX FIFO. */

	/* Setup the pin connection */
   *uartPins[portNum].TxSelReg = ( *uartPins[portNum].TxSelReg & 
                                   ~uartPins[portNum].TxMask) |
                                 uartPins[portNum].TxSelValue;
   *uartPins[portNum].RxSelReg = ( *uartPins[portNum].RxSelReg & 
                                   ~uartPins[portNum].RxMask) |
											uartPins[portNum].RxSelValue;

   /* Install the interrupt handler into the VIC */
   install_irq( uartInts[portNum].IntNumber, 
                (void *)uartInts[portNum].IntFunc,  
                uartInts[portNum].Priority);
 

   /* Enable the uart interrupts */
   uartReg[portNum]->R2.IER.RBR = 1;   /* Rx Data Available */
   uartReg[portNum]->R2.IER.THRE = 1;  /* THRE interrupt */

   /* Initialize the uart buffer */
   SerialFlush(portNum);

}

//*****************************************
//* Return the number of characters 
//* in the buffer.
int SerialAvailable(uint8_t portNum) {

	return (UART_BUFFSIZE + 
           uartData[portNum].RxHead - uartData[portNum].RxTail) % UART_BUFFSIZE;

}

//*****************************************
//* Write character to Serial Port
int SerialWrite(uint8_t portNum, int ch)  {

   while (!(uartReg[portNum]->LSR & 0x20));

   return(uartReg[portNum]->R1.THR = (unsigned char)ch);
}

//*****************************************
//* Write string to Serial Port
void SerialStr(uint8_t portNum, char * str) {
   while (*str) {
      SerialWrite(portNum, *str);
      str++;
   }
} 

//*****************************************
//* Read character from Serial Port Buffer
int  SerialRead(uint8_t portNum) {
   
   int c;
   vuint32_t *tail;

   tail = &uartData[portNum].RxTail;

	// if the head isn't ahead of the tail, we don't have any characters
	if (uartData[portNum].RxHead == *tail) {
		c = -1;
	} else {
		c = uartData[portNum].RxBuffer[*tail];
		*tail = (*tail + 1) % UART_BUFFSIZE;
	}
	
    return c;
}

//*****************************************
//* Flushes the serial buffer
void SerialFlush(uint8_t portNum)
{

   uartData[portNum].RxHead = uartData[portNum].RxTail;

}

//*****************************************
//* Reads character from Serial Port 
//* Waits in a busy loop until a character is read. 
int SerialBusyRead(uint8_t portNum) {

   while (!(uartReg[portNum]->LSR & 0x01));
   return(uartReg[portNum]->R1.RBR);

}
