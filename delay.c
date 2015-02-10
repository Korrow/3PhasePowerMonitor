/********************************************************************
* delay.c
* 
* Module used for delays in the graphical LCD function
*  
* 
* Author: David Cannings
* Project: 3 Phase Power monitor 
*  
********************************************************************/
/*
 *  Delay functions
 *  See delay.h for details
 *
 *  Make sure this code is compiled with full optimization!!!
 */

#include    "delay.h"

void DelayUs( unsigned char x)
{
    unsigned char _dcnt;

    if(x>=4){
        _dcnt=(x*(FREQ_MULT)/2);
    }
    else{
        _dcnt=1;
    }
    while(--_dcnt > 0)
    {
        asm(";nop");
        asm(";nop");
        continue;
    }
        }

void
DelayMs(unsigned char cnt) {
    unsigned char i;
    while (cnt--) {
        i=4;
        while(i--) {
            DelayUs(uS_CNT);    /* Adjust for error */
        } ;
    } ;
}


