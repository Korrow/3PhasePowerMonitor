/********************************************************************
* main.c
* 
* Main module. Calls the initialization functions and then enters the
* main loop which will call all of the tasks in turn
*  
* 
* Author: Keegan Morrow
* Date: 6/6/2013
* Project: 3 Phase Power monitor 
*  
********************************************************************/

#include "spi.h"
#include "ff.h"
#include "msp430f5172.h"
#include "io.h"
#include "fonts.h"
#include "glcd.h"
#include "rtc_calendar.h"
#include "rtc_local.h"
#include "clock.h"
#include "core.h"
#include "userinterface.h"
#include "meter.h"


FATFS Fatfs;        /* File system object */
FIL Fil;            /* File object */
INT8U Buff[128];        /* File read buffer */


void die (      /* Stop with dying message */
    FRESULT rc  /* FatFs return value */
)
{
    //printf("Failed with rc=%u.\n", rc);
    for (;;) ;
}

char* itoa(int val);

/*-----------------------------------------------------------------------*/
/* Program Main                                                          */
/*-----------------------------------------------------------------------*/

int main (void)
{

    WDTCTL = WDTPW + WDTHOLD;
    Clocks_Init();
    IOinit();
    __enable_interrupt();
    SPIinit();
    COREinit();
    while(1){
        IObuttonTask();
        METERreadTask();
        UI_Draw();
        glcd_update_task();
        COREwaitForExpire();
    }
}



/*---------------------------------------------------------*/
/* User Provided Timer Function for FatFs module           */
/*---------------------------------------------------------*/

DWORD get_fattime (void)
{
    return    ((DWORD)(2012 - 1980) << 25)  /* Year = 2012 */
            | ((DWORD)1 << 21)              /* Month = 1 */
            | ((DWORD)1 << 16)                /* Day_m = 1*/
            | ((DWORD)0 << 11)               /* Hour = 0 */
            | ((DWORD)0 << 5)              /* Min = 0 */
            | ((DWORD)0 >> 1);             /* Sec = 0 */
}

// trap isr assignation - put all unused ISR vector here
#pragma vector = ADC10_VECTOR, COMP_B_VECTOR, DMA_VECTOR, PORT1_VECTOR,\
    SYSNMI_VECTOR, TEC0_VECTOR, TEC1_VECTOR, TIMER0_A1_VECTOR,\
    TIMER0_D0_VECTOR,TIMER1_D0_VECTOR, TIMER1_D1_VECTOR,\
    UNMI_VECTOR, USCI_A0_VECTOR, USCI_B0_VECTOR, WDT_VECTOR

__interrupt void TrapIsr(void)
{
    // Trap the MCU so that the problem can be found
    while(1);
}
