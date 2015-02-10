/********************************************************************
* meter.h
* 
* This module has the initialization functions and the task to
* communicate with the MAXQ3180 measurement IC. These measurements
* are stored in variables where they can be read by other modules
*  
* 
* Author: Keegan Morrow
* Date: 6/6/2013
* Project: 3 Phase Power monitor 
*  
********************************************************************/

#ifndef METER_H_
#define METER_H_

typedef enum{
    COMMAND_CODE_READ = 0x00,
    COMMAND_CODE_RESERVED1 = 0x40,
    COMMAND_CODE_WRITE = 0x80,
    COMMAND_CODE_RESERVED2 = 0xC0
}COMMAND_CODE;

typedef enum{
    DATA_LENGTH_1 = 0x00,
    DATA_LENGTH_2 = 0x10,
    DATA_LENGTH_4 = 0x20,
    DATA_LENGTH_8 = 0x30
}DATA_LENGTH;

typedef struct{
    INT32U VoltageRms;
    INT32U CurrentRms;
    INT32S PowReal;
    INT32S PowReactive;z
    INT32S PowApparent;
    INT32S PowFactor;
}PHASE_MEASUREMENTS;

typedef struct{
    INT32U VoltageRms;
    INT32U CurrentRms;
    INT32S PowReal;
    INT32S PowReactive;
    INT32S PowApparent;
    INT16S PowFactor;
    INT16U LineFrequency;
}TOTAL_MEASUREMENTS;

typedef struct{
    PHASE_MEASUREMENTS PhaseA;
    PHASE_MEASUREMENTS PhaseB;
    PHASE_MEASUREMENTS PhaseC;
    TOTAL_MEASUREMENTS Total;
}SYSTEM_MEASUREMENTS;

extern SYSTEM_MEASUREMENTS METERMeasTable;

void METERinit(void);

void METERreadTask(void);

INT8U METERcommand(COMMAND_CODE commandcode, DATA_LENGTH datalength, INT16U address, INT8U * databuffer);

#endif /* METER_H_ */
