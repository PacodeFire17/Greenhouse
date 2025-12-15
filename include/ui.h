#ifndef UI_H
#define UI_H

#include <ti/grlib/grlib.h>

// Functions

// initialization
void graphicsInit(void);
// Print on the screen temperature and humidity
void printSensorData(int temp, int hum);
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
