#ifdef TEST_MODE
    #include <ti/devices/msp432p4xx/driverlib/driverlib.h>
    #include "LcdDriver/Crystalfontz128x128_ST7735.h"
    #include <ti/devices/msp432p4xx/inc/msp.h>
    #include <ti/grlib/grlib.h>
    #include <stdint.h>
    #include <stdio.h>
    #include "hardware.h"
    #include "ui.h"
    #include "states.h"


    int main(void){
        graphicsInit();
        //     TESTS
        printf("Printing water\n");
        // Print in range
        printWaterSettings(150);
        Delay_ms(4000);
        // Print min-max
        printWaterSettings(0);
        Delay_ms(4000);
        printWaterSettings(WATER_MAX);
        Delay_ms(4000);
        // Print out of range
        printWaterSettings(-1); // Should print 0
        Delay_ms(4000);
        printWaterSettings(WATER_MAX + 1); // Should print WATER_MAX (510)
        Delay_ms(4000);

        printf("Printing humidity\n");
        // Print in range
        printHumSettings(75);
        Delay_ms(4000);
        // Print min-max
        printHumSettings(0);
        Delay_ms(4000);
        printHumSettings(HUM_MAX);
        Delay_ms(4000);
        // Print out of range
        printHumSettings(-5);
        Delay_ms(4000);
        printHumSettings(HUM_MAX + 10);
        Delay_ms(4000);

        printf("Printing temperature\n");
        // Print in range
        printTempSettings(28);
        Delay_ms(4000);
        // Print min-max
        printTempSettings(TEMP_MIN);
        Delay_ms(4000);
        printTempSettings(TEMP_MAX);
        Delay_ms(4000);
        // Print out of range
        printTempSettings(TEMP_MIN - 5);
        Delay_ms(4000);
        printTempSettings(TEMP_MAX + 5);
        Delay_ms(4000);

        /*  PUMP,
            FAN,
            HUMIDIFIER,
            RESISTOR
        */
        printf("Printing hardware\n");
        printCurrentHardware(PUMP);
        Delay_ms(4000);
        printCurrentHardware(FAN);
        Delay_ms(4000);
        printCurrentHardware(HUMIDIFIER);
        Delay_ms(4000);
        printCurrentHardware(RESISTOR);
        Delay_ms(4000);
        // Loop wrap check
        printCurrentHardware(FAN);
        Delay_ms(4000);
        printCurrentHardware(HUMIDIFIER);
        Delay_ms(4000);
        // ERROR, caught by the compiler as a warning so should not happen
        // printCurrentHardware(4);
        // Delay_ms(4000);
        printf("Finished tests\n");
    }
#endif