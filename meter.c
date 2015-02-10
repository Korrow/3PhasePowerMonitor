/********************************************************************
* meter.c
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
/*
 * meter.c
 *
 *  Created on: Jan 15, 2013
 *      Author: Keegan
 */

#include "standard.h"
#include "msp430f5172.h"
#include "core.h"
#include "io.h"
#include "spi.h"
#include "meter.h"

// This is an arbitrarily chosen dummy value
#define COMMAND_DUMMY       (0x00)
#define COMMAND_ACK         (0x41)
#define COMMAND_NACK        (0x4E)

#define WAIT_BETWEEN_BYTES (1200)

//Gonna have to double chcek the value of this guy
#define METER_MAX_VIRTUAL_REGISTER_LENGTH   (8)

//Conversion Constant information

// The full scale input voltage to the ADC (1.024V by default)
#define METER_ADC_FULLSCALE_VOLTAGE         (1.024)

// The number of A/D counts used for voltage registers (2^24)
#define METER_VOLTAGE_COUNTS                (16777216.0)

// The voltage divider ratio of the input circuitry
#define METER_VOLTAGE_DIV_RATIO             (501.0)

// The maximum voltage that can be measured
#define METER_VOLTAGE_FULL_SCALE            (METER_ADC_FULLSCALE_VOLTAGE*METER_VOLTAGE_DIV_RATIO)

// The value of one LSB, typically between 1mV to 1nV
#define METER_VOLTAGE_LSB                   (152.6e-10)

// The conversion constant used for voltage measurements, should be greater than 1000 to avoid conversion loss
#define METER_VOLTAGE_CONVERSION_CONSTANT   ((METER_VOLTAGE_FULL_SCALE)/(METER_VOLTAGE_COUNTS*METER_VOLTAGE_LSB))

// The number of A/D counts used for current registers (2^24)
#define METER_CURRENT_COUNTS                (16777216.0)

// The current transformer ratio
#define METER_CURRENT_TRANS_RATIO           (2000.0)

// The value of the burden resistance for currents
#define METER_CURRENT_BURDEN_VAL            (40.0)

// The current to voltage conversion constant
#define METER_CURRENT_RATIO                 (METER_CURRENT_TRANS_RATIO/METER_CURRENT_BURDEN_VAL)

// The maximum current that can be measured
#define METER_CURRENT_FULL_SCALE            (METER_ADC_FULLSCALE_VOLTAGE*METER_CURRENT_RATIO)

// The value of one LSB, typically between 1nA to 10uA
#define METER_CURRENT_LSB                   (152.6e-10)

// The conversion constant used for voltage measurements, should be greater than 1000 to avoid conversion loss
#define METER_CURRENT_CONVERSION_CONSTANT   ((METER_CURRENT_FULL_SCALE)/(METER_CURRENT_COUNTS*METER_CURRENT_LSB))

// The number of A/D counts used for current registers (2^32)
#define METER_POWER_COUNTS              (4294967296.0)

// The value of one LSB, typically between 1mV to 1nV
#define METER_POWER_LSB                 (76.29e-9)

// The maximum power that can be measured
#define METER_POWER_FULL_SCALE          (METER_VOLTAGE_FULL_SCALE*METER_CURRENT_FULL_SCALE)

// The conversion constant used for power measurements, should be greater than 1000 to avoid conversion loss
#define METER_POWER_CONVERSION_CONSTANT ((METER_POWER_FULL_SCALE)/(METER_POWER_COUNTS*METER_POWER_LSB))

typedef struct{
    INT16U VoltsCc;
    INT16U AmpsCc;
    INT16U PowerCc;
    INT16U EnergyCc;
}METER_CONVERSION_CONSTANT_STRUCT;

const METER_CONVERSION_CONSTANT_STRUCT ConversionConstantTable = {
        METER_VOLTAGE_CONVERSION_CONSTANT,
        METER_CURRENT_CONVERSION_CONSTANT,
        METER_POWER_CONVERSION_CONSTANT,
        METER_VOLTAGE_FULL_SCALE
};

typedef struct{
    INT16U ACurrent;
    INT16U AVoltage;
    INT16U BCurrent;
    INT16U BVoltage;
    INT16U CCurrent;
    INT16U CVoltage;
}METER_CALIB_STRUCT;

#define BASE_CAL_VALUE      (0x4000)
#define CAL_VOLTS_A     (BASE_CAL_VALUE * (118.7/99.15))
#define CAL_VOLTS_B     (BASE_CAL_VALUE * (119.4/99.35))
#define CAL_VOLTS_C     (BASE_CAL_VALUE * (118.9/98.43))

#define CAL_AMPS_A      (BASE_CAL_VALUE)
#define CAL_AMPS_B      (BASE_CAL_VALUE)
#define CAL_AMPS_C      (BASE_CAL_VALUE)

const METER_CALIB_STRUCT CalibrationConstantTable = {
    CAL_AMPS_A,
    CAL_VOLTS_A,
    CAL_AMPS_B,
    CAL_VOLTS_B,
    CAL_AMPS_C,
    CAL_VOLTS_C
};

SYSTEM_MEASUREMENTS METERMeasTable = {
        {0,0,0,0,0,0},
        {0,0,0,0,0,0},
        {0,0,0,0,0,0},
        {0,0,0,0,0}
};

// The location of the UDP special function register
#define ADDR_UPD_SFR    (0x900)
// The location of operating mode register 1 in RAM
#define ADDR_OPMODE0    (0x0001)
// The bit in OPMODE0 that is EXTCLK
#define BIT_EXTCLK      (0x10)

// Location of the IRQ_FLAG register on the MAXQ3180
#define ADDR_IRQ_FLAG   (0x004)
// Bit value of the various flags
#define IRQ_DSPRDY (0x4000)
// Addresses of the conversion constants on the MAXQ3180
#define ADDR_VOLTS_CC   (0x014)
#define ADDR_AMP_CC     (0x016)
#define ADDR_PWR_CC     (0x018)
#define ADDR_ENR_CC     (0x01A)

// Addresses of the conversion constants on the MAXQ3180
#define ADDR_A_I_GAIN   (0x130)
#define ADDR_A_V_GAIN   (0x132)
#define ADDR_B_I_GAIN   (0x21C)
#define ADDR_B_V_GAIN   (0x21E)
#define ADDR_C_I_GAIN   (0x308)
#define ADDR_C_V_GAIN   (0x30A)

// Addresses of the measurements on the MAXQ3180
// Locations of the RMS voltage registers for each phase
#define ADD_A_VRMS      (0x831)
#define ADD_B_VRMS      (0x832)
#define ADD_C_VRMS      (0x834)
// Locations of the RMS current registers for each phase
#define ADD_A_IRMS      (0x841)
#define ADD_B_IRMS      (0x842)
#define ADD_C_IRMS      (0x844)
// Locations of the real power registers for each phase and the total
#define ADD_A_PWRP      (0x801)
#define ADD_B_PWRP      (0x802)
#define ADD_C_PWRP      (0x804)
#define ADD_T_PWRP      (0x807)
// Locations of the reactive power registers for each phase and the total
#define ADD_A_PWRQ      (0x811)
#define ADD_B_PWRQ      (0x812)
#define ADD_C_PWRQ      (0x814)
#define ADD_T_PWRQ      (0x817)
// Locations of the apparent power registers for each phase and the total
#define ADD_A_PWRS      (0x821)
#define ADD_B_PWRS      (0x822)
#define ADD_C_PWRS      (0x824)
#define ADD_T_PWRS      (0x827)
// Locations of the power factor registers for each phase
#define ADD_A_PF        (0x1C6)
#define ADD_B_PF        (0x2B2)
#define ADD_C_PF        (0x39E)

// Location of the line frequency register
#define ADD_LINEFR      (0x062)

typedef enum{
    TASK_STATE_DSPRDY_CHECK=0,
    TASK_STATE_GET_A_VRMS,
    TASK_STATE_GET_B_VRMS,
    TASK_STATE_GET_C_VRMS,
    TASK_STATE_GET_A_IRMS,
    TASK_STATE_GET_B_IRMS,
    TASK_STATE_GET_C_IRMS,
    TASK_STATE_GET_A_PWRP,
    TASK_STATE_GET_B_PWRP,
    TASK_STATE_GET_C_PWRP,
    TASK_STATE_GET_T_PWRP,
    TASK_STATE_GET_A_PWRQ,
    TASK_STATE_GET_B_PWRQ,
    TASK_STATE_GET_C_PWRQ,
    TASK_STATE_GET_T_PWRQ,
    TASK_STATE_GET_A_PWRS,
    TASK_STATE_GET_B_PWRS,
    TASK_STATE_GET_C_PWRS,
    TASK_STATE_GET_T_PWRS,
    TASK_STATE_GET_A_PF,
    TASK_STATE_GET_B_PF,
    TASK_STATE_GET_C_PF,
    TASK_STATE_GET_FREQ,
    TASK_STATE_CALC_AVG
}METER_TASK_STATE;

void METERinit(void){
    INT8U readbuffer[8];
    INT8U opmodeval = BIT_EXTCLK;

    BIT_SET(MEAS_RST_POUT,MEAS_RST_BIT);

    // Since this device uses a crystal oscillator for a clock source instead of a crystal we need to set the EXTCLK bit
    // First write to the RAM register
    while(METERcommand(COMMAND_CODE_WRITE,DATA_LENGTH_1,ADDR_OPMODE0,&opmodeval)){
        __delay_cycles(8400000);
    }
    // Then read from the special command register to copy the value from the mirror register to the hardware register
    while(METERcommand(COMMAND_CODE_READ,DATA_LENGTH_1,ADDR_UPD_SFR,&opmodeval)){
        __delay_cycles(8400000);
    }
    // Write the conversion constants
    while(METERcommand(COMMAND_CODE_WRITE,DATA_LENGTH_8,ADDR_VOLTS_CC,(INT8U*)&ConversionConstantTable)){
        __delay_cycles(8400000);
    }
    METERcommand(COMMAND_CODE_READ,DATA_LENGTH_2,ADDR_VOLTS_CC,(INT8U*)&ConversionConstantTable);
    METERcommand(COMMAND_CODE_READ,DATA_LENGTH_2,ADDR_AMP_CC,readbuffer);
    METERcommand(COMMAND_CODE_READ,DATA_LENGTH_2,ADDR_PWR_CC,readbuffer);
    //Write the Phase A calibration constants
    while(METERcommand(COMMAND_CODE_WRITE,DATA_LENGTH_4,ADDR_A_I_GAIN,(INT8U*)&CalibrationConstantTable.ACurrent)){
        __delay_cycles(8400000);
    }
    METERcommand(COMMAND_CODE_READ,DATA_LENGTH_2,ADDR_A_I_GAIN,readbuffer);
    METERcommand(COMMAND_CODE_READ,DATA_LENGTH_2,ADDR_B_I_GAIN,readbuffer);
    METERcommand(COMMAND_CODE_READ,DATA_LENGTH_2,ADDR_C_I_GAIN,readbuffer);
    METERcommand(COMMAND_CODE_READ,DATA_LENGTH_2,ADDR_A_V_GAIN,readbuffer);
    //Write the Phase B calibration constants
    while(METERcommand(COMMAND_CODE_WRITE,DATA_LENGTH_4,ADDR_B_I_GAIN,(INT8U*)&CalibrationConstantTable.BCurrent)){
        __delay_cycles(8400000);
    }
    //Write the Phase C calibration constants
    while(METERcommand(COMMAND_CODE_WRITE,DATA_LENGTH_4,ADDR_C_I_GAIN,(INT8U*)&CalibrationConstantTable.CCurrent)){
        __delay_cycles(8400000);
    }
}

void METERreadTask(void){
    static INT8U readbuffer[METER_MAX_VIRTUAL_REGISTER_LENGTH];
    static INT8U taskdelaycount = 0;
    static INT16U cycledelay = 0;
    static METER_TASK_STATE taskstate = TASK_STATE_DSPRDY_CHECK;

    if ( COREgetState() == CORE_STATE_LOG_WAKE || COREgetState() == CORE_STATE_METER){
        if ( taskdelaycount > 0 ){
            taskdelaycount--;
            cycledelay++;
        }else{
            while(taskdelaycount <= 0 ){
                switch(taskstate){
                case TASK_STATE_DSPRDY_CHECK:
                    taskdelaycount = METERcommand(COMMAND_CODE_READ,DATA_LENGTH_2,ADDR_IRQ_FLAG,readbuffer);
                    if ( ( taskdelaycount == 0 )){
                        if ((*((INT16U*)readbuffer) & IRQ_DSPRDY) ){
                            (*((INT16U*)readbuffer)&= ~IRQ_DSPRDY);
                            taskdelaycount = METERcommand(COMMAND_CODE_WRITE,DATA_LENGTH_2,ADDR_IRQ_FLAG,readbuffer);
                            taskstate++;
                        }else{
                            //Wait for the DSP cycle to finish
                            taskdelaycount++;
                        }
                    }else{
                        //Continue
                    }
                    break;
                case TASK_STATE_GET_A_VRMS:
                    taskdelaycount = METERcommand(COMMAND_CODE_READ,DATA_LENGTH_8,ADD_A_VRMS,readbuffer);
                    if (taskdelaycount == 0){
                        METERMeasTable.PhaseA.VoltageRms = *((INT32U*)&readbuffer[2]);
                        taskstate = TASK_STATE_GET_B_VRMS;
                    }else{
                        // Continue
                    }
                    break;
                case TASK_STATE_GET_B_VRMS:
                    taskdelaycount = METERcommand(COMMAND_CODE_READ,DATA_LENGTH_8,ADD_B_VRMS,readbuffer);
                    if (taskdelaycount == 0){
                        METERMeasTable.PhaseB.VoltageRms = *((INT32U*)&readbuffer[2]);
                        taskstate = TASK_STATE_GET_C_VRMS;
                    }else{
                        // Continue
                    }
                    break;
                case TASK_STATE_GET_C_VRMS:
                    taskdelaycount = METERcommand(COMMAND_CODE_READ,DATA_LENGTH_8,ADD_C_VRMS,readbuffer);
                    if (taskdelaycount == 0){
                        METERMeasTable.PhaseC.VoltageRms = *((INT32U*)&readbuffer[2]);
                        taskstate = TASK_STATE_GET_A_IRMS;
                    }else{
                        // Continue
                    }
                    break;
                case TASK_STATE_GET_A_IRMS:
                    taskdelaycount = METERcommand(COMMAND_CODE_READ,DATA_LENGTH_8,ADD_A_IRMS,readbuffer);
                    if (taskdelaycount == 0){
                        METERMeasTable.PhaseA.CurrentRms = *((INT32U*)&readbuffer[2]);
                        taskstate = TASK_STATE_GET_B_IRMS;
                    }else{
                        // Continue
                    }
                    break;
                case TASK_STATE_GET_B_IRMS:
                    taskdelaycount = METERcommand(COMMAND_CODE_READ,DATA_LENGTH_8,ADD_B_IRMS,readbuffer);
                    if (taskdelaycount == 0){
                        METERMeasTable.PhaseB.CurrentRms = *((INT32U*)&readbuffer[2]);
                        taskstate = TASK_STATE_GET_C_IRMS;
                    }else{
                        // Continue
                    }
                    break;
                case TASK_STATE_GET_C_IRMS:
                    taskdelaycount = METERcommand(COMMAND_CODE_READ,DATA_LENGTH_8,ADD_C_IRMS,readbuffer);
                    if (taskdelaycount == 0){
                        METERMeasTable.PhaseC.CurrentRms = *((INT32U*)&readbuffer[2]);
                        taskstate = TASK_STATE_GET_A_PWRP;
                        cycledelay = 0;
                        taskdelaycount = 1;
                    }else{
                        // Continue
                    }
                    break;
                case TASK_STATE_GET_A_PWRP:
                    taskdelaycount = METERcommand(COMMAND_CODE_READ,DATA_LENGTH_8,ADD_A_PWRP,readbuffer);
                    if (taskdelaycount == 0){
                        METERMeasTable.PhaseA.PowReal = *((INT32S*)&readbuffer[2]);
                        taskstate = TASK_STATE_GET_B_PWRP;
                        cycledelay = 0;
                        taskdelaycount = 1;
                    }else{
                        // Continue
                    }
                    break;
                case TASK_STATE_GET_B_PWRP:
                    taskdelaycount = METERcommand(COMMAND_CODE_READ,DATA_LENGTH_8,ADD_B_PWRP,readbuffer);
                    if (taskdelaycount == 0){
                        METERMeasTable.PhaseB.PowReal = *((INT32S*)&readbuffer[2]);
                        taskstate = TASK_STATE_GET_C_PWRP;
                        cycledelay = 0;
                        taskdelaycount = 1;
                    }else{
                        // Continue
                    }
                    break;
                case TASK_STATE_GET_C_PWRP:
                    taskdelaycount = METERcommand(COMMAND_CODE_READ,DATA_LENGTH_8,ADD_C_PWRP,readbuffer);
                    if (taskdelaycount == 0){
                        METERMeasTable.PhaseC.PowReal = *((INT32S*)&readbuffer[2]);
                        taskstate = TASK_STATE_GET_A_PWRQ;
                        cycledelay = 0;
                        taskdelaycount = 1;
                    }else{
                        // Continue
                    }
                    break;
                case TASK_STATE_GET_A_PWRQ:
                    taskdelaycount = METERcommand(COMMAND_CODE_READ,DATA_LENGTH_8,ADD_A_PWRQ,readbuffer);
                    if (taskdelaycount == 0){
                        METERMeasTable.PhaseA.PowReactive = *((INT32S*)&readbuffer[2]);
                        taskstate = TASK_STATE_GET_B_PWRQ;
                        cycledelay = 0;
                        taskdelaycount = 1;
                    }else{
                        // Continue
                    }
                    break;
                case TASK_STATE_GET_B_PWRQ:
                    taskdelaycount = METERcommand(COMMAND_CODE_READ,DATA_LENGTH_8,ADD_B_PWRQ,readbuffer);
                    if (taskdelaycount == 0){
                        METERMeasTable.PhaseB.PowReactive = *((INT32S*)&readbuffer[2]);
                        taskstate = TASK_STATE_GET_C_PWRQ;
                        cycledelay = 0;
                        taskdelaycount = 1;
                    }else{
                        // Continue
                    }
                    break;
                case TASK_STATE_GET_C_PWRQ:
                    taskdelaycount = METERcommand(COMMAND_CODE_READ,DATA_LENGTH_8,ADD_C_PWRQ,readbuffer);
                    if (taskdelaycount == 0){
                        METERMeasTable.PhaseC.PowReactive = *((INT32S*)&readbuffer[2]);
                        taskstate = TASK_STATE_GET_A_PWRS;
                        cycledelay = 0;
                        taskdelaycount = 1;
                    }else{
                        // Continue
                    }
                    break;
                case TASK_STATE_GET_A_PWRS:
                    taskdelaycount = METERcommand(COMMAND_CODE_READ,DATA_LENGTH_8,ADD_A_PWRS,readbuffer);
                    if (taskdelaycount == 0){
                        METERMeasTable.PhaseA.PowApparent = *((INT32S*)&readbuffer[2]);
                        taskstate = TASK_STATE_GET_B_PWRS;
                        cycledelay = 0;
                        taskdelaycount = 1;
                    }else{
                        // Continue
                    }
                    break;
                case TASK_STATE_GET_B_PWRS:
                    taskdelaycount = METERcommand(COMMAND_CODE_READ,DATA_LENGTH_8,ADD_B_PWRS,readbuffer);
                    if (taskdelaycount == 0){
                        METERMeasTable.PhaseB.PowApparent = *((INT32S*)&readbuffer[2]);
                        taskstate = TASK_STATE_GET_C_PWRS;
                        cycledelay = 0;
                        taskdelaycount = 1;
                    }else{
                        // Continue
                    }
                    break;
                case TASK_STATE_GET_C_PWRS:
                    taskdelaycount = METERcommand(COMMAND_CODE_READ,DATA_LENGTH_8,ADD_C_PWRS,readbuffer);
                    if (taskdelaycount == 0){
                        METERMeasTable.PhaseC.PowApparent = *((INT32S*)&readbuffer[2]);
                        taskstate = TASK_STATE_GET_A_PF;
                        cycledelay = 0;
                        taskdelaycount = 1;
                    }else{
                        // Continue
                    }
                    break;
                case TASK_STATE_GET_A_PF:
                    taskdelaycount = METERcommand(COMMAND_CODE_READ,DATA_LENGTH_8,ADD_A_PF,readbuffer);
                    if (taskdelaycount == 0){
                        METERMeasTable.PhaseA.PowFactor = *((INT16S*)&readbuffer[1]);
                        taskstate = TASK_STATE_GET_B_PF;
                        cycledelay = 0;
                        taskdelaycount = 1;
                    }else{
                        // Continue
                    }
                    break;
                case TASK_STATE_GET_B_PF:
                    taskdelaycount = METERcommand(COMMAND_CODE_READ,DATA_LENGTH_8,ADD_B_PF,readbuffer);
                    if (taskdelaycount == 0){
                        METERMeasTable.PhaseB.PowFactor = *((INT16S*)&readbuffer[1]);
                        taskstate = TASK_STATE_GET_C_PF;
                        cycledelay = 0;
                        taskdelaycount = 1;
                    }else{
                        // Continue
                    }
                    break;
                case TASK_STATE_GET_C_PF:
                    taskdelaycount = METERcommand(COMMAND_CODE_READ,DATA_LENGTH_8,ADD_C_PF,readbuffer);
                    if (taskdelaycount == 0){
                        METERMeasTable.PhaseC.PowFactor = *((INT16S*)&readbuffer[1]);
                        taskstate = TASK_STATE_GET_FREQ;
                        cycledelay = 0;
                        taskdelaycount = 1;
                    }else{
                        // Continue
                    }
                    break;
                case TASK_STATE_GET_FREQ:
                    taskdelaycount = METERcommand(COMMAND_CODE_READ,DATA_LENGTH_2,ADD_LINEFR,readbuffer);
                    if (taskdelaycount == 0){
                        METERMeasTable.Total.LineFrequency = *((INT16U*)&readbuffer[1]);
                        taskstate = TASK_STATE_CALC_AVG;
                        cycledelay = 0;
                        taskdelaycount = 1;
                    }else{
                        // Continue
                    }
                    break;
                case TASK_STATE_CALC_AVG:
                    METERMeasTable.Total.PowReal = (METERMeasTable.PhaseA.PowReal + METERMeasTable.PhaseB.PowReal + METERMeasTable.PhaseC.PowReal)/3;
                    METERMeasTable.Total.PowApparent = (METERMeasTable.PhaseA.PowApparent + METERMeasTable.PhaseB.PowApparent + METERMeasTable.PhaseC.PowApparent)/3;
                    METERMeasTable.Total.PowReactive = (METERMeasTable.PhaseA.PowReactive + METERMeasTable.PhaseB.PowReactive + METERMeasTable.PhaseC.PowReactive)/3;
                    METERMeasTable.Total.PowFactor = (METERMeasTable.PhaseA.PowFactor + METERMeasTable.PhaseB.PowFactor + METERMeasTable.PhaseC.PowFactor)/3;
                    METERMeasTable.Total.CurrentRms = (METERMeasTable.PhaseA.CurrentRms + METERMeasTable.PhaseB.CurrentRms + METERMeasTable.PhaseC.CurrentRms)/3;
                    METERMeasTable.Total.VoltageRms = (METERMeasTable.PhaseA.VoltageRms + METERMeasTable.PhaseB.VoltageRms + METERMeasTable.PhaseC.VoltageRms)/3;
                    taskstate = TASK_STATE_DSPRDY_CHECK;
                    cycledelay = 0;
                    // Delay for 150 ms to slow the measurement rate
                    taskdelaycount = 150;
                    break;
                default:

                    break;
                }
            }
        }
    }
}

// Returns the number of 10's of ms to delay the task
INT8U METERcommand(COMMAND_CODE commandcode, DATA_LENGTH datalength, INT16U address, INT8U * databuffer){
    INT8U totaldata;
    INT8U currentdata = 0;
    INT8U recievedbyte;
    switch(datalength){
    case DATA_LENGTH_1:
        totaldata = 1;
        break;
    case DATA_LENGTH_2:
        totaldata = 2;
        break;
    case DATA_LENGTH_4:
        totaldata = 4;
        break;
    case DATA_LENGTH_8:
        totaldata = 8;
        break;
    default:
        return(0);
    }
    __delay_cycles(WAIT_BETWEEN_BYTES);
    MEAS_CS_LOW();
    __delay_cycles(600);
    recievedbyte = SPI_DataB0((INT8U)commandcode | (INT8U)datalength | (INT8U)((address >>8)&0xFF));
    if ( recievedbyte != 0xC1){
        //Did not receive expected return
        MEAS_CS_HIGH();
        return(35);
    }
    __delay_cycles(WAIT_BETWEEN_BYTES);
    recievedbyte = SPI_DataB0((INT8U)(address & 0xFF));
    if (recievedbyte != 0xC2){
        //Did not receive expected return
        MEAS_CS_HIGH();
        return(35);
    }
    switch(commandcode){
    case COMMAND_CODE_READ:
        //Send sync bytes, used to allow Q3180 to finish processing
        do{
            __delay_cycles(WAIT_BETWEEN_BYTES);
            recievedbyte = SPI_DataB0(COMMAND_DUMMY);
        }while (recievedbyte == COMMAND_NACK);

        if (recievedbyte != COMMAND_ACK){
            MEAS_CS_HIGH();
            return(35);
        }
        // Read in data
        while (currentdata < totaldata){
            __delay_cycles(WAIT_BETWEEN_BYTES);
            databuffer[currentdata++] = SPI_DataB0(COMMAND_DUMMY);
        }
        // If CRC was enabled it would be received here
        break;
    case COMMAND_CODE_WRITE:
        while(currentdata < totaldata){
            __delay_cycles(WAIT_BETWEEN_BYTES);
            if (SPI_DataB0(databuffer[currentdata++]) != COMMAND_ACK){
                MEAS_CS_HIGH();
                return(35);
            }
        }
        // If CRC was enabled it would be sent here
        // Sync bytes
        do{
            __delay_cycles(WAIT_BETWEEN_BYTES);
        }while ( SPI_DataB0(COMMAND_DUMMY) != COMMAND_ACK);
        break;
    default:
        MEAS_CS_HIGH();
        return(0);
    }
    MEAS_CS_HIGH();
    return(0);
}
