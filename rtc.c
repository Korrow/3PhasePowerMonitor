/********************************************************************
* rtc.c
* 
* This module has functions and variables that are used to handle
* the real time clock. It is reliant on an interrupt to call the
* increment second function in order to update the real time clock
*  
* 
* Author: Keegan Morrow
* Date: 6/6/2013
* Project: 3 Phase Power monitor 
*  
********************************************************************/
#include "msp430f5172.h"
#include "standard.h"
#include "rtc.h"

static RTC_TIME rtcCurrentTime={1,2,3,4,MONTH_JANUARY,13};

const INT8S monthLengthTable[MONTH_COUNT] = {31,-1,31,30,31,30,31,31,30,31,30,31};

void RTCincrementSecond(void){
    rtcCurrentTime.second++;
    if ( rtcCurrentTime.second > RTC_MIN_LENGTH){
        rtcCurrentTime.second = 0;
        RTCincrementMinute();
    }else{
        // Do not increment the other variables
    }
}

void RTCincrementMinute(void){
    rtcCurrentTime.minute++;
    if ( rtcCurrentTime.minute > RTC_HOUR_LENGTH){
        rtcCurrentTime.minute = 0;
        RTCincrementHour();
    }else{
        // Do not increment the other variables
    }
}

void RTCincrementHour(void){
    rtcCurrentTime.hour++;
    if ( rtcCurrentTime.hour >RTC_DAY_LENGTH){
        rtcCurrentTime.hour = 0;
        RTCincrementDay();
    }else{
        // Do not increment the other variables
    }
}

void RTCincrementDay(void){
    rtcCurrentTime.day++;
    if ( rtcCurrentTime.day >= monthLengthTable[rtcCurrentTime.month]){
        rtcCurrentTime.day = DAY_SUNDAY;
        RTCincrementMonth();
    }else{
        // Do not increment the other variables
    }
}

void RTCincrementMonth(void){
    rtcCurrentTime.month++;
    if ( rtcCurrentTime.month >= MONTH_COUNT){
        rtcCurrentTime.month = MONTH_JANUARY;
        RTCincrementYear();
    }else{
        // Do not increment the other variables
    }
}

void RTCdecrementSecond(void){
    if ( rtcCurrentTime.second >= 0){
        rtcCurrentTime.second--;
    }else{
        // Do not increment the other variables
    }
}

void RTCdecrementMinute(void){
    rtcCurrentTime.minute++;
    if ( rtcCurrentTime.minute > 0){
        rtcCurrentTime.minute--;
    }else{
        // Do not increment the other variables
    }
}
void RTCdecrementHour(void){
    if ( rtcCurrentTime.hour > 0){
        rtcCurrentTime.hour--;
    }else{
        // Do not increment the other variables
    }
}
void RTCdecrementDay(void){
    if ( rtcCurrentTime.day > 1){
        rtcCurrentTime.day--;
    }else{
        // Do not increment the other variables
    }
}
void RTCdecrementMonth(void){
    if ( rtcCurrentTime.month != MONTH_JANUARY){
        rtcCurrentTime.month--;
    }else{
        // Do not change
    }
}
void RTCdecrementYear(void){
    if ( rtcCurrentTime.year > 1 ){
        rtcCurrentTime.year--;
    }else{
        // Don't change the year
    }
}

void RTCincrementYear(void){
    if ( rtcCurrentTime.year < 99){
        rtcCurrentTime.year++;
    }else{
        // Don't change the year
    }
}

INT8U RTCgetSecond(void){
    return rtcCurrentTime.second;
}

INT8U RTCgetMinute(void){
    return rtcCurrentTime.minute;
}

INT8U RTCgetHour(void){
    return rtcCurrentTime.hour;
}

INT8U RTCgetDay(void){
    return rtcCurrentTime.day;
}

RTC_MONTH RTCgetMonth(void){
    return rtcCurrentTime.month;
}

INT8U RTCgetMonthInt(void){
    return rtcCurrentTime.month + 1;
}

INT8U RTCgetYear(void){
    return rtcCurrentTime.year;
}

void RTCAddSeconds(RTC_TIME *pmoddedtime, INT8U seconds){
    INT8U minutestoadd = 0;
    pmoddedtime->second += seconds;
    while(pmoddedtime->second >= RTC_MIN_LENGTH){
        pmoddedtime->second -= RTC_MIN_LENGTH;
        minutestoadd++;
    }
    RTCAddMinutes(pmoddedtime,minutestoadd);
}
void RTCAddMinutes(RTC_TIME *pmoddedtime, INT8U minutes){
    INT8U hourstoadd = 0;
    pmoddedtime->minute += minutes;
    while(pmoddedtime->minute >= RTC_HOUR_LENGTH){
        pmoddedtime->minute -= RTC_HOUR_LENGTH;
        hourstoadd++;
    }
    RTCAddHours(pmoddedtime,hourstoadd);
}
void RTCAddHours(RTC_TIME *pmoddedtime, INT8U hours){
    INT8U daystoadd = 0;
    pmoddedtime->hour += hours;
    while(pmoddedtime->hour >= RTC_DAY_LENGTH){
        pmoddedtime->hour -= RTC_DAY_LENGTH;
        daystoadd++;
    }
    RTCAddDays(pmoddedtime,daystoadd);
}
void RTCAddDays(RTC_TIME *pmoddedtime, INT8U days){
    INT8U monthstoadd = 0;
    pmoddedtime->day += days;
    while(pmoddedtime->day >= RTC_DAY_LENGTH){
        pmoddedtime->day -= RTC_DAY_LENGTH;
        monthstoadd++;
    }
    RTCAddMonths(pmoddedtime,monthstoadd);
}
void RTCAddMonths(RTC_TIME *pmoddedtime, INT8U months){
    INT8U yearstoadd = 0;
    pmoddedtime->month += months;
    while(pmoddedtime->day >= MONTH_COUNT){
        pmoddedtime->day -= MONTH_COUNT;
        yearstoadd++;
    }
    RTCAddYears(pmoddedtime,yearstoadd);
}
void RTCAddYears(RTC_TIME *pmoddedtime, INT16U years){
    pmoddedtime->year += years;
}
