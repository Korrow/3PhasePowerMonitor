/********************************************************************
* measurement.c
* 
* This module has all the typedefs and functions that will allow the 
* UI to keep track of the present measurement mode
*  
* 
* Author: Keegan Morrow
* Date: 6/6/2013
* Project: 3 Phase Power monitor 
*  
********************************************************************/

#include "msp430f5172.h"
#include "measurement.h"

MEASUREMENT_MODE MEAS_MeasMode = MEAS_MODE_VOLTS_PHASE;

SYS_MODE MEAS_SysMode = SYS_MODE_DELTA;

MEAS_MODE_T MEAS_Mode_t = MEAS_MODE_T_LINE;

MEAS_MODE_UNIT MEAS_Mode_Units = MEAS_MODE_UNIT_VOLTS;

void MEAS_SetSysMode_Delta(void){
    MEAS_SysMode = SYS_MODE_DELTA;
}

void MEAS_SetSysMode_Wye(void){
    MEAS_SysMode = SYS_MODE_WYE;
}

void MEAS_SetSysMode_WyeN(void){
    MEAS_SysMode = SYS_MODE_WYE_N;
}

void MEAS_SetMeasMode_VPhase(void){
    MEAS_MeasMode = MEAS_MODE_VOLTS_PHASE;
    MEAS_Mode_t = MEAS_MODE_T_PHASE;
    MEAS_Mode_Units = MEAS_MODE_UNIT_VOLTS;
}

void MEAS_SetMeasMode_VLine(void){
    MEAS_MeasMode = MEAS_MODE_VOLTS_LINE;
    MEAS_Mode_t = MEAS_MODE_T_LINE;
    MEAS_Mode_Units = MEAS_MODE_UNIT_VOLTS;
}

void MEAS_SetMeasMode_APhase(void){
    MEAS_MeasMode = MEAS_MODE_AMPS_PHASE;
    MEAS_Mode_t = MEAS_MODE_T_PHASE;
    MEAS_Mode_Units = MEAS_MODE_UNIT_AMPS;
}

void MEAS_SetMeasMode_ALine(void){
    MEAS_MeasMode = MEAS_MODE_AMPS_LINE;
    MEAS_Mode_t = MEAS_MODE_T_LINE;
    MEAS_Mode_Units = MEAS_MODE_UNIT_AMPS;
}

void MEAS_SetMeasMode_PActive(void){
    MEAS_MeasMode = MEAS_MODE_POWER_ACTIVE;
    MEAS_Mode_t = MEAS_MODE_T_POWER;
    MEAS_Mode_Units = MEAS_MODE_UNIT_W;
}

void MEAS_SetMeasMode_PReactive(void){
    MEAS_MeasMode = MEAS_MODE_POWER_REACTIVE;
    MEAS_Mode_t = MEAS_MODE_T_POWER;
    MEAS_Mode_Units = MEAS_MODE_UNIT_VAR;
}

void MEAS_SetMeasMode_PApparent(void){
    MEAS_MeasMode = MEAS_MODE_POWER_APPARENT;
    MEAS_Mode_t = MEAS_MODE_T_POWER;
    MEAS_Mode_Units = MEAS_MODE_UNIT_VA;
}

void MEAS_SetMeasMode_LineFrequency(void){
    MEAS_MeasMode = MEAS_MODE_LINE_FREQUENCY;
    MEAS_Mode_t = MEAS_MODE_T_LINE;
    MEAS_Mode_Units = MEAS_MODE_UNIT_HZ;
}
