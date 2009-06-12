#ifndef WIRING_PINS_H
#define WIRING_PINS_H

#define kPinCount 43 /* The total number of pins in the PinTable */

typedef enum {
   eRXTX,
   eDIGITAL,
   eANALOG,
   eNumPinTypes,
} PIN_TYPE_T; 

typedef enum {
   eP0,
   eP1,
   eP2,
   eP3,
   eP4,
} PORTID_T;

typedef struct {
   uint8_t     PinId;
   PORTID_T    PortNum;
   uint8_t     PortPin;
   PIN_TYPE_T  PinIsA;
} PIN_DESC_T;

extern PIN_DESC_T PinTable[];


#endif