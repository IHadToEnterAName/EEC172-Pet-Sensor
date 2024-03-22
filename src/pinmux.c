#include "pinmux.h"
#include "hw_types.h"
#include "hw_memmap.h"
#include "hw_gpio.h"
#include "pin.h"
#include "gpio.h"
#include "prcm.h"
#include "rom.h"
#include "rom_map.h"


//*****************************************************************************
void PinMuxConfig(void)
{


    //
    // Set unused pins to PIN_MODE_0 with the exception of JTAG pins 16,17,19,20
    //

       PinModeSet(PIN_03, PIN_MODE_0);
       PinModeSet(PIN_04, PIN_MODE_0);
       PinModeSet(PIN_06, PIN_MODE_0);
       PinModeSet(PIN_21, PIN_MODE_0);
       PinModeSet(PIN_53, PIN_MODE_0);
       PinModeSet(PIN_60, PIN_MODE_0);
       PinModeSet(PIN_63, PIN_MODE_0);

    //
    // Enable Peripheral Clocks
    //

    //
       PRCMPeripheralClkEnable(PRCM_GPIOA0, PRCM_RUN_MODE_CLK);
           PRCMPeripheralClkEnable(PRCM_GPIOA2, PRCM_RUN_MODE_CLK);
           PRCMPeripheralClkEnable(PRCM_GPIOA3, PRCM_RUN_MODE_CLK);
    PRCMPeripheralClkEnable(PRCM_GPIOA1, PRCM_RUN_MODE_CLK);
    PRCMPeripheralClkEnable(PRCM_GPIOA2, PRCM_RUN_MODE_CLK);
    PRCMPeripheralClkEnable(PRCM_GPIOA3, PRCM_RUN_MODE_CLK);
    //
    PRCMPeripheralClkEnable(PRCM_UARTA0, PRCM_RUN_MODE_CLK);
    PRCMPeripheralClkEnable(PRCM_GSPI, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralClkEnable(PRCM_TIMERA2, PRCM_RUN_MODE_CLK);


    //
    // Configure PIN_45 for DC Input
    //
    PinTypeGPIO(PIN_45, PIN_MODE_0, false);
    GPIODirModeSet(GPIOA3_BASE, 0x80, GPIO_DIR_MODE_OUT);

    //
    // Configure PIN_64 for GPIO Output
    //
    MAP_PinTypeGPIO(PIN_64, PIN_MODE_0, false);
    MAP_GPIODirModeSet(GPIOA1_BASE, 0x2, GPIO_DIR_MODE_OUT);


       //
       // Configure PIN_15 for GPIO Output // CS OC
       //
       PinTypeGPIO(PIN_15, PIN_MODE_0, false);
       GPIODirModeSet(GPIOA2_BASE, 0x40, GPIO_DIR_MODE_OUT);

       //
       // Configure PIN_08 for GPIO Input
       //
       MAP_PinTypeGPIO(PIN_08, PIN_MODE_0, false);
       MAP_GPIODirModeSet(GPIOA2_BASE, 0x2, GPIO_DIR_MODE_IN);

       //
       // Configure PIN_18 for GPIO Output // Reset
       //
       PinTypeGPIO(PIN_18, PIN_MODE_0, false);
       GPIODirModeSet(GPIOA3_BASE, 0x10, GPIO_DIR_MODE_OUT);

       //
       // Configure PIN_05 for SPI0 GSPI_CLK
       //
       PinTypeSPI(PIN_05, PIN_MODE_7);

       //
       // Configure PIN_07 for SPI0 GSPI_MOSI
       //
       PinTypeSPI(PIN_07, PIN_MODE_7);
       PinTypeGPIO(PIN_21, PIN_MODE_0, false);
           GPIODirModeSet(GPIOA3_BASE, 0x2, GPIO_DIR_MODE_IN);


           //
           // Configure PIN_55 for UART0 UART0_TX
           //
           PinTypeUART(PIN_55, PIN_MODE_3);

           //
           // Configure PIN_57 for UART0 UART0_RX
           //
           PinTypeUART(PIN_57, PIN_MODE_3);

           MAP_PinTypeGPIO(PIN_62, PIN_MODE_0, false);
               MAP_GPIODirModeSet(GPIOA0_BASE, 0x80, GPIO_DIR_MODE_IN);

               // CLOCK B
               // Configure PIN_50 for GPIO Output
               //
               MAP_PinTypeGPIO(PIN_50, PIN_MODE_0, false);
               MAP_GPIODirModeSet(GPIOA0_BASE, 0x1, GPIO_DIR_MODE_OUT);

               //
               // Configure PIN_01 for I2C0 I2C_SCL
               //
               MAP_PinTypeI2C(PIN_01, PIN_MODE_1);

               //
               // Configure PIN_02 for I2C0 I2C_SDA
               //
               MAP_PinTypeI2C(PIN_02, PIN_MODE_1);
}
