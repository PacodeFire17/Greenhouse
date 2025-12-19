#ifndef UI_H
#define UI_H

#ifdef TEST_MODE
    #include <stdint.h>
    #include <stdio.h>
    #include <stdbool.h>
	#include "hardware.h"
#else
    #include <ti/devices/msp432p4xx/driverlib/driverlib.h>
    #include "LcdDriver/Crystalfontz128x128_ST7735.h"
    #include <ti/devices/msp432p4xx/inc/msp.h>
    #include <ti/grlib/grlib.h>
    #include <stdint.h>
    #include <stdio.h>
    #include "hardware.h"
#endif

// Functions declaration
void graphicsInit(void);
void printSensorData(int temp, int hum); 	// Print on the screen temperature and humidity
// example
void drawTitle(void);
// Settings print
void printWaterSettings(int level);
void printHumSettings(int level);
void printTempSettings(int level);
void printCurrentHardware(Hardware hw);

// Graphic library context
extern Graphics_Context g_sContext;

#endif
