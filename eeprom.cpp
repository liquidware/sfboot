#include <LPC23xx.H>                       /* LPC23xx definitions  */
#include <stdint.h>
#include <stdio.h>                         /* standard I/O .h-file */

/* EEPROM Instructions */
static const uint8_t kEEREAD    = 3;
static const uint8_t kEEWRITE   = 2;  
static const uint8_t kEEWRDI    = 3;
static const uint8_t kEEWREN    = 2; 
static const uint8_t kEERDSR    = 5;
static const uint8_t kEEWRSR    = 1;
  

//*************************************************************
// EEPROM INitialization
//       Device: Microchip 25AA128 SPI EEPROM  
void eepromInit(void) {

   /* Setup pin P0.6,7,8 for SPI */
   PINSEL0 |= 0x000AA000;
   
   /* CPOL = 0, CPHA = 1, 8-bits, SPI mode */
   SSP1CR0 = 0x0087;
   
   /* SPI Master,       */
   SSP1CR1 =  0x004;
   
   /* SPI Enable,       */
   SSP1CR1 =  0x006;
}

//*************************************************************
// Write data to SSP1
void ssp1Write(uint8_t data) {


      /* Check the TX FIFO status */
      while( !((SSP1SR >> 1) & 1) ) {
         ; /* Wait for TX to be ready */
      }     
      
     /* Put the data into the TX FIFO */
      SSP1DR = data;

}
  
//*************************************************************
// Read one byte from the SSP1
uint8_t ssp1Read() {

      /* Wait for a byte to be received */
      while( !((SSP1SR >> 2) & 1) ) {
         /* Send dummy data */
         ssp1Write(0);
      }     
      
      /* Return the received byte */
      return (uint8_t)SSP1DR;

}

//*************************************************************
// EEPROM write data
// Returns: EEPROM Status Register  
// lower cs
// send wren instr
// raise cs
// lower cs
// send write instr
// send 16-bit address
// send data up 64
//  increment page?
//  continue writing
//  raise cs
int eepromWrite(uint16_t dataEEDest, uint8_t * const dataSrc, uint32_t length) {

uint32_t ii;

   /* Send Write Enable */      
   ssp1Write(kEEWREN);

   /* Send Write Enable Instruction */      
   ssp1Write(kEEWREN);

   /* Send Write Instruction */      
   ssp1Write(kEEWRITE);

   /* Send Address High Byte */      
   ssp1Write((dataEEDest >> 8) & 0xFF);

   /* Send Address LOW Byte */      
   ssp1Write(dataEEDest & 0xFF);

   for (ii=0; ii < length; ii++ ) {

      ssp1Write(dataSrc[ii]);
   }

 return 1;
}
 
//*************************************************************
// EEPROM read data
// Returns: EEPROM Status Register   
    // lower cs
    // send read instr
    // 16-bit address
    // clock 8*len
    // raise cs
    //      
int eepromRead(uint8_t * dataDest, uint16_t dataEESrc, uint32_t length) {


uint32_t ii;

   /* Send Read Instruction */      
   ssp1Write(kEEREAD);

   /* Send Address High Byte */      
   ssp1Write((dataEESrc >> 8) & 0xFF);

   /* Send Address LOW Byte */      
   ssp1Write(dataEESrc & 0xFF);
   
   /* Store the bytes from EEPROM */   
   for (ii=0; ii< length; ii++) {      
      dataDest[ii] = ssp1Read();
   }

   return 1;
} 