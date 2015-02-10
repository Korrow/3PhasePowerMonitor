/********************************************************************
* spi.c
* 
*  This module has all of the low level SPI communication functions
* and the functions to initialize the hardware modules
*  
* 
* Author: Keegan Morrow
* Date: 6/6/2013
* Project: 3 Phase Power monitor 
*  
********************************************************************/

#include "msp430f5172.h"
#include "standard.h"
#include "spi.h"

void SPIinit(void)
{
    // Set up the SPI module as a master
    // Make this code able to operate on different modules etc

    // Disable the module before changing any settings
    UCA0CTL0 = UCSWRST;
    UCB0CTL0 = UCSWRST;
    UCA0BRW = 1;
    UCB0BRW = 13;
    UCA0CTL0 = (UCMST | UCSYNC | UCMODE_0 | UCCKPH | UCMSB | UCSWRST);
    UCB0CTL0 = (UCMST | UCSYNC | UCMODE_0 | UCCKPH | UCMSB | UCSWRST);
    UCA0CTL1 = UCSSEL__SMCLK;
    UCB0CTL1 = UCSSEL__SMCLK;
    // Do not use modulation
    UCA0MCTL = 0;
    // Exit reset mode
    BIT_CLR(UCA0CTL1, UCSWRST);
    BIT_CLR(UCB0CTL1, UCSWRST);
}

INT8U SPI_DataA0 ( INT8U Data)
{
    // Make sure any transmissions are finished
    while(!(UCA0IFG & UCTXIFG));
    UCA0TXBUF = Data;
    // Wait for transaction and any received data
    while (!(UCA0IFG & UCRXIFG));
    return (UCA0RXBUF);
}

INT8U SPI_DataB0 ( INT8U Data)
{
    // Make sure any transmissions are finished
    while(!(UCB0IFG & UCTXIFG));
    UCB0TXBUF = Data;
    // Wait for transaction and any received data
    while (!(UCB0IFG & UCRXIFG));
    return (UCB0RXBUF);
}
