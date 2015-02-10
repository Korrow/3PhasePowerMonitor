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

#ifndef RTC_H_
#define RTC_H_

typedef enum{
    DAY_SUNDAY=0,
    DAY_MONDAY,
    DAY_TUESDAY,
    DAY_WEDNESDAY,
    DAY_THURSDAY,
    DAY_FRIDAY,
    DAY_SATURDAY,
    DAY_COUNT
}RTC_DAY;

typedef enum{
    MONTH_JANUARY = 0,
    MONTH_FEBRUARY,
    MONTH_MARCH,
    MONTH_APRIL,
    MONTH_MAY,
    MONTH_JUNE,
    MONTH_JULY,
    MONTH_AUGUST,
    MONTH_SEPTEMBER,
    MONTH_OCTOBER,
    MONTH_NOVEMBER,
    MONTH_DECEMBER,
    MONTH_COUNT
}RTC_MONTH;

typedef struct{
    INT8U second;
    INT8U minute;
    INT8U hour;
    INT8U day;
    RTC_MONTH month;
    INT8U year;
}RTC_TIME;


INT8U RTCgetSecond(void);
INT8U RTCgetMinute(void);
INT8U RTCgetHour(void);
INT8U RTCgetDay(void);
RTC_MONTH RTCgetMonth(void);
INT8U RTCgetYear(void);

void RTCAddSeconds(RTC_TIME *pmoddedtime, INT8U seconds);
void RTCAddMinutes(RTC_TIME *pmoddedtime, INT8U minutes);
void RTCAddHours(RTC_TIME *pmoddedtime, INT8U hours);
void RTCAddDays(RTC_TIME *pmoddedtime, INT8U days);
void RTCAddMonths(RTC_TIME *pmoddedtime, INT8U months);
void RTCAddYears(RTC_TIME *pmoddedtime, INT16U years);

void RTCincrementSecond(void);
void RTCincrementMinute(void);
void RTCincrementHour(void);
void RTCincrementDay(void);
void RTCincrementMonth(void);
void RTCincrementYear(void);

void RTCdecrementSecond(void);
void RTCdecrementMinute(void);
void RTCdecrementHour(void);
void RTCdecrementDay(void);
void RTCdecrementMonth(void);
void RTCdecrementYear(void);

#define RTC_MIN_LENGTH      (60)
#define RTC_HOUR_LENGTH     (60)
#define RTC_DAY_LENGTH      (24)

#endif /* RTC_H_ */
