/********************************************************************
* core.c
* 
* Module used to control the base functionality of the device. This includes
* the selection of different device modes and the kernel, as well as the timer
* used by the real time clock
*  
* 
* Author: Keegan Morrow
* Date: 6/6/2013
* Project: 3 Phase Power monitor 
*  
********************************************************************/

#include "standard.h"
#include "msp430f5172.h"
#include "meter.h"
#include "core.h"
#include "userinterface.h"
#include "glcd.h"
#include "rtc.h"
#include "RTC_Calendar.h"
#include "io.h"

RTC_TIME coreNextWakeup = {
    0,0,0,0,0,0
};

// For now this is seconds, just for testing
INT16U coreWakeInterval = 15;

#define KERNEL_PERIOD_SET       (328)

void coreDisablePeripherals(void);

void coreEnablePeripherals(void);

void coreUpdateWakeTime(void);

INT8U KernelContinue = 0;
CORE_STATE coreState = CORE_STATE_NO_INIT;

void COREinit(void){
    // Initialize the timer used for the real time clock

    // Set the timer to use ACLK as the clock source (assumes ACLK = 32768Hz)
    TA0CTL |= TASSEL__ACLK;
    // Enable timer CCR0 interrupts
    TA0CCTL0 |=CCIE;
    // Start the timer in count up mode
    TA0CTL |=MC__UP;
    // Factor to divide ACLK by
    TA0CCR0 = 32768 -1;

    // Initialize the timer used for the timeslicer
    TD0CCTL1 = OUTMOD_4 + CCIE;               // CCR1 toggle, interrupt enabled
    TD0CTL0 = TDSSEL_1 + MC_2 + TDCLR + TDIE; // ACLK, contmode, clear TDR,

    // Enter the initial state
    COREsetState(CORE_STATE_METER);

    // Enable global interrupts
    __bis_SR_register(GIE);
}

CORE_STATE COREgetState(void){
    return coreState;
}

void COREsetState(CORE_STATE newcorestate){
    CORE_STATE oldcorestate = coreState;
    INT8U sleep = 0;
    if ( oldcorestate != newcorestate ){
        coreState = newcorestate;
        // Shut down any unneeded stuff
        switch(oldcorestate){
        case CORE_STATE_NO_INIT:
            // No need to shut anything down
            break;
        case CORE_STATE_METER:
            glcd_shutdown();
            break;
        case CORE_STATE_LOG_SLEEP:
            IOdisableButtonInterrupts();
            break;
        case CORE_STATE_LOG_WAKE:

            break;
        case CORE_STATE_COUNT:

            break;
        default:
            //Error
            break;
        }
        // Initialize the new features needed
        switch(newcorestate){
        case CORE_STATE_NO_INIT:
            // Error
            break;
        case CORE_STATE_METER:
            coreEnablePeripherals();
            UIinit();
            METERinit();
            glcd_init();
            break;
        case CORE_STATE_LOG_SLEEP:
            coreDisablePeripherals();
            IOenableButtonInterrupts();
            coreUpdateWakeTime();
            sleep = 1;
            break;
        case CORE_STATE_LOG_WAKE:
            coreEnablePeripherals();
            METERinit();
            break;
        case CORE_STATE_COUNT:
            // Error
            break;
        default:
            // Error
            break;
        }
        if ( sleep ){
            COREsleep();
        }else{
            //Don't sleep
        }
    }else{
        // No change, do nothing
    }
}

void coreUpdateWakeTime(void){
    coreNextWakeup.second = RTCgetSecond();
    coreNextWakeup.minute = RTCgetMinute();
    coreNextWakeup.hour = RTCgetHour();
    coreNextWakeup.day = RTCgetDay();
    coreNextWakeup.month = RTCgetMonth();
    coreNextWakeup.year = RTCgetYear();

    RTCAddSeconds(&coreNextWakeup,coreWakeInterval);
}

void COREwaitForExpire(void){
    while(!KernelContinue){}
    KernelContinue = 0;
}

// Timer0_D1 Interrupt Vector (TDIV) handler
#pragma vector=TIMER0_D1_VECTOR
__interrupt void COREslicerInt(void)
{
    switch(__even_in_range(TD0IV,30))
    {
    case 0: break;
    case 2:

        TD0CCR1 += KERNEL_PERIOD_SET;       // Add Offset to CCR1
        KernelContinue++;
        break;
    case 4:  break;                         // CCR2 not used
    case 6:  break;                         // CCR3 not used
    case 8:  break;                         // CCR4 not used
    case 10: break;                         // reserved
    case 12: break;                         // reserved
    case 14: break;
    case 16: break;                         // overflow
    case 18: break;                         // Clock fail low
    case 20: break;                         // Clock fail high
    case 22: break;                         // Hi-res freq locked
    case 24: break;                         // Hi-res freq unlocked
    case 26: break;                         // reserved
    case 28: break;                         // reserved
    case 30: break;                         // reserved
    default: break;

    }
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A(void){
    RTCincrementSecond();
    if ( coreState == CORE_STATE_LOG_SLEEP){
        if (RTCgetYear() >= coreNextWakeup.year){
            if(RTCgetMonth() >= coreNextWakeup.month){
                if(RTCgetDay() >= coreNextWakeup.day){
                    if(RTCgetHour() >= coreNextWakeup.hour){
                        if(RTCgetMinute() >= coreNextWakeup.minute){
                            if((RTCgetSecond() >= coreNextWakeup.second)){
                                COREsetState(CORE_STATE_LOG_WAKE);
                                __bic_SR_register_on_exit(LPM3_bits);
                            }
                        }
                    }
                }
            }
        }
    }else{
        // Unhandled core state
    }
}


void COREsleep(void){
    _BIS_SR(LPM3_bits | GIE);
}

void COREenterLoggingMode(void){
    // Set the alarm here as well
    COREsetState(CORE_STATE_LOG_SLEEP);
}

void coreDisablePeripherals(void){
    // Removes power from all of the peripherals
    BIT_CLR(VREG_ON_POUT,VREG_ON_BIT);
}

void coreEnablePeripherals(void){
    BIT_CLR(VREG_ON_POUT,VREG_ON_BIT);
}
