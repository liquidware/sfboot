/******************************************************************************/
/* IAP In-System Application Programming Demo                                 */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

//************************************************
//*	Preprocessor Directives
//************************************************

#include <stdio.h>                         /* standard I/O .h-file */
#include <LPC23xx.H>                       /* LPC23xx definitions  */
#include <math.h>
#include <stdlib.h>
#include <stdint.h>

#include "fio.h"
#include "Serial.h"
#include "sbl_iap.h"
#include "sbl_config.h"
#include "SFBChecksum.h"

#define LED_FIOPIN  FIO2PIN
#define LED_FIOSET  FIO2SET
#define LED_FIOCLR  FIO2CLR

#define SWITCH_FIOPIN   FIO1PIN

#define LED_GREEN_MASK    (1UL<<0)
#define LED_RED_MASK      (1UL<<1)
#define LED_BLUE_MASK     (1UL<<2)

#define LED_SOUTH_MASK (1UL<<3)
#define LED_WEST_MASK (1UL<<4)
#define LED_NORTH_MASK  (1UL<<5)
#define LED_EAST_MASK  (1UL<<26)
#define SWITCH_MASK    (1UL<<22)

#define kMCUId "lpc2368"
//#define kApplicationStartAddr  USER_FLASH_START    // from sbl_config.h
#define kSFBootConfigSize	   0x1000 				//size of the config sector
#define kApplicationEndAddr    USER_FLASH_END      // from sbl_config.h
#define kSPM_PAGE_SIZE         512
#define kUSART_RX_BUFFER_SIZE  kSPM_PAGE_SIZE               
#define kSoftwareIdentifier     "AVRBOOT"
#define kSoftwareVersionHigh   '0'
#define kSoftwareVersionLow    '7'

#define kDevType               0x74         //mega645
#define kSignatureByte3        0x1E         //mega2560
#define kSignatureByte2        0x98         //mega2560
#define kSignatureByte1        0x01         //mega2560      

//************************************************
//*	Provide our own (dead) libc library functions  
//************************************************
extern "C" int _vfprintf_r(struct _reent *, FILE*, const char*, __gnuc_va_list) {return 0;} 
extern "C" void *_malloc_r(struct _reent *, size_t) {return (void *)0; }
extern "C" void _free_r(struct _reent *, void*) {}
extern "C" void *_realloc_r(struct _reent *, void*, size_t) {return (void *)0; };
extern "C" void *realloc(void *, size_t) {return (void *)0; };
extern "C" void *_calloc_r(struct _reent *, size_t, size_t) {return (void *)0; };

extern "C" void *malloc(size_t) { return (void *)0; }
extern "C" void  free(void*) {  }
extern "C" int __aeabi_atexit(void *object,
							void (*destructor)(void *),
							void *dso_handle)
							{
							return 0;
							}


extern void sysInit(void);

//************************************************
//*	Global Variables 
//************************************************
unsigned char *kApplicationStartAddr = (unsigned char*)0x1000;//(unsigned char *)(USER_FLASH_START + 
													       //	 kSFBootConfigSize); // the user application
unsigned char *gAddress=0; // butterfly address 
unsigned char *gAddressPgm=(unsigned char*)kApplicationStartAddr;   //address to program
unsigned char gBuffer[kUSART_RX_BUFFER_SIZE];  //RAM buffer for incoming data 
unsigned char gDevice;
unsigned char gDevType;
char gRecvCommand;
volatile unsigned int purge;

static SFBChecksum gRunning;
bool gSFBootValidChecksum = false;
const SFBChecksum *gConfigCs = (SFBChecksum *)(0x1000);        //1 sector behind the user app
void (*app_code_entry)(void);

//************************************************
//*	Local Function Prototypes
//************************************************
void ButterflyService(char recvCommand);
char BufferLoad(unsigned int size, unsigned char memType);
void BlockRead(unsigned int size, unsigned char memType);
void LEDspinPattern(unsigned int spinCount);

//**********************************************
//* Lights up the outside LEDs in 
//* a rotational pattern.
//*
//* spinCount = the number of times to spin 
void LEDSpinPattern(unsigned int spinCount) {

   unsigned int face=0;
   volatile unsigned int wait;

   /* Set the I/O direction */
   GPIOInit( 2, FAST_PORT, DIR_OUT, LED_RED_MASK );
   GPIOInit( 2, FAST_PORT, DIR_OUT, LED_GREEN_MASK );
   GPIOInit( 2, FAST_PORT, DIR_OUT, LED_BLUE_MASK );

   GPIOInit( 1, FAST_PORT, DIR_OUT, LED_EAST_MASK );
   GPIOInit( 2, FAST_PORT, DIR_OUT, LED_SOUTH_MASK );
   GPIOInit( 2, FAST_PORT, DIR_OUT, LED_WEST_MASK );
   GPIOInit( 2, FAST_PORT, DIR_OUT, LED_NORTH_MASK );

   /*Setup the initial state */ 

   /* Turn off the RGB LEDs */
   FIO2SET |= LED_RED_MASK;
   FIO2SET |= LED_GREEN_MASK;
   FIO2SET |= LED_BLUE_MASK;

   FIO2SET |= LED_NORTH_MASK;
   FIO2SET |= LED_SOUTH_MASK;
   FIO2SET |= LED_WEST_MASK;
   FIO1SET |= LED_EAST_MASK;
   
   spinCount*=4;
   
   /* Spin! */
   while (spinCount--) {

      /* Custom tuned for minimal epilepsy
	  	 Busy wait...  */
      
      /* Change the light pattern */
      switch (face) {

      case 0:
         FIO2SET |= LED_NORTH_MASK;
         FIO2SET |= LED_SOUTH_MASK;
         FIO2SET |= LED_WEST_MASK;
         FIO1CLR |= LED_EAST_MASK;
         break;

      case 1:
         FIO2SET |= LED_NORTH_MASK;
         FIO2CLR |= LED_SOUTH_MASK;
         FIO2SET |= LED_WEST_MASK;
         FIO1SET |= LED_EAST_MASK;
         break;

      case 2:
         FIO2SET |= LED_NORTH_MASK;
         FIO2SET |= LED_SOUTH_MASK;
         FIO2CLR |= LED_WEST_MASK;
         FIO1SET |= LED_EAST_MASK;
         break;

      case 3:
         FIO2CLR |= LED_NORTH_MASK;
         FIO2SET |= LED_SOUTH_MASK;
         FIO2SET |= LED_WEST_MASK;
         FIO1SET |= LED_EAST_MASK;
         break;
      }

      for (wait=0;wait<600000; wait++) {
         ;
      }

      /* Move to the next led */
      face++;
      if (face>=4) {
         face=0;
      }
   }  
}

 

int main (void)  {
	 
	/* System Init */
	sysInit();
   SerialBegin(57600); 

   /* Startup effects, don't be annoying here */
   LEDSpinPattern(2);  

#if 0
   /* Calculate the checksum on the entire user application space */
   SFBChecksumInit(gRunning);
   SFBChecksumAddBytes(gRunning,
                       (const char*)kApplicationStartAddr,
                       (kApplicationEndAddr - (int)kApplicationStartAddr));

    /* Compare stored Flash checksum with RAM checksum */
	if (SFBChecksumEqual(*gConfigCs, gRunning))
	{
		/* Got a valid checksum */
		gSFBootValidChecksum = true;
	}
#endif

   while(1)
   {
      
      /* Check for a app start request */ 
     if ( !(SWITCH_FIOPIN & SWITCH_MASK))
      {
         /* Start the app!! */
         app_code_entry =  (void (*)(void))kApplicationStartAddr;
         app_code_entry();
      }

      /* Check for an incoming byte to be read from UART 0 */
      if (U0LSR & 0x01)
      {
         /* Get the command character */
         gRecvCommand = SerialRead();
   
         /* Handle the butterfly command */
         ButterflyService(gRecvCommand);
      }
   }

}


//*******************************************************************************
// Loosely based on the AVR buttery protocol.
// Handles the received commands
void ButterflyService(char recvCommand)
{
   
   /* Handle Command Character */
   switch(recvCommand)
   {
   
      case 'a':            // Autoincrement?
         SerialWrite('Y');
      break;

      case 'A':            // Write address
         purge  = ( SerialRead() << 8);    // High byte of address
         purge |=   SerialRead();          // low byte of address
         
         purge <<= 2;                      // !! convert from word address to byte address

         SerialWrite('\r');
      break;

       case 'H':           // Write address
       {
       

        purge |= ( SerialRead() << 16);    // High byte of address
        purge |= ( SerialRead() << 8);    // 2nd byte of address
        purge |=   SerialRead();          // low byte of address
        
 //       if(purge == 0)
 //       {
         gAddress =  0;
         gAddressPgm = (unsigned char*)kApplicationStartAddr;
  //      }

         SerialWrite('\r');
         }
      break;

      case 'b':            // Buffer load support
         SerialWrite('Y');                          // Report buffer load supported
         SerialWrite((kUSART_RX_BUFFER_SIZE >>8));    // Report buffer size in bytes
         SerialWrite(kUSART_RX_BUFFER_SIZE);          // low byte of size

         SerialWrite('\r');
      break;

      case 'B':            // Start Buffer load
      {
         unsigned int bufferSize;
         unsigned char memType;
         unsigned char res;

         bufferSize = SerialRead() << 8;     // Load high byte of buffersize
         bufferSize |= SerialRead();          // Load low byte of buffersize
         memType = SerialRead();            // Load memory type ('E' or 'F')

         res = BufferLoad(bufferSize, memType);

         SerialWrite(res);    //return the result
                     //SerialWrite('\r');
      }
      break;

      case 'g':            // Block readout
      {
         unsigned int bufferSize;
         unsigned char memType;

         bufferSize = SerialRead() << 8;            // Load high byte of buffersize
         bufferSize |= SerialRead();                 // Load low byte of buffersize
         memType = SerialRead();            // Load memory type ('E' or 'F')         

        if (bufferSize == 1)
        {         
            SerialWrite('\r');
        }
        else
        {
            BlockRead(bufferSize, memType);
        }

        break;
      }
      case 'e':            // Chip Erase 
         erase_user_flash();
         //erase(kApplicationStartAddr,kApplicationEndAddr);

         SerialWrite('\r');
      break;

      case 'E':            // Exit Upgrade
      {
   //         unsigned int x;

#if 0
            /* Write out the checksum */
            SFBChecksumInit(gRunning);
            SFBChecksumAddBytes(gRunning,
                       (const char*)kApplicationStartAddr,
                       (kApplicationEndAddr - (int)kApplicationStartAddr));
 //           unsigned char *dataPtr = ;

            /* Copy the checksum bytes */
 //           for (=0; x< sizeof(gRunning); x++)
 //              gBuffer[x] = dataPtr[x];
            
            write_flash((unsigned int*)(gConfigCs), (unsigned char*)&gRunning, kSPM_PAGE_SIZE);
#endif

         SerialWrite('\r');
         SerialWrite('\r');
          
         app_code_entry =  (void (*)(void))kApplicationStartAddr;
         app_code_entry();


      }
      break;

      case 'P':            // Enter programming mode
         SerialWrite('\r');
      break;

      case 'F':            // fuse
         //execute_user_code();
         SerialWrite('\r');
      break;

      case 'r':            // fuse
         SerialWrite('\r');
      break;

      case 'N':            // fuse
         SerialWrite('\r');
      break;

      case 'Q':            // fuse
         SerialWrite('\r');
      break;

      case 'L':            // Leave programming mode
         SerialWrite('\r');
      break;

      case 'p':            // return programmer type
         SerialWrite('S');
      break;

      case 't':           // return device type
         SerialWrite(kDevType);
         SerialWrite(0);
      break;

      case 'T':           // Set device
         gDevice = SerialRead();
         SerialWrite('\r');
      break;

      case 'S':            //return software identifier
         {
         //volatile long xx;

         //unsigned int res=0;

         erase_user_flash();

          SerialStr(kSoftwareIdentifier);
          
         
            //for(xx=0;xx<20000000;xx++) {;}         
         //for(res=0; res<512; res++)
         //{
        // gBuffer[res] = res;
         
        // }
        // write_flash((unsigned int*)gAddress, gBuffer, 512);
         }
      break;

      case 'V':            //return software version
          SerialWrite(kSoftwareVersionHigh);
          SerialWrite(kSoftwareVersionLow);
      break;

      case 's':            // Return Signature byte
         SerialWrite(kSignatureByte1);
         SerialWrite(kSignatureByte2);
         SerialWrite(kSignatureByte3);
      break;

      default:
         SerialStr("?");
      break;
   }

}

//*******************************************************************************
// Send a block of program memory via serial
void BlockRead(unsigned int size, unsigned char memType)
{
    unsigned int cnt=0;

//    if (memType == 'F')
//    {
//volatile unsigned int sz = size;
unsigned int byteCntPgm=0;
unsigned int byteCntDumb=0;

if (gAddressPgm >= (unsigned char*)0x7800)
{
byteCntPgm=0;
}

        for(cnt = 0; cnt< size; cnt++)
        {
           unsigned char *b = (unsigned char*)gAddressPgm;
           unsigned char byte;

           /* test for in app range */
          if (gAddress >= (unsigned char*)(kApplicationStartAddr))
           { 
            byte = b[cnt];
            byteCntPgm++;
           }
           else
           {
            /* Send dummy data */
            byte = 0xFF;
            byteCntDumb++;
           }

           SerialWrite(byte);
        }
//    }

    gAddress+= byteCntDumb;
    gAddressPgm+= byteCntPgm;
}

//*******************************************************************************
// Load the RAM buffer with bytes from serial
// !!!need to fix
char BufferLoad(unsigned int size, unsigned char memType)
{
   unsigned int cnt;

/* Fill up an entire page */
    for (cnt=0; cnt < kSPM_PAGE_SIZE; cnt++) 
    {
        if (cnt<size)
        {    
            gBuffer[cnt] = SerialRead();
        }
        else
        {     
           gBuffer[cnt]=0xFF;
        }
    }
   

if (gAddress == (unsigned char*)0x7A00)
{
gAddress = gAddress;
}

if (gAddress >= (unsigned char*)(kApplicationStartAddr))
{
           write_flash((unsigned int*)(gAddressPgm), gBuffer, kSPM_PAGE_SIZE);
           gAddressPgm+=size;
}

gAddress+=size;
//       }
//   }

   return '\r';
}