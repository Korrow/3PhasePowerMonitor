/********************************************************************
* io.h
* 
* Module used to handle the GPIO. This includes initialization,
* macros to read and write to the ports, and button debounce code
*  
* 
* Author: Keegan Morrow
* Date: 6/6/2013
* Project: 3 Phase Power monitor 
*  
********************************************************************/

#ifndef IO_H_
#define IO_H_

#include "standard.h"

void IOinit(void);

#define LCD_SD_CK_BIT       BIT0
#define LCD_SD_CK_POUT      P1OUT
#define LCD_SD_CK_PDIR      P1DIR
#define LCD_SD_CK_PSEL      P1SEL
#define LCD_SD_CK_HIGH()    BIT_SET(LCD_SD_CK_POUT, LCD_SD_CK_BIT)
#define LCD_SD_CK_LOW()     BIT_CLR(LCD_SD_CK_POUT, LCD_SD_CK_BIT)

#define LCD_SD_SIMO_BIT     BIT1
#define LCD_SD_SIMO_POUT    P1OUT
#define LCD_SD_SIMO_PDIR    P1DIR
#define LCD_SD_SIMO_PSEL    P1SEL
#define LCD_SD_SIMO_HIGH()  BIT_SET(LCD_SD_SIMO_POUT, LCD_SD_SIMO_BIT)
#define LCD_SD_SIMO_LOW()   BIT_CLR(LCD_SD_SIMO_POUT, LCD_SD_SIMO_BIT)

#define LCD_SD_MISO_BIT     BIT2
#define LCD_SD_MISO_POUT    P1OUT
#define LCD_SD_MISO_PDIR    P1DIR
#define LCD_SD_MISO_PSEL    P1SEL
#define LCD_SD_MISO()       (LCD_SD_MISO_POUT & LCD_SD_MISO_BIT)

#define SD_CS_BIT           BIT0
#define SD_CS_POUT          P2OUT
#define SD_CS_PDIR          P2DIR
#define SD_CS_HIGH()        BIT_SET(SD_CS_POUT, SD_CS_BIT)
#define SD_CS_LOW()         BIT_CLR(SD_CS_POUT, SD_CS_BIT)

#define LCD_P_EN_BIT        BIT1
#define LCD_P_EN_POUT       P3OUT
#define LCD_P_EN_PDIR       P3DIR
#define LCD_P_EN_HIGH()     BIT_SET(LCD_P_EN_POUT, LCD_P_EN_BIT)
#define LCD_P_EN_LOW()      BIT_CLR(LCD_P_EN_POUT, LCD_P_EN_BIT)

#define LCD_A0_BIT          BIT3
#define LCD_A0_POUT         P3OUT
#define LCD_A0_PDIR         P3DIR
#define LCD_A0_HIGH()       BIT_SET(LCD_A0_POUT, LCD_A0_BIT)
#define LCD_A0_LOW()        BIT_CLR(LCD_A0_POUT, LCD_A0_BIT)

#define LCD_RST_BIT         BIT5
#define LCD_RST_POUT        P3OUT
#define LCD_RST_PDIR        P3DIR
#define LCD_RST_HIGH()      BIT_SET(LCD_RST_POUT, LCD_RST_BIT)
#define LCD_RST_LOW()       BIT_CLR(LCD_RST_POUT, LCD_RST_BIT)

#define LCD_CS_BIT          BIT6
#define LCD_CS_POUT         P3OUT
#define LCD_CS_PDIR         P3DIR
#define LCD_CS_HIGH()       BIT_SET(LCD_CS_POUT, LCD_CS_BIT)
#define LCD_CS_LOW()        BIT_CLR(LCD_CS_POUT, LCD_CS_BIT)

#define MEAS_CK_BIT         BIT3
#define MEAS_CK_POUT        P1OUT
#define MEAS_CK_PDIR        P1DIR
#define MEAS_CK_PSEL        P1SEL
#define MEAS_CK_HIGH()      BIT_SET(MEAS_RTC_CK_POUT, MEAS_RTC_CK_BIT)
#define MEAS_CK_LOW()       BIT_CLR(MEAS_RTC_CK_POUT, MEAS_RTC_CK_BIT)

#define MEAS_SIMO_BIT       BIT4
#define MEAS_SIMO_POUT      P1OUT
#define MEAS_SIMO_PDIR      P1DIR
#define MEAS_SIMO_PSEL      P1SEL
#define MEAS_SIMO_HIGH()    BIT_SET(MEAS_RTC_SIMO_POUT, MEAS_RTC_SIMO_BIT)
#define MEAS_SIMO_LOW()     BIT_CLR(MEAS_RTC_SIMO_POUT, MEAS_RTC_SIMO_BIT)

#define MEAS_MISO_BIT       BIT5
#define MEAS_MISO_POUT      P1OUT
#define MEAS_MISO_PDIR      P1DIR
#define MEAS_MISO_PSEL      P1SEL
#define MEAS_MISO()         (MEAS_RTC_MISO_POUT & MEAS_RTC_SIMO_BIT)

#define MEAS_CS_BIT         BIT1
#define MEAS_CS_POUT        PJOUT
#define MEAS_CS_PDIR        PJDIR
#define MEAS_CS_HIGH()      BIT_SET(MEAS_CS_POUT, MEAS_CS_BIT)
#define MEAS_CS_LOW()       BIT_CLR(MEAS_CS_POUT, MEAS_CS_BIT)

#define MEAS_RST_BIT        BIT0
#define MEAS_RST_POUT       PJOUT
#define MEAS_RST_PDIR       PJDIR
#define MEAS_RST_HIGH()     BIT_SET(MEAS_CS_POUT, MEAS_CS_BIT)
#define MEAS_RST_LOW()      BIT_CLR(MEAS_CS_POUT, MEAS_CS_BIT)

#define MEAS_IRQ_BIT        BIT2
#define MEAS_IRQ_POUT       PJOUT
#define MEAS_IRQ_PDIR       PJDIR
#define MEAS_IRQ_PIN        PJIN

#define BUTTON_PORT         (P2IN)
#define BUTTON_POUT         (P2OUT)
#define BUTTON_IE           (P2IE)
#define BUTTON_PREN         (P2REN)

#define BUTTON_LEFT_BIT     (BIT7)
#define BUTTON_RIGHT_BIT    (BIT6)
#define BUTTON_UP_BIT       (BIT5)
#define BUTTON_DOWN_BIT     (BIT4)
#define BUTTON_SELECT_BIT   (BIT3)

#define BUTTON_LEFT()       (BUTTON_PORT & BUTTON_LEFT_BIT)
#define BUTTON_RIGHT()      (BUTTON_PORT & BUTTON_RIGHT_BIT)
#define BUTTON_UP()         (BUTTON_PORT & BUTTON_UP_BIT)
#define BUTTON_DOWN()       (BUTTON_PORT & BUTTON_DOWN_BIT)
#define BUTTON_SELECT()     (BUTTON_PORT & BUTTON_SELECT_BIT)

#define BUTTON_POWER_PORT   (P3IN)
#define BUTTON_POWER_PDIR   (P3DIR)
#define BUTTON_POWER_POUT   (P3OUT)
#define BUTTON_POWER_PREN   (P3REN)
#define BUTTON_POWER_IE     (P3IE)
#define BUTTON_POWER_BIT    (BIT2)

#define VREG_ON_BIT          BIT6
#define VREG_ON_POUT         P1OUT
#define VREG_ON_HIGH()       BIT_SET(VREG_ON_POUT, VREG_ON_BIT)
#define VREG_ON_LOW()        BIT_CLR(VREG_ON_POUT, VREG_ON_BIT)

INT8U IOgetButtons(void);

void IObuttonTask(void);

void IOdisableButtonInterrupts(void);
void IOenableButtonInterrupts(void);

#endif /* IO_H_ */
