/********************************************************************
* glcd.c
* 
* Module used for communication with the ST7565 graphical LCD.
* Contains functions for initialization, and refreshing
* as well as functions for drawing bitmaps to the LCD buffer
*  
* 
* Author: David Cannings
* Modified by: Keegan Morrow
* Date: 6/6/2013
* Project: 3 Phase Power monitor 
*  
********************************************************************/
#include "msp430f5172.h"
#include "standard.h"
#include "glcd-config.h"
#include "glcd.h"
#include "delay.h"
#include "spi.h"
#include "io.h"
#include "ui_elements.h"
//#define ST_BIT_BANG

/** Global variable that tracks whether the screen is the "normal" way up. */
unsigned char glcd_flipped;

int pagemap[] = { 4, 5, 6, 7, 0, 1, 2, 3 };

/** Global buffer to hold the current screen contents. */
// This has to be kept here because the width & height are set in
// st7565-config.h
unsigned char glcd_buffer[SCREEN_WIDTH * SCREEN_HEIGHT / 8] = {
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,

0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,

0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,

0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,

0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,

0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,

0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,

0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,

};


#ifdef ST7565_DIRTY_PAGES
unsigned char glcd_dirty_pages;
#endif

void glcd_pixel(unsigned char x, unsigned char y, unsigned char colour) {

    if (x > SCREEN_WIDTH || y > SCREEN_HEIGHT) return;

    // Real screen coordinates are 0-63, not 1-64.
    x -= 1;
    y -= 1;

    unsigned short array_pos = x + ((y / 8) * 128);

#ifdef ST7565_DIRTY_PAGES
#error ** ST7565_DIRTY_PAGES enabled, only changed pages will be written to the GLCD **
    glcd_dirty_pages |= 1 << (array_pos / 128);
#endif

    if (colour) {
        glcd_buffer[array_pos] |= 1 << (y % 8);
    } else {
        glcd_buffer[array_pos] &= 0xFF ^ 1 << (y % 8);
    }
}

void glcd_blank(void) {
    int n;
    int y;
    int x;
    // Reset the internal buffer
    for (n = 1; n <= (SCREEN_WIDTH * SCREEN_HEIGHT / 8) - 1; n++) {
        glcd_buffer[n] = 0;
    }

    // Clear the actual screen
    for (y = 0; y < 8; y++) {
        glcd_command(GLCD_CMD_SET_PAGE | y);

        // Reset column to 0 (the left side)
        glcd_command(GLCD_CMD_COLUMN_LOWER);
        glcd_command(GLCD_CMD_COLUMN_UPPER);

        // We iterate to 132 as the internal buffer is 65*132, not
        // 64*124.
        for (x = 0; x < 132; x++) {
            glcd_data(0x00);
        }
    }
}

void glcd_refresh(void) {
    int y;
    int x;
    for (y = 0; y < 8; y++) {

#ifdef ST7565_DIRTY_PAGES
        // Only copy this page if it is marked as "dirty"
        if (!(glcd_dirty_pages & (1 << y))){
            continue;
        }
#endif

        glcd_command(GLCD_CMD_SET_PAGE | pagemap[y]);

        // Reset column to the left side.  The internal memory of the
        // screen is 132*64, we need to account for this if the display
        // is flipped.
        //
        // Some screens seem to map the internal memory to the screen
        // pixels differently, the ST7565_REVERSE define allows this to
        // be controlled if necessary.
        glcd_command(GLCD_CMD_COLUMN_LOWER | 1);
        glcd_command(GLCD_CMD_COLUMN_UPPER);

        for (x = 0; x < 128; x++) {
            glcd_data(glcd_buffer[y * 128 + x ]);
        }
    }

#ifdef ST7565_DIRTY_PAGES
    // All pages have now been updated, reset the indicator.
    glcd_dirty_pages = 0;
#endif
}

void glcd_update_task(void){
    static INT8U turncounter = 0;
    if (turncounter >= 3){
        glcd_refresh();
        turncounter = 0;
    }else{
        turncounter++;
    }
}

void glcd_init(void) {
    glcd_flipped = 0;

    // Power the LCD
    BIT_SET(LCD_P_EN_POUT,LCD_P_EN_BIT);

    // Select the chip
    LCD_CS_LOW();

    LCD_RST_LOW();

    // Datasheet says "wait for power to stabilise" but gives
    // no specific time!
    DelayMs(50);

    LCD_RST_HIGH();

    // Datasheet says max 1ms here
    //DelayMs(1);

    // Set LCD bias to 1/9th
    glcd_command(GLCD_CMD_BIAS_9);

    // Horizontal output direction (ADC segment driver selection)
    glcd_command(GLCD_CMD_HORIZONTAL_NORMAL);

    // Vertical output direction (common output mode selection)
    glcd_command(GLCD_CMD_VERTICAL_REVERSE);

    // Reset start position to the top
    glcd_command(GLCD_CMD_DISPLAY_START);

    // The screen is the "normal" way up
    glcd_flipped = 0;

    // Set internal resistor.  A suitable middle value is used as
    // the default.
    // Changed this to match adafruit settings
    glcd_command(GLCD_CMD_RESISTOR | 0x6);

    // Power control setting (datasheet step 7)
    // Note: Skipping straight to 0x7 works with my hardware.
    glcd_command(GLCD_CMD_POWER_CONTROL | 0x4);
    DelayMs(50);
    glcd_command(GLCD_CMD_POWER_CONTROL | 0x6);
    DelayMs(50);
    glcd_command(GLCD_CMD_POWER_CONTROL | 0x7);
    DelayMs(10);

    // Volume set (brightness control).  A middle value is used here
    // also.
    glcd_command(GLCD_CMD_VOLUME_MODE);
    glcd_command(40);

    // Turn the display on
    glcd_command(GLCD_CMD_DISPLAY_ON);
    // Unselect the chip
    LCD_CS_HIGH();
}

void glcd_shutdown(void){
    // Power the LCD
    BIT_CLR(LCD_P_EN_POUT,LCD_P_EN_BIT);
    LCD_RST_LOW();
}

void glcd_data(unsigned char data) {

    int n;
    // A0 is high for display data
    LCD_A0_HIGH();

    // Select the chip
    LCD_CS_LOW();

#ifdef ST_BIT_BANG
    for (n = 0; n < 8; n++) {

        if (data & 0x80) {
            LCD_SD_SIMO_HIGH();
        } else {
            LCD_SD_SIMO_LOW();
        }

        // Pulse SCL
        LCD_SD_CK_HIGH();
        LCD_SD_CK_LOW();

        data <<= 1;
    }
#else
    (void)SPI_DataA0(data);
#endif //ST_BIT_BANG
    // Put A0 back low
    LCD_A0_LOW();
    // Unselect the chip
    LCD_CS_HIGH();
}


void glcd_command(char command) {

    int n;
    // A0 is low for command data
    LCD_A0_LOW();

    // Select the chip
    LCD_CS_LOW();

#ifdef ST_BIT_BANG
    for (n = 0; n < 8; n++) {

        if (command & 0x80) {
            LCD_SD_SIMO_HIGH();
        } else {
            LCD_SD_SIMO_LOW();
        }

        // Pulse SCL
        LCD_SD_CK_HIGH();
        LCD_SD_CK_LOW();

        command <<= 1;
    }
#else
    (void)SPI_DataA0(command);
#endif //ST_BIT_BANG

    // Unselect the chip
    LCD_CS_HIGH();
}

void glcd_flip_screen(unsigned char flip) {
    if (flip) {
        glcd_command(GLCD_CMD_HORIZONTAL_NORMAL);
        glcd_command(GLCD_CMD_VERTICAL_REVERSE);
        glcd_flipped = 0;
    } else {
        glcd_command(GLCD_CMD_HORIZONTAL_REVERSE);
        glcd_command(GLCD_CMD_VERTICAL_NORMAL);
        glcd_flipped = 1;
    }
}

void glcd_inverse_screen(unsigned char inverse) {
    if (inverse) {
        glcd_command(GLCD_CMD_DISPLAY_REVERSE);
    } else {
        glcd_command(GLCD_CMD_DISPLAY_NORMAL);
    }
}

void glcd_test_card(void) {
    unsigned char p = 0xF0;
    unsigned char q;
    int n;

    for (n = 1; n <= (SCREEN_WIDTH * SCREEN_HEIGHT / 8); n++) {
        glcd_buffer[n - 1] = p;

        if (n % 4 == 0) {
            q = p;
            p = p << 4;
            p |= q >> 4;
        }
    }

    glcd_refresh();
}

void glcd_contrast(char resistor_ratio, char contrast) {
    if (resistor_ratio > 7 || contrast > 63) return;

    glcd_command(GLCD_CMD_RESISTOR | resistor_ratio);
    glcd_command(GLCD_CMD_VOLUME_MODE);
    glcd_command(contrast);
}

INT8U glcd_draw_menu_item(INT8U page, INT8S x,const INT8U *data,INT8U size, INT8U options){
    INT8U currx = 0;
    //glcd_buffer[(SCREEN_WIDTH*page)+x]=0xFF;
    if (x < 0){
        data -=x;
        x = 0;
    }
    while((currx < size)&&((x+currx)<SCREEN_WIDTH)){
            glcd_buffer[(SCREEN_WIDTH*page)+(x+currx)] = data[currx];
            if ((options & GLCD_MENU_INVERT) && data[currx]!=0xFF){
                glcd_buffer[(SCREEN_WIDTH*page)+(x+currx)] ^= 0x7F;
            }else{
                // Do Nothing
            }
            if ((options & GLCD_MENU_TOPBORDER)){
                glcd_buffer[(SCREEN_WIDTH*page)+(x+currx)] |= 0x01;
            }
        currx++;
    }
    return (currx+x);
}


INT8S glcd_draw_int(INT8U page,INT8S x,INT32U num,INT8U minpredec, INT8U options,INT8U decimals, const INT8U *fnt[], INT8U width){
    INT8S currx = x;
    INT8U digitarray[10];
    INT8S index = 0;
    INT32U currnum = num;
    INT8U pastzeroes = 0;
    while ( index < 10){
        digitarray[index] = currnum%10;
        currnum /=10;
        index++;
    }
    index--;\
    while ( index >= 0){
        if (digitarray[index] != 0 ){
            pastzeroes = 1;
        }else{
            //Nothing changes
        }
        if (pastzeroes|| ((minpredec+decimals) > index)){
            if ( index == decimals-1 ){
                currx =  glcd_draw_menu_item(page, currx,font_8px_decimal_glcd_bmp,4, options);
            }
            currx = glcd_draw_digit(page,currx, digitarray[index], options, fnt, width);
        }else{
            //Nothing changes
        }
        index--;
    }
    return(currx);
}

INT8S glcd_draw_digit(INT8U page,INT8S x,INT8U num, INT8U options, const INT8U *fnt[], INT8U width){
    //Assume the digit is between 0 and 9
    INT8S currx = 0;
    while(currx < width){
        glcd_buffer[(SCREEN_WIDTH*page)+(x+currx)] = fnt[num][currx];
        if ( options & GLCD_MENU_INVERT ){
            glcd_buffer[(SCREEN_WIDTH*page)+(x+currx)] ^= 0xFF;
        }else{
            // Do nothing
        }
        if ( options & GLCD_MENU_TOPBORDER ){
            glcd_buffer[(SCREEN_WIDTH*page)+(x+currx)] |= 0x01;
        }else{
            // Do nothing
        }
        currx++;
    }
    if ( options & GLCD_MENU_TOPBORDER ){
        glcd_buffer[(SCREEN_WIDTH*page)+(x+currx)] = 0x01;
    }else{
        glcd_buffer[(SCREEN_WIDTH*page)+(x+currx)] = 0x00;
    }
    if ( options & GLCD_MENU_INVERT ){
        glcd_buffer[(SCREEN_WIDTH*page)+(x+currx)] ^= 0xFF;
    }
    currx++;
    return(x+currx);
}

void glcd_reset_buffer(void){
    int n;
    for (n = 1; n <= (SCREEN_WIDTH * SCREEN_HEIGHT / 8) - 1; n++) {
        glcd_buffer[n] = 0;
    }
}
