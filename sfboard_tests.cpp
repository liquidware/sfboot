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
#include <stdint.h>
#include "wiring.h"
#include "sfboard_tests.h"

//****************************
//* Turns on all LEDs
void turnOnAllLEDs() {

   for (int l=0; l<7; l++) {
      pinMode(l+32, OUTPUT);
      digitalWrite(l+32, LOW);
   }

}

//***************************************
//* Toggles all the pins on the board.
//* Each pin is toggled a different number of times
//* See Pin mapping
void wiggleAllPins() {
uint8_t pin;

   /* All as output */
   for (int x=0; x<kPinCount; x++) {
      pinMode(x, OUTPUT);
   }

   /* Pin Toggling */
   for (pin=0; pin < kPinCount; pin++) {
      for (int x=0; x<pin+1; x++) {
         digitalWrite(pin, LOW);
         digitalWrite(pin, HIGH);
         digitalWrite(pin, LOW);
      }
   }
}

//****************************************
//* Toggles the Outbound power 
//* on all four faces
void toggleOutboundPower() {
volatile long wait;

   pinMode(7, OUTPUT);
   pinMode(15, OUTPUT);
   pinMode(23, OUTPUT);
   pinMode(31, OUTPUT);
         
   digitalWrite(7, LOW);
   digitalWrite(15, LOW);
   digitalWrite(23, LOW);
   digitalWrite(31, LOW);
   
   for (wait=0;wait<600000; wait++) {
     ;
   }
   
    digitalWrite(7, HIGH);
    digitalWrite(15, HIGH);
    digitalWrite(23, HIGH);
    digitalWrite(31, HIGH);
   
   for (wait=0;wait<600000; wait++) {
     ;
   }
}


//**********************************************
//* Lights up the outside LEDs in 
//* a rotational pattern.
//*
//* spinCount = the number of times to spin 
void spinLEDs(unsigned int spinCount) {

   unsigned int face=0;
   volatile unsigned int wait;

   /* Turn off the RGB LEDs */
   digitalWrite(35, HIGH);
   digitalWrite(36, HIGH);
   digitalWrite(37, HIGH);
   digitalWrite(38, HIGH);
   
   spinCount*=4;
   
   /* Spin! */
   while (spinCount--) {

      /* Custom tuned for minimal epilepsy ;-) */
      
      /* Change the light pattern */
      switch (face) {

      case 0:
         digitalWrite(35, HIGH);
         digitalWrite(36, HIGH);
         digitalWrite(37, LOW);
         digitalWrite(38, HIGH);
         break;

      case 1:
         digitalWrite(35, HIGH);
         digitalWrite(36, LOW);
         digitalWrite(37, HIGH);
         digitalWrite(38, HIGH);
         break;

      case 2:
         digitalWrite(35, HIGH);
         digitalWrite(36, HIGH);
         digitalWrite(37, HIGH);
         digitalWrite(38, LOW); 
         break;

      case 3:
         digitalWrite(35, LOW);
         digitalWrite(36, HIGH);
         digitalWrite(37, HIGH);
         digitalWrite(38, HIGH);
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