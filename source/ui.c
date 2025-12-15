#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include "LcdDriver/Crystalfontz128x128_ST7735.h"
#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/grlib/grlib.h>

#include <stdint.h>
#include <stdio.h>
#include "hardware.h"
#include "states.h"
#include "ui.h"
#include "dht22.h"




void graphicsInit(void)
{
    /* Initializes display */
    Crystalfontz128x128_Init();

    /* Set default screen orientation */
    Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP);

    /* Initializes graphics context */
    Graphics_initContext(&g_sContext, &g_sCrystalfontz128x128,
                         &g_sCrystalfontz128x128_funcs);
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED);
    Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
    GrContextFontSet(&g_sContext, &g_sFontFixed6x8);
    Graphics_clearDisplay(&g_sContext);
}

void printSensorData(int temp, int hum) {
    printf("Temperature: %d\t humidity: %d\n", temp, hum);
}

/*
 * Clear display and redraw
 * Example implementation using accelerometer data
 */
void drawTitle(void)
{
    Graphics_clearDisplay(&g_sContext);
    Graphics_drawStringCentered(&g_sContext, (int8_t *) "Accelerometer:",
    AUTO_STRING_LENGTH,
                                64, 30,
                                OPAQUE_TEXT);
    // Purely as an example

    int resultsBuffer[3] = {0,1,2};
    char string[10];
        sprintf(string, "X: %5d", resultsBuffer[0]);
        Graphics_drawStringCentered(&g_sContext, (int8_t *) string, 8, 64, 50,
        OPAQUE_TEXT);

        sprintf(string, "Y: %5d", resultsBuffer[1]);
        Graphics_drawStringCentered(&g_sContext, (int8_t *) string, 8, 64, 70,
        OPAQUE_TEXT);

        sprintf(string, "Z: %5d", resultsBuffer[2]);
        Graphics_drawStringCentered(&g_sContext, (int8_t *) string, 8, 64, 90,
        OPAQUE_TEXT);
}


void printWaterSettings(int level){
    // TODO
    printf("Value of level = %d\n",level);
}

void printHumSettings(int level){
    // TODO!
    printf("Value of level = %d\n",level);
}

void printTempSettings(int level){
    // TODO!
    printf("Value of level = %d\n",level);
}

// Prints on the screen the current hardware being used in manual mode
void printCurrentHardware(Hardware hw){
    // TODO!
    printf("Manual operation\n");
}

