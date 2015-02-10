/********************************************************************
* clock.c
* 
* Module used to set up the system clocks (MCLK, SMCLK, and ACLK)
*  
* 
* Author: Keegan Morrow
* Date: 6/6/2013
* Project: 3 Phase Power monitor 
*  
********************************************************************/
#include "standard.h"
#include "msp430f5172.h"
void Clocks_Init(void)
{

  // Setup Port Pins
  BIT_SET(PJSEL, BIT4+BIT5);                // Port select XT1
  BIT_CLR(UCSCTL6, XT1OFF);                     // XT1 On
  BIT_SET(UCSCTL6, XCAP_3);                        // Internal load cap

  // Loop until XT1 fault flag is cleared
  do
  {
      BIT_CLR(UCSCTL7, XT1LFOFFG);                  // Clear XT1 fault flags
  }while (UCSCTL7&XT1LFOFFG);               // Test XT1 fault flag

  __bis_SR_register(SCG0);                  // Disable the FLL control loop
  UCSCTL0 = 0x0000;                         // Set lowest possible DCOx, MODx
  UCSCTL1 = DCORSEL_5;                      // Select DCO range 24MHz operation
  UCSCTL2 = FLLD_1 + 374;                   // Set DCO Multiplier for 12MHz
                                            // (N + 1) * FLLRef = Fdco
                                            // (374 + 1) * 32768 = 12MHz
                                            // Set FLL Div = fDCOCLK/2
  __bic_SR_register(SCG0);                  // re-enable the FLL control loop

  // Worst-case settling time for the DCO when the DCO range bits have been
  // changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
  // UG for optimization.
  // 32 x 32 x 12 MHz / 32,768 Hz = 375000 = MCLK cycles for DCO to settle
  __delay_cycles(375000);

  // Loop until XT1 & DCO fault flag is cleared
  do
  {
    UCSCTL7 &= ~(XT1LFOFFG + XT1HFOFFG + DCOFFG);
                                            // Clear XT1,DCO fault flags
    SFRIFG1 &= ~OFIFG;                      // Clear fault flags
  }while (SFRIFG1&OFIFG);                   // Test oscillator fault flag
}

