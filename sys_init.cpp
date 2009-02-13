#include <stdint.h>

#include "LPC_REGS.h"
#include "sys_config.h"

/******************************************************************************
 *
 * Function Name: configPLL()
 *
 * Description:
 *    This function starts up the PLL then sets up the GPIO pins before
 *    waiting for the PLL to lock.  It finally engages the PLL and
 *    returns
 *
 * Calling Sequence: 
 *    void
 *
 * Returns:
 *    void
 * partily based on code for NXP example collection
 *****************************************************************************/
static void configPLL(void)
{
	uint32_t readback;

	// check if PLL connected, disconnect if yes
	if ( PLLSTAT & PLLSTAT_PLLC ) {
		PLLCON = PLLCON_PLLE;       /* Enable PLL, disconnected ( PLLC = 0 )*/
		PLLFEED = 0xaa;
		PLLFEED = 0x55;
	}

	PLLCON  = 0;        /* Disable PLL, disconnected */
	PLLFEED = 0xaa;
	PLLFEED = 0x55;

	SCS |= SCS_OSCEN;   /* Enable main OSC, SCS Man p.28 */
	while( !( SCS & SCS_OSCSTAT ) ) {
		;	/* Wait until main OSC is usable */
	}

	CLKSRCSEL = CLKSRC_MAIN_OSC;   /* select main OSC as the PLL clock source */

	PLLCFG = PLLCFG_MSEL | PLLCFG_NSEL;
	PLLFEED = 0xaa;
	PLLFEED = 0x55;
	
	PLLCON = PLLCON_PLLE;       /* Enable PLL, disconnected ( PLLC = 0 ) */
	PLLFEED = 0xaa;
	PLLFEED = 0x55;
	
	CCLKCFG = CCLKCFG_CCLKSEL_VAL;     /* Set clock divider, Manual p.45 */

//#if USE_USB
//	USBCLKCFG = USBCLKDivValue; /* usbclk = 288 MHz/6 = 48 MHz */
//#endif
	
	while ( ( PLLSTAT & PLLSTAT_PLOCK ) == 0 )  {
		; /* Check lock bit status */
	}
	
	readback = PLLSTAT & 0x00FF7FFF;
	while ( readback != (PLLCFG_MSEL | PLLCFG_NSEL) )
	{
		; // stall - invalid readback
	}
	
	PLLCON = ( PLLCON_PLLE | PLLCON_PLLC );  /* enable and connect */
	PLLFEED = 0xaa;
	PLLFEED = 0x55;
	while ( ((PLLSTAT & PLLSTAT_PLLC) == 0) ) {
		;  /* Check connect bit status, wait for connect */
	}
}

static void lowInit(void)
{
	configPLL();
	
	// setup & enable the MAM
	MAMCR = MAMCR_OFF;
#if CCLK < 20000000
	MAMTIM = 1;
#elif CCLK < 40000000
	MAMTIM = 2;
#else
	MAMTIM = 3;
#endif
	MAMCR = MAMCR_FULL;
	
	// set the peripheral bus speed
	// value computed from config.h
	
#if PBSD == 4
	PCLKSEL0 = 0x00000000;	/* PCLK is 1/4 CCLK */
	PCLKSEL1 = 0x00000000;
#elif PBSD == 2
	PCLKSEL0 = 0xAAAAAAAA;	/* PCLK is 1/2 CCLK */
	PCLKSEL1 = 0xAAAAAAAA;
#elif PBSD == 1
	PCLKSEL0 = 0x55555555;	/* PCLK is the same as CCLK */
	PCLKSEL1 = 0x55555555;
#else
#error invalid p-clock divider    
#endif

	return;
}

/******************************************************************************
 *
 * Function Name: sysInit()
 *
 * Description:
 *    This function is responsible for initializing the program
 *    specific hardware
 *
 * Calling Sequence: 
 *    void
 *
 * Returns:
 *    void
 *
 *****************************************************************************/
void sysInit(void)
{
	uint32_t i = 0;
	volatile uint32_t *vect_addr, *vect_prio;

	lowInit();                            // setup clocks and processor port pins

	// set the interrupt controller defaults
#if defined(RAM_RUN)
	MEMMAP = MEMMAP_SRAM;                 // map interrupt vectors space into SRAM
#elif defined(ROM_RUN)
	MEMMAP = MEMMAP_FLASH;                // map interrupt vectors space into FLASH
#else
#error RUN_MODE not defined!
#endif
	
	/* initialize VIC */
	VICIntEnClr  = 0xffffffff;
	VICVectAddr  = 0x00000000;
	VICIntSelect = 0x00000000; /* all IRQ */
	
	/* set all the vector and vector control register to 0 */
	for ( i = 0; i < 32; i++ ) {
		vect_addr = (uint32_t *)(VIC_BASE_ADDR + VECT_ADDR_INDEX + i*4);
		vect_prio = (uint32_t *)(VIC_BASE_ADDR + VECT_PRIO_INDEX + i*4);
		*vect_addr = 0x00000000;
		*vect_prio = 0x0000000F;
	}

	// non-existing on LPC23xx/24xx:  VICDefVectAddr = (uint32_t)reset;     // point unvectored IRQs to reset()
	
	//  wdtInit();                         // initialize the watchdog timer
//	initSysTime();                        // initialize the system timer

#if (UART0_SUPPORT)
//	uart0Init(UART_BAUD(HOST_BAUD_U0), UART_8N1, UART_FIFO_8); // setup the UART
#endif
#if (UART1_SUPPORT)
//	uart1Init(UART_BAUD(HOST_BAUD_U1), UART_8N1, UART_FIFO_8); // setup the UART
#endif

	SCS |= (1UL<<0); // set GPIOM in SCS for fast IO
}