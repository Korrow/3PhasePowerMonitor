/********************************************************************
* measurement.h
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

#ifndef MEASUREMENT_H_
#define MEASUREMENT_H_

#include "standard.h"

typedef enum{
    SYS_MODE_DELTA,
    SYS_MODE_WYE,
    SYS_MODE_WYE_N
}SYS_MODE;

typedef enum{
    MEAS_MODE_SOURCE_1,
    MEAS_MODE_SOURCE_2,
    MEAS_MODE_SOURCE_3,
    MEAS_MODE_SOURCE_N,
    MEAS_MODE_SOURCE_AB,
    MEAS_MODE_SOURCE_BC,
    MEAS_MODE_SOURCE_CA,
    MEAS_MODE_SOURCE_AVG,
    MEAS_MODE_SOURCE_TOTAL,
    MEAS_MODE_SOURCE_COUNT
}MEAS_MODE_SOURCE;

typedef enum{
    MEAS_MODE_VOLTS_PHASE,
    MEAS_MODE_VOLTS_LINE,
    MEAS_MODE_AMPS_PHASE,
    MEAS_MODE_AMPS_LINE,
    MEAS_MODE_POWER_ACTIVE,
    MEAS_MODE_POWER_REACTIVE,
    MEAS_MODE_POWER_APPARENT,
    MEAS_MODE_LINE_FREQUENCY,
    MEAS_MODE_COUNT
}MEASUREMENT_MODE;

/*  Come up with some better names for these, for now the t is target but
    that doesn't make much sense    */
/*  These typedefs are used to communicate information about a measurement
    that is passed to another function*/
typedef enum{
    MEAS_MODE_T_LINE = 0,
    MEAS_MODE_T_PHASE,
    MEAS_MODE_T_POWER,
    MEAS_MODE_T_COUNT
}MEAS_MODE_T;

typedef enum{
    MEAS_MODE_UNIT_VOLTS = 0,
    MEAS_MODE_UNIT_AMPS,
    MEAS_MODE_UNIT_VA,
    MEAS_MODE_UNIT_W,
    MEAS_MODE_UNIT_VAR,
    MEAS_MODE_UNIT_HZ,
    MEAS_MODE_UNIT_COUNT
}MEAS_MODE_UNIT;

// This is a huge struct, it may not be practical to store data like this.
typedef struct{
    INT16S VOLTS_P1;
    INT16S VOLTS_P2;
    INT16S VOLTS_P3;
    INT16S VOLTS_P_AVG;
    INT16S VOLTS_L1;
    INT16S VOLTS_L2;
    INT16S VOLTS_L3;
    INT16S VOLTS_L_AVG;
    INT16S AMPS_P1;
    INT16S AMPS_P2;
    INT16S AMPS_P3;
    INT16S AMPS_P_AVG;
    INT16S AMPS_L1;
    INT16S AMPS_L2;
    INT16S AMPS_L3;
    INT16S AMPS_L_AVG;
    INT16S POWER_ACT_P1;
    INT16S POWER_ACT_P2;
    INT16S POWER_ACT_P3;
    INT16S POWER_ACT_T;
    INT16S POWER_REAC_P1;
    INT16S POWER_REAC_P2;
    INT16S POWER_REAC_P3;
    INT16S POWER_REAC_T;
    INT16S POWER_APP_P1;
    INT16S POWER_APP_P2;
    INT16S POWER_APP_P3;
    INT16S POWER_APP_T;
}MEASUREMENT_STRUCT;

extern MEASUREMENT_MODE MEAS_MeasMode;

extern SYS_MODE MEAS_SysMode;

extern MEASUREMENT_STRUCT MEAS_results;

extern MEAS_MODE_T MEAS_Mode_t;

extern MEAS_MODE_UNIT MEAS_Mode_Units;

void MEAS_SetSysMode_Delta(void);

void MEAS_SetSysMode_Wye(void);

void MEAS_SetSysMode_WyeN(void);

void MEAS_SetMeasMode_VPhase(void);

void MEAS_SetMeasMode_VLine(void);

void MEAS_SetMeasMode_APhase(void);

void MEAS_SetMeasMode_ALine(void);

void MEAS_SetMeasMode_PActive(void);

void MEAS_SetMeasMode_PReactive(void);

void MEAS_SetMeasMode_PApparent(void);

void MEAS_SetMeasMode_LineFrequency(void);

#endif /* MEASUREMENT_H_ */
