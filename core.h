/********************************************************************
* core.h
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

#ifndef CORE_H_
#define CORE_H_

typedef enum{
	CORE_STATE_NO_INIT,
	CORE_STATE_METER,
	CORE_STATE_LOG_SLEEP,
	CORE_STATE_LOG_WAKE,
	CORE_STATE_COUNT
}CORE_STATE;

void COREinit(void);

void COREsetState(CORE_STATE newcorestate);

void COREenterLoggingMode(void);

CORE_STATE COREgetState(void);

void COREsetWakeCounter(INT8U newwakecounter);

void COREsleep(void);

void COREwaitForExpire(void);

#endif /* CORE_H_ */
