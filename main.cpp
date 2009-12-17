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

extern void sysInit(void);

//************************************************
//*	Type Definitions
//************************************************

/* SFB Bootloader Config Memory in Flash */
typedef struct {
   SFBChecksum AppChecksum;
   uint64_t    AppBurnCount;
   uint32_t    AppLength;
} SFB_CONFIG_MEM_T;

//************************************************
//*	Global Variables
//************************************************

unsigned char  *kApplicationStartAddr = (unsigned char*)0x2200;

unsigned char *gAddress=0;                                          // Running butterfly address
unsigned char *gAddressPgm=(unsigned char*)kApplicationStartAddr;   // Real flash address to program
unsigned char gBuffer[kUSART_RX_BUFFER_SIZE];                       //RAM buffer for incoming data
unsigned char gDevice;
unsigned char gDevType;
char gRecvCommand;
volatile unsigned int purge;

/* Configuration Memory */
SFB_CONFIG_MEM_T * gConfigMemPtr = (SFB_CONFIG_MEM_T*)(0x2000); // Flash Location of Configuration Memory
SFB_CONFIG_MEM_T gConfigMem;                                    // RAM structure for the config


static SFBChecksum gRunning;
void (*app_code_entry)(void);

bool gSFBootValidChecksum;
uint64_t wait;
uint8_t pin;
uint8_t x;
uint8_t uart;
uint8_t index;
volatile bool released = false;
vint32_t state = -1;

//************************************************
//*	Local Function Prototypes
//************************************************
void ButterflyService(char recvCommand);
char BufferLoad(unsigned int size, unsigned char memType);
void BlockRead(unsigned int size, unsigned char memType);

/**
 * Check the input switch and possibly jump to the app.
 */
void checkSwitch(void) {

      state = digitalRead(42);
      if (state) {
         released = true;
      }

      if (released && !state && gSFBootValidChecksum) {
         /* Start the app!! */
         app_code_entry =  (void (*)(void))kApplicationStartAddr;
         app_code_entry();
      }
}


int main (void)  {


	/* System Init */
	sysInit();

   /* Make the switch pin an input */
   pinMode(42, INPUT);

   /* Read the flash burn counter */
   gConfigMem.AppChecksum[0]  = gConfigMemPtr->AppChecksum[0];
   gConfigMem.AppChecksum[1]  = gConfigMemPtr->AppChecksum[1];
   gConfigMem.AppBurnCount = gConfigMemPtr->AppBurnCount;
   gConfigMem.AppLength =   gConfigMemPtr->AppLength;

   /* Calculate the checksum on the entire user application space */
   SFBChecksumInit(gRunning);
   SFBChecksumAddBytes(gRunning,
                       (const char*)kApplicationStartAddr,
                       gConfigMem.AppLength);

   /* Read the stored Flash checksum with RAM checksum */
	gSFBootValidChecksum = SFBChecksumEqual(gConfigMem.AppChecksum, gRunning);

   /* Should we start the App? */
   if (gSFBootValidChecksum && digitalRead(42) )
   {
      /* Start the app!! */
      app_code_entry =  (void (*)(void))kApplicationStartAddr;
      app_code_entry();
   }

   /* Start the bootloader */
   SerialBegin(0,115200);
   turnOnAllLEDs();

   /* Startup effects */
   spinLEDs(2);

   /* Wait in the Bootloader */
   while(1)
   {
      /* Check the switch */
      checkSwitch();

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

         gAddress =  0;
         gAddressPgm = (unsigned char*)kApplicationStartAddr;

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

         SerialWrite(0, '\r');
      break;

      case 'E':            // Exit Upgrade
      {

         /* Increment the Burn Counter */
         gConfigMem.AppBurnCount++;
         gConfigMem.AppLength =  gAddressPgm - kApplicationStartAddr;

         /* Calculate the checksum */
         SFBChecksumInit(gConfigMem.AppChecksum);
         SFBChecksumAddBytes(gConfigMem.AppChecksum,
                             (const char*)kApplicationStartAddr,
                             gConfigMem.AppLength);

         /* Store the Configuration to Flash */
         write_flash((unsigned int*)&gConfigMemPtr->AppChecksum,
                     (unsigned char*)&gConfigMem.AppChecksum,
                      512,
                      false);

         /* Respond  */
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
         /* Note, we don't see an erase
            through AVRdude, so this hack is required. */
         erase_user_flash();
         SerialStr(0, kSoftwareIdentifier);

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

    gAddress+= byteCntDumb;
    gAddressPgm+= byteCntPgm;
}

//*******************************************************************************
// Load the RAM buffer with bytes from serial
char BufferLoad(unsigned int size, unsigned char memType)
{
   unsigned int cnt;

   /* Fill up an entire page in RAM from Serial */
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

   /* Since AVRdude sends us blank data to start off,
      check to see if we've hit the APP start address */
   if (gAddress >= (unsigned char*)(kApplicationStartAddr))
   {

      write_flash((unsigned int*)(gAddressPgm),
                  gBuffer,
                  kSPM_PAGE_SIZE,
                  true);

      /* Increment the real programming address */
      gAddressPgm+=size;

   }

   /* Increment the Butterfly address  */
   gAddress+=size;

   return '\r';
}