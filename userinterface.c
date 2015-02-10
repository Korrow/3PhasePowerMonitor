/********************************************************************
* userinterface.c
* 
* Module used for the user interface. It has structures for all the
* menus, a task to write the menus to a display buffer, and functions
* to support these
*  
* 
* Author: Keegan Morrow
* Date: 6/6/2013
* Project: 3 Phase Power monitor 
*  
********************************************************************/

#include "standard.h"
#include "msp430f5172.h"
#include "core.h"
#include "ui_elements.h"
#include "glcd.h"
#include "fonts.h"
#include "io.h"
#include "measurement.h"
#include "rtc_calendar.h"
#include "rtc.h"
#include "meter.h"

void UiEnterStateSetTime(void);
void UiEnterStateSetLogTime(void);
void UiEnterStateSetLogInt(void);
void UiEnterStateNormal(void);

void UI_Draw_Time(INT8S page, INT8S x);
INT8S UI_DrawMeasurement(INT8S page, INT8S x, INT32S measurement, MEAS_MODE_SOURCE source, MEAS_MODE_T isline, MEAS_MODE_UNIT units);

typedef enum{
    UI_STATE_NORMAL = 0,
    UI_STATE_SET_TIME,
    UI_STATE_SET_LOG_TIME,
    UI_STATE_SET_LOG_INT,
    UI_STATE_COUNT
}UI_STATE;

typedef struct uielement{
    const INT8U *BitMap;
    const struct uielement *pFirstChildStruct;
    void (*pElementFunc)(void);
}UI_ELEMENT;

typedef struct{
    INT8S TopCoord;
    INT8S ChildCoord;
    INT8U TopLevelExpanded;
}UI_COORD;

typedef struct{
    void (*pUpFunc)(void);
    void (*pDownFunc)(void);
}UI_TIME_MENU;

const UI_TIME_MENU UiRtcTime[] = {
    {RTCincrementMonth,RTCdecrementMonth},
    {RTCincrementDay,RTCdecrementDay},
    {RTCincrementYear,RTCdecrementYear},
    {RTCincrementHour,RTCdecrementHour},
    {RTCincrementMinute,RTCdecrementMinute},
    {RTCincrementSecond,RTCdecrementSecond},
};

UI_COORD MenuNavState={0,-1,0};

UI_STATE UiState = UI_STATE_NORMAL;

#define CHILD_COORD_MAX 3

const UI_ELEMENT SetupMenu[] = {
    {uppermenutime_glcd_bmp,(UI_ELEMENT*)0x00,(void*)UiEnterStateSetTime},
    {uppermenudelta_glcd_bmp,(UI_ELEMENT*)0x00,MEAS_SetSysMode_Delta},
    {uppermenuwye_glcd_bmp,(UI_ELEMENT*)0x00,MEAS_SetSysMode_Wye},
    {uppermenuwye_n_glcd_bmp,(UI_ELEMENT*)0x00,MEAS_SetSysMode_WyeN},
    {(const INT8U *)0x00,(UI_ELEMENT*)0x00,(void*)0x00}
};

const UI_ELEMENT MeasMenu[] = {
    {uppermenuvolts_glcd_bmp,(UI_ELEMENT*)0x00,MEAS_SetMeasMode_VPhase},
    {uppermenuamps_glcd_bmp,(UI_ELEMENT*)0x00,MEAS_SetMeasMode_APhase},
    {uppermenufreq_glcd_bmp,(UI_ELEMENT*)0x00,MEAS_SetMeasMode_LineFrequency},
    {(const INT8U *)0x00,(UI_ELEMENT*)0x00,(void*)0x00}
};

const UI_ELEMENT LogMenu[] = {
    {uppermenutime_glcd_bmp,(UI_ELEMENT*)0x00,(void*)0x00},
    {uppermenuinterval_glcd_bmp,(UI_ELEMENT*)0x00,(void*)0x00},
    {uppermenustart_glcd_bmp,(UI_ELEMENT*)0x00,(void*)0x00},
    {(const INT8U *)0x00,(UI_ELEMENT*)0x00,(void*)0x00}
};

const UI_ELEMENT PowerMenu[] = {
    {uppermenuactive_glcd_bmp,(UI_ELEMENT*)0x00,MEAS_SetMeasMode_PActive},
    {uppermenureactive_glcd_bmp,(UI_ELEMENT*)0x00,MEAS_SetMeasMode_PReactive},
    {uppermenuapparent_glcd_bmp,(UI_ELEMENT*)0x00,MEAS_SetMeasMode_PApparent},
    {uppermenupf_glcd_bmp,(UI_ELEMENT*)0x00,(void*)0x00},
    {(const INT8U *)0x00,(UI_ELEMENT*)0x00,(void*)0x00}
};

const UI_ELEMENT TopLevel[] = {
    {uppermenusetup_glcd_bmp,SetupMenu,(void*)0x00},
    {uppermenumeas_glcd_bmp,MeasMenu,(void*)0x00},
    {uppermenupower_glcd_bmp,PowerMenu,(void*)0x00},
    {uppermenulog_glcd_bmp,LogMenu,(void*)0x00},
    {uppermenualt_glcd_bmp,(void*)0x00,(void*)0x00},//Add a function for this one
    {(const INT8U *)0x00,(void*)0x00,(void*)0x00}
};

INT16U testglobal = 0;

void TestFunc(void){
    testglobal++;
}

void UIinit(void){
    MenuNavState.ChildCoord = -1;
    MenuNavState.TopCoord = 0;
    MenuNavState.TopLevelExpanded = 0;
}

void UI_Draw(void){
    INT8U topindex =0;
    INT8U childindex=0;
    INT8U xloc = 0;
    INT8U invert = 0;
    const INT8U *pBitmap;
    static INT8U turncounter = 0;
    static INT8U lastbuttonpressed = 0;

    if ( COREgetState() == CORE_STATE_METER){
        if ( turncounter >= 3 ){
            turncounter = 0;
            glcd_reset_buffer();
            switch(UiState){
            case UI_STATE_NORMAL:
                if (IOgetButtons()){
                    lastbuttonpressed = IOgetButtons();
                }else{
                    if (lastbuttonpressed & BUTTON_LEFT_BIT){
                        if (MenuNavState.TopCoord > 0){
                            MenuNavState.TopLevelExpanded = 0;
                            MenuNavState.ChildCoord = -1;
                            MenuNavState.TopCoord--;
                        }else{
                            // Ignore the button press
                        }
                    }
                    else if (lastbuttonpressed & BUTTON_RIGHT_BIT){
                        if (MenuNavState.TopCoord < 4){
                            MenuNavState.TopLevelExpanded = 0;
                            MenuNavState.ChildCoord = -1;
                            MenuNavState.TopCoord++;
                        }else{
                            // Ignore the button press
                        }
                    }
                    else if (lastbuttonpressed & BUTTON_UP_BIT){
                        if (MenuNavState.ChildCoord > -1){
                            MenuNavState.ChildCoord--;
                        }else{
                            // Ignore the button press
                        }
                    }
                    else if (lastbuttonpressed & BUTTON_DOWN_BIT){
                        if ((TopLevel[MenuNavState.TopCoord].pFirstChildStruct[MenuNavState.ChildCoord+1].BitMap != 0x00)&&(MenuNavState.TopLevelExpanded ==1)){
                            MenuNavState.ChildCoord++;
                        }else{
                            // Ignore the button press
                        }
                    }
                    else if (lastbuttonpressed & BUTTON_SELECT_BIT){
                        if (MenuNavState.ChildCoord == -1){
                            if (TopLevel[MenuNavState.TopCoord].pElementFunc != 0x00){
                                TopLevel[MenuNavState.TopCoord].pElementFunc();
                            }else{
                                // Don't call a function
                            }
                            if (MenuNavState.TopLevelExpanded == 0 && TopLevel[MenuNavState.TopCoord].pFirstChildStruct != 0x00){
                                MenuNavState.TopLevelExpanded = 1;
                            }else{

                                MenuNavState.TopLevelExpanded = 0;
                            }
                        }else{
                            if (TopLevel[MenuNavState.TopCoord].pFirstChildStruct[MenuNavState.ChildCoord].pElementFunc != 0x00){
                                TopLevel[MenuNavState.TopCoord].pFirstChildStruct[MenuNavState.ChildCoord].pElementFunc();
                            }else{
                                // Don't call a function
                            }
                            // Collapse the menu
                            MenuNavState.ChildCoord = -1;
                            MenuNavState.TopLevelExpanded = 0;
                        }
                    }
                    lastbuttonpressed = 0;
                }
                switch(MEAS_MeasMode){
                case MEAS_MODE_VOLTS_PHASE:
                case MEAS_MODE_VOLTS_LINE:
                    UI_DrawMeasurement(2, 6, METERMeasTable.PhaseA.VoltageRms, MEAS_MODE_SOURCE_1, MEAS_MODE_T_PHASE, MEAS_MODE_UNIT_VOLTS);
                    UI_DrawMeasurement(2, 67, METERMeasTable.PhaseB.VoltageRms, MEAS_MODE_SOURCE_2, MEAS_MODE_T_PHASE, MEAS_MODE_UNIT_VOLTS);
                    UI_DrawMeasurement(4, 6, METERMeasTable.PhaseC.VoltageRms, MEAS_MODE_SOURCE_3, MEAS_MODE_T_PHASE, MEAS_MODE_UNIT_VOLTS);
                    UI_DrawMeasurement(4, 67, METERMeasTable.Total.VoltageRms, MEAS_MODE_SOURCE_AVG, MEAS_MODE_T_PHASE, MEAS_MODE_UNIT_VOLTS);
                    break;
                case MEAS_MODE_AMPS_PHASE:
                case MEAS_MODE_AMPS_LINE:
                    UI_DrawMeasurement(2, 6, METERMeasTable.PhaseA.CurrentRms, MEAS_MODE_SOURCE_1, MEAS_MODE_T_PHASE, MEAS_MODE_UNIT_AMPS);
                    UI_DrawMeasurement(2, 67, METERMeasTable.PhaseB.CurrentRms, MEAS_MODE_SOURCE_2, MEAS_MODE_T_PHASE, MEAS_MODE_UNIT_AMPS);
                    UI_DrawMeasurement(4, 6, METERMeasTable.PhaseC.CurrentRms, MEAS_MODE_SOURCE_3, MEAS_MODE_T_PHASE, MEAS_MODE_UNIT_AMPS);
                    UI_DrawMeasurement(4, 67, METERMeasTable.Total.CurrentRms, MEAS_MODE_SOURCE_AVG, MEAS_MODE_T_PHASE, MEAS_MODE_UNIT_AMPS);
                    break;
                case MEAS_MODE_POWER_ACTIVE:
                    UI_DrawMeasurement(2, 6, METERMeasTable.PhaseA.PowReal, MEAS_MODE_SOURCE_1, MEAS_MODE_T_PHASE, MEAS_MODE_UNIT_W);
                    UI_DrawMeasurement(2, 67, METERMeasTable.PhaseB.PowReal, MEAS_MODE_SOURCE_2, MEAS_MODE_T_PHASE, MEAS_MODE_UNIT_W);
                    UI_DrawMeasurement(4, 6, METERMeasTable.PhaseC.PowReal, MEAS_MODE_SOURCE_3, MEAS_MODE_T_PHASE, MEAS_MODE_UNIT_W);
                    UI_DrawMeasurement(4, 67, METERMeasTable.Total.PowReal, MEAS_MODE_SOURCE_AVG, MEAS_MODE_T_PHASE, MEAS_MODE_UNIT_W);
                    break;
                case MEAS_MODE_POWER_REACTIVE:
                    UI_DrawMeasurement(2, 6, METERMeasTable.PhaseA.PowReactive, MEAS_MODE_SOURCE_1, MEAS_MODE_T_PHASE, MEAS_MODE_UNIT_VAR);
                    UI_DrawMeasurement(2, 67, METERMeasTable.PhaseB.PowReactive, MEAS_MODE_SOURCE_2, MEAS_MODE_T_PHASE, MEAS_MODE_UNIT_VAR);
                    UI_DrawMeasurement(4, 6, METERMeasTable.PhaseC.PowReactive, MEAS_MODE_SOURCE_3, MEAS_MODE_T_PHASE, MEAS_MODE_UNIT_VAR);
                    UI_DrawMeasurement(4, 67, METERMeasTable.Total.PowReactive, MEAS_MODE_SOURCE_AVG, MEAS_MODE_T_PHASE, MEAS_MODE_UNIT_VAR);
                    break;
                case MEAS_MODE_POWER_APPARENT:
                    UI_DrawMeasurement(2, 6, METERMeasTable.PhaseA.PowApparent, MEAS_MODE_SOURCE_1, MEAS_MODE_T_PHASE, MEAS_MODE_UNIT_VA);
                    UI_DrawMeasurement(2, 67, METERMeasTable.PhaseB.PowApparent, MEAS_MODE_SOURCE_2, MEAS_MODE_T_PHASE, MEAS_MODE_UNIT_VA);
                    UI_DrawMeasurement(4, 6, METERMeasTable.PhaseC.PowApparent, MEAS_MODE_SOURCE_3, MEAS_MODE_T_PHASE, MEAS_MODE_UNIT_VA);
                    UI_DrawMeasurement(4, 67, METERMeasTable.Total.PowApparent, MEAS_MODE_SOURCE_AVG, MEAS_MODE_T_PHASE, MEAS_MODE_UNIT_VA);
                    break;
                case MEAS_MODE_LINE_FREQUENCY:
                    UI_DrawMeasurement(2, 6, METERMeasTable.Total.LineFrequency, MEAS_MODE_SOURCE_TOTAL, MEAS_MODE_T_PHASE, MEAS_MODE_UNIT_HZ);
                default:
                    // Draw nothing
                    break;
                }
                // Draw the top menu bar
                xloc = 0;
                while(TopLevel[topindex].BitMap != 0x00){
                    if((MenuNavState.TopCoord == topindex)&&(MenuNavState.ChildCoord == -1)){
                        invert = GLCD_MENU_INVERT;
                    }else{
                        invert = GLCD_MENU_NORMAL;
                    }
                    glcd_draw_menu_item(0, xloc,(TopLevel[topindex].BitMap+1),26,invert);
                    //Implement using recursive functions later?
                    childindex = 0;
                    if (MenuNavState.TopLevelExpanded && (MenuNavState.TopCoord == topindex)){
                        while(TopLevel[topindex].pFirstChildStruct[childindex].BitMap != 0x00){
                            if((MenuNavState.TopCoord == topindex)&&(MenuNavState.ChildCoord == childindex)){
                                invert = GLCD_MENU_INVERT;
                            }else{
                                invert = GLCD_MENU_NORMAL;
                            }
                            glcd_draw_menu_item((childindex+1), (xloc-1),(TopLevel[topindex].pFirstChildStruct[childindex].BitMap),27,invert);
                            childindex++;
                        }
                    }
                    topindex++;
                    xloc +=26;
                }
                xloc = 0;
                //Draw the lower status bar
                xloc = glcd_draw_menu_item(7, xloc, lowerbarsys_glcd_bmp, 16, GLCD_MENU_TOPBORDER);
                switch(MEAS_SysMode){
                case SYS_MODE_DELTA:
                    pBitmap = lowerbarsysdelta_glcd_bmp;
                    break;
                case SYS_MODE_WYE:
                    pBitmap = lowerbarsyswye_glcd_bmp;
                    break;
                case SYS_MODE_WYE_N:
                    pBitmap = lowerbarsyswyen_glcd_bmp;
                    break;
                default:
                    pBitmap = lowerbarsysdelta_glcd_bmp;
                    break;
                }
                xloc = glcd_draw_menu_item(7, xloc,pBitmap, 7,GLCD_MENU_TOPBORDER);
                xloc = glcd_draw_menu_item(7,xloc,lowerbarmeas_glcd_bmp, 20, GLCD_MENU_TOPBORDER);
                xloc = glcd_draw_menu_item(7,xloc,lowerbarmeasv_glcd_bmp, 4, GLCD_MENU_TOPBORDER);
                xloc = glcd_draw_menu_item(7,xloc,lowerbarmeasphase_glcd_bmp, 19, GLCD_MENU_TOPBORDER);

                UI_Draw_Time(7,xloc);
                break;
            case UI_STATE_SET_TIME:
                if (IOgetButtons()){
                    lastbuttonpressed = IOgetButtons();
                }else{
                    if (lastbuttonpressed & BUTTON_LEFT_BIT){
                        if (MenuNavState.TopCoord > 0){
                            MenuNavState.TopCoord--;
                        }else{
                            // Ignore the button press
                        }
                    }else if (lastbuttonpressed & BUTTON_RIGHT_BIT){
                        if (MenuNavState.TopCoord < 5){
                            MenuNavState.TopCoord++;
                        }else{
                            // Ignore the button press
                        }
                    }else if (lastbuttonpressed & BUTTON_UP_BIT){
                        if (UiRtcTime[MenuNavState.TopCoord].pUpFunc != 0x00){
                            UiRtcTime[MenuNavState.TopCoord].pUpFunc();
                        }else{
                            // Ignore the button press
                        }
                    }else if (lastbuttonpressed & BUTTON_DOWN_BIT){
                        if (UiRtcTime[MenuNavState.TopCoord].pDownFunc != 0x00){
                            UiRtcTime[MenuNavState.TopCoord].pDownFunc();
                        }else{
                            // Ignore the button press
                        }
                    }else if (lastbuttonpressed & BUTTON_SELECT_BIT){
                        UiEnterStateNormal();
                    }else{
                        // Unhandled keypress, do nothing
                    }
                    lastbuttonpressed = 0;
                }
                if (MenuNavState.TopCoord == 0){
                    invert = GLCD_MENU_INVERT;
                }else{
                    invert = GLCD_MENU_NORMAL;
                }
                xloc = glcd_draw_int(3,8,1+RTCgetMonth(), 2,invert,0, MeasFontTable, MEAS_FONT_WIDTH);
                xloc = glcd_draw_menu_item(3, xloc, fnt_wendy_slash_glcd_bmp, 3, GLCD_MENU_NORMAL);
                xloc = glcd_draw_menu_item(3, xloc, background_glcd_bmp, 1, GLCD_MENU_NORMAL);
                if (MenuNavState.TopCoord == 1){
                    invert = GLCD_MENU_INVERT;
                }else{
                    invert = GLCD_MENU_NORMAL;
                }
                xloc = glcd_draw_int(3,xloc,RTCgetDay(), 2,invert,0, MeasFontTable, MEAS_FONT_WIDTH);
                xloc = glcd_draw_menu_item(3, xloc, fnt_wendy_slash_glcd_bmp, 3, GLCD_MENU_NORMAL);
                xloc = glcd_draw_menu_item(3, xloc, background_glcd_bmp, 1, GLCD_MENU_NORMAL);
                if (MenuNavState.TopCoord == 2){
                    invert = GLCD_MENU_INVERT;
                }else{
                    invert = GLCD_MENU_NORMAL;
                }
                xloc = glcd_draw_int(3,xloc,RTCgetYear(), 2,invert,0, MeasFontTable, MEAS_FONT_WIDTH);
                xloc = glcd_draw_menu_item(3, xloc, background_glcd_bmp, 3, GLCD_MENU_NORMAL);
                xloc = glcd_draw_menu_item(3, xloc, background_glcd_bmp, 3, GLCD_MENU_NORMAL);
                if (MenuNavState.TopCoord == 3){
                    invert = GLCD_MENU_INVERT;
                }else{
                    invert = GLCD_MENU_NORMAL;
                }
                xloc = glcd_draw_int(3,xloc,RTCgetHour(), 2,invert,0, MeasFontTable, MEAS_FONT_WIDTH) - 1;
                xloc = glcd_draw_menu_item(3, xloc, fnt_wendy_colon_glcd_bmp, 3, GLCD_MENU_NORMAL);
                if (MenuNavState.TopCoord == 4){
                    invert = GLCD_MENU_INVERT;
                }else{
                    invert = GLCD_MENU_NORMAL;
                }
                xloc = glcd_draw_int(3,xloc,RTCgetMinute(), 2,invert,0, MeasFontTable, MEAS_FONT_WIDTH);;
                xloc = glcd_draw_menu_item(3, xloc, fnt_wendy_colon_glcd_bmp, 3, GLCD_MENU_NORMAL);
                if (MenuNavState.TopCoord == 5){
                    invert = GLCD_MENU_INVERT;
                }else{
                    invert = GLCD_MENU_NORMAL;
                }
                xloc = glcd_draw_int(3,xloc,RTCgetSecond(), 2,invert,0, MeasFontTable, MEAS_FONT_WIDTH);;
                break;
            case UI_STATE_SET_LOG_TIME:

                break;
            case UI_STATE_SET_LOG_INT:
                break;
            default:
                // Error
                break;
            }
        }else{
            turncounter++;
        }
    }else if ( COREgetState() == CORE_STATE_LOG_SLEEP ){
        if (IOgetButtons()){
            COREsetState(CORE_STATE_METER);
        }else{
            // Do nothing
        }
    }
}

// Revise this for more consistant naming
void UI_Draw_Time(INT8S page, INT8S x){
    INT8S currx = x;
    //Draw the date first
    currx = glcd_draw_int(page,currx,1+RTCgetMonth(), 2,GLCD_MENU_TOPBORDER,0, WendyFontTable, WENDY_FONT_WIDTH);

    currx = glcd_draw_menu_item(page, currx, fnt_wendy_slash_glcd_bmp, 3, GLCD_MENU_TOPBORDER);
    currx = glcd_draw_menu_item(page, currx, background_glcd_bmp, 1, GLCD_MENU_TOPBORDER);
    currx = glcd_draw_int(page,currx,RTCgetDay(), 2,GLCD_MENU_TOPBORDER,0, WendyFontTable, WENDY_FONT_WIDTH);
    currx = glcd_draw_menu_item(page, currx, fnt_wendy_slash_glcd_bmp, 3, GLCD_MENU_TOPBORDER);
    currx = glcd_draw_menu_item(page, currx, background_glcd_bmp, 1, GLCD_MENU_TOPBORDER);
    currx = glcd_draw_int(page,currx,RTCgetYear(), 2,GLCD_MENU_TOPBORDER,0, WendyFontTable, WENDY_FONT_WIDTH);
    currx = glcd_draw_menu_item(page, currx, background_glcd_bmp, 1, GLCD_MENU_TOPBORDER);
    currx = currx = glcd_draw_int(page,currx,RTCgetHour(), 2,GLCD_MENU_TOPBORDER,0, WendyFontTable, WENDY_FONT_WIDTH) - 1;
    currx = glcd_draw_menu_item(page, currx, fnt_wendy_colon_glcd_bmp, 3, GLCD_MENU_TOPBORDER);
    currx = glcd_draw_int(page,currx,RTCgetMinute(), 2,GLCD_MENU_TOPBORDER,0, WendyFontTable, WENDY_FONT_WIDTH);;
    currx = glcd_draw_menu_item(page, currx, fnt_wendy_colon_glcd_bmp, 3, GLCD_MENU_TOPBORDER);
    currx = glcd_draw_int(page,currx,RTCgetSecond(), 2,GLCD_MENU_TOPBORDER,0, WendyFontTable, WENDY_FONT_WIDTH);;

}

INT8S UI_DrawMeasurement(INT8S page, INT8S x, INT32S measurement, MEAS_MODE_SOURCE source, MEAS_MODE_T isline, MEAS_MODE_UNIT units){
    INT8S currx = x;
    const INT8U *pBitmap;
    switch(isline){
    case MEAS_MODE_T_LINE:
        pBitmap = font_8px_l_glcd_bmp;
        break;
    case MEAS_MODE_T_PHASE:
        pBitmap = font_8px_p_glcd_bmp;
        break;
    case MEAS_MODE_T_POWER:
        pBitmap = font_8px_p_glcd_bmp;
        break;
    default:
        pBitmap = font_8px_p_glcd_bmp;
    }
    currx = glcd_draw_menu_item(page, currx,pBitmap,MEAS_FONT_WIDTH, 0);
    switch(source){
    case MEAS_MODE_SOURCE_1:
        currx = glcd_draw_digit(page, currx, 1, 0, MeasFontTable, MEAS_FONT_WIDTH);
        break;
    case MEAS_MODE_SOURCE_2:
        currx = glcd_draw_digit(page, currx, 2, 0, MeasFontTable, MEAS_FONT_WIDTH);
        break;
    case MEAS_MODE_SOURCE_3:
        currx = glcd_draw_digit(page, currx, 3, 0, MeasFontTable, MEAS_FONT_WIDTH);
        break;
    case MEAS_MODE_SOURCE_N:
        //Placeholder
        break;
    case MEAS_MODE_SOURCE_AB:
        //Placeholder
        break;
    case MEAS_MODE_SOURCE_BC:
        //Placeholder
        break;
    case MEAS_MODE_SOURCE_CA:
        //Placeholder
        break;
    case MEAS_MODE_SOURCE_AVG:
        currx = glcd_draw_menu_item(page, currx,font_8px_a_glcd_bmp,MEAS_FONT_WIDTH, 0);
        break;
    case MEAS_MODE_SOURCE_TOTAL:
        currx = glcd_draw_menu_item(page, currx,font_8px_t_glcd_bmp,MEAS_FONT_WIDTH, 0);
        break;
    }

    currx = glcd_draw_menu_item(page, currx,font_8px_colon_glcd_bmp,MEAS_FONT_WIDTH, 0);
    if ( measurement < 0 ){
        currx = glcd_draw_menu_item(page, currx,font_8px_dash_glcd_bmp,MEAS_FONT_WIDTH, 0);
        measurement = 0 - measurement;
    }

    switch(units){
    case MEAS_MODE_UNIT_VOLTS:
        currx = glcd_draw_int(page,currx,measurement, 1,0,3, MeasFontTable, MEAS_FONT_WIDTH);
        currx = glcd_draw_menu_item(page, currx,font_8px_v_glcd_bmp,MEAS_FONT_WIDTH, 0);
        break;
    case MEAS_MODE_UNIT_AMPS:
        currx = glcd_draw_int(page,currx,measurement, 1,0,3, MeasFontTable, MEAS_FONT_WIDTH);
        currx = glcd_draw_menu_item(page, currx,font_8px_a_glcd_bmp,MEAS_FONT_WIDTH, 0);
        break;
    case MEAS_MODE_UNIT_VA:
        currx = glcd_draw_int(page,currx,measurement, 1,0,2, MeasFontTable, MEAS_FONT_WIDTH);
        currx = glcd_draw_menu_item(page, currx,font_8px_v_glcd_bmp,MEAS_FONT_WIDTH, 0);
        currx = glcd_draw_menu_item(page, currx,font_8px_a_glcd_bmp,MEAS_FONT_WIDTH, 0);
        break;
    case MEAS_MODE_UNIT_W:
        currx = glcd_draw_int(page,currx,measurement, 1,0,2, MeasFontTable, MEAS_FONT_WIDTH);
        currx = glcd_draw_menu_item(page, currx,font_8px_w_glcd_bmp,MEAS_FONT_WIDTH, 0);
        break;
    case MEAS_MODE_UNIT_VAR:
        currx = glcd_draw_int(page,currx,measurement, 1,0,2, MeasFontTable, MEAS_FONT_WIDTH);
        currx = glcd_draw_menu_item(page, currx,font_8px_v_glcd_bmp,MEAS_FONT_WIDTH, 0);
        currx = glcd_draw_menu_item(page, currx,font_8px_a_glcd_bmp,MEAS_FONT_WIDTH, 0);
        currx = glcd_draw_menu_item(page, currx,font_8px_r_glcd_bmp,MEAS_FONT_WIDTH, 0);
        break;
    case MEAS_MODE_UNIT_HZ:
        currx = glcd_draw_int(page,currx,measurement, 1,0,3, MeasFontTable, MEAS_FONT_WIDTH);
        currx = glcd_draw_menu_item(page, currx,font_8px_h_glcd_bmp,MEAS_FONT_WIDTH, 0);
        currx = glcd_draw_menu_item(page, currx,font_8px_z_glcd_bmp,MEAS_FONT_WIDTH, 0);
    default:
        // No Units
    }
    return(currx);
}

void UiEnterStateSetTime(void){
    MenuNavState.ChildCoord = 0;
    MenuNavState.TopCoord = 0;
    MenuNavState.TopLevelExpanded = 0;
    UiState = UI_STATE_SET_TIME;
}
void UiEnterStateSetLogTime(void){
    MenuNavState.ChildCoord = 0;
    MenuNavState.TopCoord = 0;
    MenuNavState.TopLevelExpanded = 0;
    UiState = UI_STATE_SET_LOG_TIME;
}
void UiEnterStateSetLogInt(void){
    MenuNavState.ChildCoord = 0;
    MenuNavState.TopCoord = 0;
    MenuNavState.TopLevelExpanded = 0;
    UiState = UI_STATE_SET_LOG_INT;
}
void UiEnterStateNormal(void){
    MenuNavState.ChildCoord = -1;
    MenuNavState.TopCoord = 0;
    MenuNavState.TopLevelExpanded = 0;
    UiState = UI_STATE_NORMAL;
}
