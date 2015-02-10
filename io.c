/********************************************************************
* io.c
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

#include "standard.h"
#include "msp430f5172.h"
#include "io.h"
#include "core.h"

#define DEBOUNCE_SAMPLES 3

INT8U ButtonSamples[DEBOUNCE_SAMPLES];

void IOinit(void){
    // Enable write access to the port mapping registers
    PMAPKEYID = PMAPKEY;
    // Set 1.0 to operate as the SPI clock
    P1MAP0 = PM_UCA0CLK;
    // Set 1.1 to operate as the SPI SIMO
    P1MAP1 = PM_UCA0SIMO;
    // Set 1.2 to operate as the SPI SOMI
    P1MAP2 = PM_UCA0SOMI;
    // Set 1.3 to act as UCB0CLK
    P1MAP3 = PM_UCB0CLK;
    // Set 1.4 to act as UCB0SIMO
    P1MAP4 = PM_UCB0SIMO;
    // Set 1.5 to act as UCB0SOMI, note that the SOMO seems to be a typo
    P1MAP5 = PM_UCB0SOMO;
    // Disable write access to the port mapping registers
    PMAPKEYID = 0;

    // Set the LCD/SD card SPI bus clock to use the SPI peripheral function
    LCD_SD_CK_PSEL |= LCD_SD_CK_BIT;

    // Set the LCD/SD card SPI clock to be an output (used if bitbanging is enabled)
    LCD_SD_CK_PDIR |= LCD_SD_CK_BIT;

    // Set the LCD/SD card SPI bus SIMO to use the SPI peripheral function
    LCD_SD_SIMO_PSEL |= LCD_SD_SIMO_BIT;

    // Set the LCD/SD card SPI SIMO to be an output (used if bitbanging is enabled)
    LCD_SD_SIMO_PDIR |= LCD_SD_SIMO_BIT;

    // Set the LCD/SD card SPI bus MISO to use the SPI peripheral function
    LCD_SD_MISO_PSEL |= LCD_SD_MISO_BIT;

    // Set the LCD/SD card SPI MISO to be an input (used if bitbanging is enabled)
    LCD_SD_MISO_PDIR &= !LCD_SD_MISO_BIT;

    // Set the SD Card CS to be an output
    SD_CS_PDIR |= SD_CS_BIT;

    // Set the LCD power enable to be an output 
    LCD_P_EN_PDIR |= LCD_P_EN_BIT;

    // Set the LCD A0 to be an output 
    LCD_A0_PDIR |= LCD_A0_BIT;

    // Set the LCD RST to be an output
    LCD_RST_PDIR |= LCD_RST_BIT;

    // Set the LCD CS to be an output
    LCD_CS_PDIR |= LCD_CS_BIT;

    // Set the Q3180 card SPI bus clock to use the SPI peripheral function
    MEAS_CK_PSEL |= MEAS_CK_BIT;

    // Set the Q3180 card SPI clock to be an output (used if bitbanging is enabled)
    MEAS_CK_PDIR |= MEAS_CK_BIT;

    // Set the Q3180 card SPI bus SIMO to use the SPI peripheral function
    MEAS_SIMO_PSEL |= MEAS_SIMO_BIT;

    // Set the Q3180 card SPI SIMO to be an output (used if bitbanging is enabled)
    MEAS_SIMO_PDIR |= MEAS_SIMO_BIT;

    // Set the Q3180 card SPI bus MISO to use the SPI peripheral function
    MEAS_MISO_PSEL |= MEAS_MISO_BIT;

    // Set the Q3180 card SPI MISO to be an output (used if bitbanging is enabled)
    MEAS_MISO_PDIR |= MEAS_MISO_BIT;

    // Set the Q3180 CS to be an output
    MEAS_CS_PDIR |= MEAS_CS_BIT;

    // Set the Q3180 RST to be an output
    MEAS_CS_PDIR |= MEAS_RST_PDIR;

    // Set the Q3180 IRQ to be an input
    MEAS_IRQ_PDIR &=~ MEAS_IRQ_BIT;

    // Enable pullup resistors for the button pins
    BUTTON_POUT |= BUTTON_LEFT_BIT | BUTTON_RIGHT_BIT | BUTTON_UP_BIT\
                | BUTTON_DOWN_BIT | BUTTON_SELECT_BIT;

    BUTTON_PREN |= BUTTON_LEFT_BIT | BUTTON_RIGHT_BIT | BUTTON_UP_BIT\
                | BUTTON_DOWN_BIT | BUTTON_SELECT_BIT;

    // Set the power button pin to be an input
    BIT_CLR(BUTTON_POWER_PDIR,BUTTON_POWER_BIT);
    // Set the power button pin to have a pullup resistor
    BIT_SET(BUTTON_POWER_POUT,BUTTON_POWER_BIT);
    BIT_SET(BUTTON_POWER_PREN,BUTTON_POWER_BIT);

}

void IObuttonTask(void){
    INT8S index;
    //Rotate the previously taken samples through the array
    for (index = DEBOUNCE_SAMPLES - 1; index >= 0; index--){
        ButtonSamples[index] = ButtonSamples[index - 1];
    }
    //This assumes that all the buttons are on the same port
    ButtonSamples[0] = (~BUTTON_PORT)&(BUTTON_LEFT_BIT | BUTTON_RIGHT_BIT | BUTTON_UP_BIT | BUTTON_DOWN_BIT | BUTTON_SELECT_BIT);
}

INT8U IOgetButtons(void){
    INT8S index;
    for (index = 0; index <DEBOUNCE_SAMPLES-1; index++){
        if (ButtonSamples[index] != ButtonSamples[index + 1]){
            // The samples don't match, assume it was a glitch
            return(0);
        }
    }
    // Samples all match
    return ButtonSamples[0];
}

void IOenableButtonInterrupts(void){
    // Clear any existing button interrupts
    BIT_CLR(P2IFG, BUTTON_LEFT_BIT | BUTTON_RIGHT_BIT | BUTTON_UP_BIT | BUTTON_DOWN_BIT | BUTTON_SELECT_BIT);
    // Set the button interrupt direction to interrupt on a high to low transition
    BIT_SET(P2IES, BUTTON_LEFT_BIT | BUTTON_RIGHT_BIT | BUTTON_UP_BIT | BUTTON_DOWN_BIT | BUTTON_SELECT_BIT);
    // Enable button interrupts
    BIT_SET(P2IE, BUTTON_LEFT_BIT | BUTTON_RIGHT_BIT | BUTTON_UP_BIT | BUTTON_DOWN_BIT | BUTTON_SELECT_BIT);
}

void IOdisableButtonInterrupts(void){
    BIT_CLR(P2IE, BUTTON_LEFT_BIT | BUTTON_RIGHT_BIT | BUTTON_UP_BIT | BUTTON_DOWN_BIT | BUTTON_SELECT_BIT);
}


#pragma vector=PORT2_VECTOR
__interrupt void ButtonInterrupt(void){
    switch(__even_in_range(P2IV, 16)){
    case BUTTON_LEFT_BIT:
    case BUTTON_RIGHT_BIT:
    case BUTTON_UP_BIT:
    case BUTTON_DOWN_BIT:
    case BUTTON_SELECT_BIT:
        // Exit low power mode for a few cycles through the kernel
        if (COREgetState()== CORE_STATE_LOG_SLEEP){
            __bic_SR_register_on_exit(LPM3_bits); // Exit active
        }else{
            //Do nothing
        }
        break;
    default:
        break;
    }
}
