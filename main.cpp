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

#include <LPC23xx.H>                       /* LPC23xx definitions  */
#include <stdint.h>
#include <stdio.h>                         /* standard I/O .h-file */
#include <math.h>
#include <stdlib.h>


#include "fio.h"
#include "wiring.h"
#include "Serial.h"
#include "sbl_iap.h"
#include "sbl_config.h"
#include "SFBChecksum.h"
#include "eeprom.h"
#include "sfboard_tests.h"

#define SWITCH_FIOPIN   FIO1PIN
#define SWITCH_MASK     (1<<29)

#define kMCUId "lpc2368"
//#define kApplicationStartAddr  USER_FLASH_START    // from sbl_config.h
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
//extern "C" int __aeabi_atexit(void *object,
//							void (*destructor)(void *),
//							void *dso_handle)
//							{
//							return 0;
//							}

extern void sysInit(void);

/* SFB Bootloader Config Memory in Flash */ 
typedef struct {
   SFBChecksum AppChecksum;
   uint8_t     SpareConfigData[24];
   uint8_t     AppData[kSPM_PAGE_SIZE];
} SFB_CONFIG_MEM_T;

//************************************************
//*	Global Variables 
//************************************************
 
unsigned char  kConfigMemLength = 0x100;
unsigned char  *kApplicationStartAddr = (unsigned char*)0x2020;

unsigned char *gAddress=0; // butterfly address 
unsigned char *gAddressPgm=(unsigned char*)kApplicationStartAddr;   //address to program
unsigned char gBuffer[kUSART_RX_BUFFER_SIZE];           //RAM buffer for incoming data
SFB_CONFIG_MEM_T gConfigMem;                            // RAM structure for the config + first page of app

unsigned char gDevice;
unsigned char gDevType;
char gRecvCommand;
volatile unsigned int purge;

static SFBChecksum gRunning;
const  SFBChecksum *gConfigCs = (SFBChecksum *)(0x2400);        //1 sector behind the user app
bool   gSFBootValidChecksum   = false;
void (*app_code_entry)(void);

//************************************************
//*	Local Function Prototypes
//************************************************
void ButterflyService(char recvCommand);
char BufferLoad(unsigned int size, unsigned char memType);
void BlockRead(unsigned int size, unsigned char memType);

uint64_t wait;
uint8_t pin;
uint8_t x;
uint8_t uart;
uint8_t index;
bool gGotFirstPage = false;

int main (void)  {

	/* System Init */
	sysInit();

   for (int x=0; x<kPinCount; x++) {
      pinMode(x, OUTPUT);
   }

   turnOnAllLEDs();

   /* Startup effects, don't be annoying here */
   spinLEDs(2);  

   SerialBegin(0,57600);

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

   while(1)
   {
      
      /* Check for an app start */ 
      if (gSFBootValidChecksum && (SWITCH_FIOPIN & SWITCH_MASK))
      {
         /* Start the app!! */
         app_code_entry =  (void (*)(void))kApplicationStartAddr;
         app_code_entry();
      }

      /* Check for an incoming byte to be read from UART 0 */
      if (U0LSR & 0x01)
      {
         /* Get the command character */
         gRecvCommand = SerialBusyRead(0);
   
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
         SerialWrite(0, 'Y');
      break;

      case 'A':            // Write address
         purge  = ( SerialBusyRead(0) << 8);    // High byte of address
         purge |=   SerialBusyRead(0);          // low byte of address
         
         purge <<= 2;                      // !! convert from word address to byte address

         SerialWrite(0, '\r');
      break;

       case 'H':           // Write address
       {
       

        purge |= ( SerialBusyRead(0) << 16);    // High byte of address
        purge |= ( SerialBusyRead(0) << 8);    // 2nd byte of address
        purge |=   SerialBusyRead(0);          // low byte of address
        
 //       if(purge == 0)
 //       {
         gAddress =  0;
         gAddressPgm = (unsigned char*)kApplicationStartAddr;
  //      }

         SerialWrite(0, '\r');
         }
      break;

      case 'b':            // Buffer load support
         SerialWrite(0, 'Y');                          // Report buffer load supported
         SerialWrite(0, (kUSART_RX_BUFFER_SIZE >>8));    // Report buffer size in bytes
         SerialWrite(0, kUSART_RX_BUFFER_SIZE);          // low byte of size

         SerialWrite(0, '\r');
      break;

      case 'B':            // Start Buffer load
      {
         unsigned int bufferSize;
         unsigned char memType;
         unsigned char res;

         bufferSize = SerialBusyRead(0) << 8;     // Load high byte of buffersize
         bufferSize |= SerialBusyRead(0);          // Load low byte of buffersize
         memType = SerialBusyRead(0);            // Load memory type ('E' or 'F')

         res = BufferLoad(bufferSize, memType);

         SerialWrite(0,res);    //return the result
                     //SerialWrite('\r');
      }
      break;

      case 'g':            // Block readout
      {
         unsigned int bufferSize;
         unsigned char memType;

         bufferSize = SerialBusyRead(0) << 8;            // Load high byte of buffersize
         bufferSize |= SerialBusyRead(0);                 // Load low byte of buffersize
         memType = SerialBusyRead(0);            // Load memory type ('E' or 'F')         

        if (bufferSize == 1)
        {         
            SerialWrite(0,'\r');
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

         SerialWrite(0, '\r');
      break;

      case 'E':            // Exit Upgrade
      {

         /* Calculate the checksum */
         SFBChecksumInit(gRunning);
         SFBChecksumAddBytes(gRunning,
                            (const char*)kApplicationStartAddr,
                            (kApplicationEndAddr - (int)kApplicationStartAddr));
         
         /* Store the Checksum to RAM */
         gConfigMem.AppChecksum[0] = gRunning[0];
         gConfigMem.AppChecksum[1] = gRunning[1];

         /* Store the Configuration to Flash */
         write_flash((unsigned int*)(kApplicationStartAddr - kConfigMemLength),
                     (unsigned char*)&gConfigMem,
                     kConfigMemLength+kSPM_PAGE_SIZE);

         SerialWrite(0, '\r');
         SerialWrite(0, '\r');
          
         app_code_entry =  (void (*)(void))kApplicationStartAddr;
         app_code_entry();


      }
      break;

      case 'P':            // Enter programming mode
         SerialWrite(0, '\r');
      break;

      case 'F':            // fuse
         //execute_user_code();
         SerialWrite(0, '\r');
      break;

      case 'r':            // fuse
         SerialWrite(0, '\r');
      break;

      case 'N':            // fuse
         SerialWrite(0, '\r');
      break;

      case 'Q':            // fuse
         SerialWrite(0, '\r');
      break;

      case 'L':            // Leave programming mode
         SerialWrite(0, '\r');
      break;

      case 'p':            // return programmer type
         SerialWrite(0, 'S');
      break;

      case 't':           // return device type
         SerialWrite(0, kDevType);
         SerialWrite(0, 0);
      break;

      case 'T':           // Set device
         gDevice = SerialBusyRead(0);
         SerialWrite(0, '\r');
      break;

      case 'S':            //return software identifier
         {
         //volatile long xx;

         //unsigned int res=0;

         erase_user_flash();

          SerialStr(0, kSoftwareIdentifier);
          
         
            //for(xx=0;xx<20000000;xx++) {;}         
         //for(res=0; res<512; res++)
         //{
        // gBuffer[res] = res;
         
        // }
        // write_flash((unsigned int*)gAddress, gBuffer, 512);
         }
      break;

      case 'V':            //return software version
          SerialWrite(0, kSoftwareVersionHigh);
          SerialWrite(0, kSoftwareVersionLow);
      break;

      case 's':            // Return Signature byte
         SerialWrite(0, kSignatureByte1);
         SerialWrite(0, kSignatureByte2);
         SerialWrite(0, kSignatureByte3);
      break;

      default:
         SerialStr(0, "?");
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

           SerialWrite(0, byte);
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
            gBuffer[cnt] = SerialBusyRead(0);
        }
        else
        {     
           gBuffer[cnt]=0xFF;
        }
    }

/* Since AVRdude sends us blank data, 
   check to see if we've hit the APP start address */
   if (gAddress >= (unsigned char*)(kApplicationStartAddr))
   {
   
    //  if (gGotFirstPage) {
         /* Normally Burn the data */
         write_flash((unsigned int*)(gAddressPgm), gBuffer, kSPM_PAGE_SIZE);
   
    //  } else {
         /* Copy the data into RAM */
     //    for(int x=0; x< kSPM_PAGE_SIZE; x++) {
     //       gConfigMem.AppData[x] = gBuffer[x];
      //   }
   
      gAddressPgm+=size;
   
      /* Latch this on */
      //gGotFirstPage = true;
   }

   gAddress+=size;
   
   
   return '\r';
}