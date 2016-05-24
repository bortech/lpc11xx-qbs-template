/******************************************************************************
 * @file:    system_LPC11xx.c
 * @purpose: CMSIS Cortex-M0 Device Peripheral Access Layer Source File
 *           for the NXP LPC11xx Device Series 
 * @version: V1.0
 * @date:    26. Nov. 2008
 *----------------------------------------------------------------------------
 *
 * Copyright (C) 2008 ARM Limited. All rights reserved.
 *
 * ARM Limited (ARM) is supplying this software for use with Cortex-M3 
 * processor based microcontrollers.  This file can be freely distributed 
 * within development tools that are supporting such ARM based processors. 
 *
 * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * ARM SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 ******************************************************************************/
#include <stdint.h>
#include "LPC11xx.h"

#define CLOCK_SETUP           1
#define MAIN_PLL_SETUP        1

#define MAIN_CLKSRCSEL_Val    0x00000000
#define MAIN_PLL_M_Val        0x00000003
#define MAIN_PLL_P_Val        0x00000001

#define SYS_AHB_DIV_Val       1			/* 1 through 255, typical is 1 or 2 or 4 */

/*----------------------------------------------------------------------------
  DEFINES
 *----------------------------------------------------------------------------*/
    
/*----------------------------------------------------------------------------
  Define clocks
 *----------------------------------------------------------------------------*/
#define XTAL        (12000000UL)        /* Oscillator frequency               */
#define OSC_CLK     (      XTAL)        /* Main oscillator frequency          */
#define IRC_OSC     (12000000UL)        /* Internal RC oscillator frequency   */
#define WDT_OSC     (  250000UL)        /* WDT oscillator frequency           */

/*----------------------------------------------------------------------------
  Clock Variable definitions
 *----------------------------------------------------------------------------*/
uint32_t ClockSource = IRC_OSC;
uint32_t SystemFrequency = IRC_OSC; /*!< System Clock Frequency (Core Clock)  */
uint32_t SystemAHBFrequency = IRC_OSC;

/**
 * Misc. clock generation modules
 *
 * @param  none
 * @return none
 *
 * @brief  Setup the microcontroller system.
 *         Initialize the System and update the SystemFrequency variable.
 */
void Main_PLL_Setup ( void )
{
  uint32_t regVal;

  ClockSource = OSC_CLK;
  LPC_SYSCON->SYSPLLCLKSEL = MAIN_CLKSRCSEL_Val;   /* Select system OSC */
  LPC_SYSCON->SYSPLLCLKUEN = 0x01;                 /* Update clock source */
  LPC_SYSCON->SYSPLLCLKUEN = 0x00;                 /* toggle Update register once */
  LPC_SYSCON->SYSPLLCLKUEN = 0x01;
  while ( !(LPC_SYSCON->SYSPLLCLKUEN & 0x01) ); /* Wait until updated */

  regVal = LPC_SYSCON->SYSPLLCTRL;
  regVal &= ~0x1FFul;
  LPC_SYSCON->SYSPLLCTRL = (regVal | (MAIN_PLL_P_Val<<5) | MAIN_PLL_M_Val);
  
  /* Enable main system PLL, main system PLL bit 7 in PDRUNCFG. */
  LPC_SYSCON->PDRUNCFG &= ~(0x1<<7);
  while ( !(LPC_SYSCON->SYSPLLSTAT & 0x01) );	/* Wait until it's locked */

  LPC_SYSCON->MAINCLKSEL = 0x03;		/* Select PLL clock output */
  LPC_SYSCON->MAINCLKUEN = 0x01;		/* Update MCLK clock source */
  LPC_SYSCON->MAINCLKUEN = 0x00;		/* Toggle update register once */
  LPC_SYSCON->MAINCLKUEN = 0x01;
  while ( !(LPC_SYSCON->MAINCLKUEN & 0x01) );	/* Wait until updated */

  LPC_SYSCON->SYSAHBCLKDIV = SYS_AHB_DIV_Val;	/* SYS AHB clock, typical is 1 or 2 or 4 */

#if MAIN_PLL_SETUP
  SystemFrequency = ClockSource * (MAIN_PLL_M_Val+1);
#else
  SystemFrequency = ClockSource;
#endif
  SystemAHBFrequency = (uint32_t)(SystemFrequency / SYS_AHB_DIV_Val);
  return;
}

/**
 * Initialize the system
 *
 * @param  none
 * @return none
 *
 * @brief  Setup the microcontroller system.
 *         Initialize the System and update the SystemFrequency variable.
 */
void SystemInit (void)
{
  uint32_t i;

#if (CLOCK_SETUP)                       /* Clock Setup */
  /* bit 0 default is crystal bypass, 
  bit1 0=0~20Mhz crystal input, 1=15~50Mhz crystal input. */
  LPC_SYSCON->SYSOSCCTRL = 0x00;

  /* main system OSC run is cleared, bit 5 in PDRUNCFG register */
  LPC_SYSCON->PDRUNCFG &= ~(0x1ul << 5);

  /* Wait 200us for OSC to be stablized, no status 
  indication, dummy wait. */
  for ( i = 0; i < 0x100; i++ );

#if (MAIN_PLL_SETUP)
  Main_PLL_Setup();  
#endif

#endif	/* endif CLOCK_SETUP */

  /* System clock to the IOCON needs to be enabled or
  most of the I/O related peripherals won't work. */
  LPC_SYSCON->SYSAHBCLKCTRL |= (1ul << 16);
  return;
}