#include <stdint.h>
#include "wiring_pins.h"

PIN_DESC_T PinTable[] = {
/* North */ 
           /*PinID,   PortNum,   PortPin,   PinIsA */ 
               {0,      eP0,       3,       eRXTX },     /* NorthRX */ 
               {1,      eP0,       2,       eRXTX },     /* NorthTX */
               {2,      eP1,       0,       eDIGITAL },  /* NorthGP1 */
               {3,      eP1,       1,       eDIGITAL },  /* NorthGP2 */
               {4,      eP4,       29,      eDIGITAL },  /* NorthGP3 */
               {5,      eP4,       28,      eDIGITAL },  /* NorthGP4 */
               {6,      eP1,       16,      eDIGITAL },  /* NorthVinEnb  */
               {7,      eP1,       17,      eDIGITAL },  /* NorthVoutEnb */
/* South */ 
           /*PinID,   PortNum,   PortPin,   PinIsA */ 
               {8,      eP0,       16,      eRXTX },     /* SouthRX */ 
               {9,      eP0,       15,      eRXTX },     /* SouthTX */
               {10,     eP0,       17,      eDIGITAL },  /* SouthGP1 */
               {11,     eP0,       18,      eDIGITAL },  /* SouthGP2 */
               {12,     eP0,       20,      eDIGITAL },  /* SouthGP3 */
               {13,     eP0,       19,      eDIGITAL },  /* SouthGP4 */
               {14,     eP1,       14,      eDIGITAL },  /* SouthVinEnb  */
               {15,     eP1,       15,      eDIGITAL },  /* SouthVoutEnb */
/* East */ 
           /*PinID,   PortNum,   PortPin,   PinIsA */ 
               {16,     eP0,       11,      eRXTX },     /* EastRX */ 
               {17,     eP0,       10,      eRXTX },     /* EastTX */
               {18,     eP2,       13,      eDIGITAL },  /* EastGP1 */
               {19,     eP1,       24,      eDIGITAL },  /* EastGP2 */
               {20,     eP1,       25,      eDIGITAL },  /* EastGP3 */
               {21,     eP1,       26,      eDIGITAL },  /* EastGP4 */
               {22,     eP1,       9,       eDIGITAL },  /* EastVinEnb  */
               {23,     eP1,       10,      eDIGITAL },  /* EastVoutEnb */
/* West */ 
           /*PinID,   PortNum,   PortPin,   PinIsA */ 
               {24,     eP0,       1,       eRXTX },     /* WestRX */ 
               {25,     eP0,       0,       eRXTX },     /* WestTX */
               {26,     eP1,       20,      eDIGITAL },  /* WestGP1 */
               {27,     eP1,       19,      eDIGITAL },  /* WestGP2 */
               {28,     eP1,       23,      eDIGITAL },  /* WestGP3 */
               {29,     eP1,       21,      eDIGITAL },  /* WestGP4 */
               {30,     eP1,       4,       eDIGITAL },  /* WestVinEnb  */
               {31,     eP1,       8,       eDIGITAL },  /* WestVoutEnb */
/* LED */
               {32,     eP2,       0,       eDIGITAL },  /* RGB */
               {33,     eP2,       1,       eDIGITAL },  /* RGB */
               {34,     eP2,       2,       eDIGITAL },  /* RGB */

               {35,     eP2,       3,       eDIGITAL },  /* LED */
               {36,     eP2,       4,       eDIGITAL },  /* LED */
               {37,     eP2,       5,       eDIGITAL },  /* LED */
               {38,     eP2,       12,       eDIGITAL }, /* LED */
/* MUX */
               {39,     eP3,       26,       eDIGITAL }, /* MUXA */
               {40,     eP3,       25,       eDIGITAL }, /* MUXB */
               {41,     eP0,       21,       eDIGITAL }, /* MUXC */
/* Switch */
               {42,     eP1,       29,       eDIGITAL }, /* Switch1 */

/* ADC */
               {43,     eP0,       23,       eDIGITAL }, /* ADC0 */
               {44,     eP0,       24,       eDIGITAL }, /* ADC1 */
               {45,     eP0,       25,       eDIGITAL }, /* ADC2 */
               {46,     eP0,       26,       eDIGITAL }, /* ADC3 */
               {47,     eP1,       30,       eDIGITAL }, /* ADC4 */
               {48,     eP1,       31,       eDIGITAL }, /* ADC5 */

};