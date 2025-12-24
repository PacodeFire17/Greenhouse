#ifdef TEST_MODE
    #include <stdint.h>
    #include <stdio.h>
    #include <stdbool.h>
    #include "hardware.h"
    #include "ui.h"
#else
    #include <ti/devices/msp432p4xx/driverlib/driverlib.h>
    #include "LcdDriver/Crystalfontz128x128_ST7735.h"
    #include <ti/devices/msp432p4xx/inc/msp.h>
    #include <ti/grlib/grlib.h>
    #include <stdint.h>
    #include <stdio.h>
    #include "hardware.h"
    #include "ui.h"
    #include "states.h"
#endif




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
    char string[16];
    sprintf(string, "Starting...");
    Graphics_drawStringCentered(&g_sContext, (int8_t *) string, AUTO_STRING_LENGTH, 64, 50, OPAQUE_TEXT);
}

// Prints temperature and humidity to the display
void printSensorData(int temp, int hum) {
    // printf("[UI] Printing - Temperature: %d C\t humidity: %d %%\n", temp, hum);
    Graphics_clearDisplay(&g_sContext);
    char string[20];
    sprintf(string, "AUTOMATIC");
    Graphics_drawStringCentered(&g_sContext, (int8_t *) string, AUTO_STRING_LENGTH, 64, 30, OPAQUE_TEXT);
    sprintf(string, "Temperature: %d C", temp);
    Graphics_drawStringCentered(&g_sContext, (int8_t *) string, AUTO_STRING_LENGTH, 64, 50, OPAQUE_TEXT);
    sprintf(string, "Humidity: %d %%", hum);
    Graphics_drawStringCentered(&g_sContext, (int8_t *) string, AUTO_STRING_LENGTH, 64, 70, OPAQUE_TEXT);
}

// Prints current water quantity and next setting on the display
void printWaterSettings(int level){
    Graphics_clearDisplay(&g_sContext);
    char string[21];
    printf("[UI] Printing target water level: %d\n",level);
    
    sprintf(string, "SETTINGS");
    Graphics_drawStringCentered(&g_sContext, (int8_t *) string, AUTO_STRING_LENGTH, 64, 30, OPAQUE_TEXT);
    sprintf(string, "Water level:");
    Graphics_drawStringCentered(&g_sContext, (int8_t *) string, AUTO_STRING_LENGTH, 64, 50, OPAQUE_TEXT);
    if (level > WATER_MAX || level < 0) {
        level = WATER_MAX * (level > WATER_MAX);
        printf("[UI] ERROR: water level out of bounds\n");
    } 
    if (level == WATER_MAX) {
        sprintf(string, "%d ml/day (MAX)", level);
    } else if (level == 0) {
        sprintf(string, "%d ml/day (MIN)", level);
    } else {
        sprintf(string, "%d ml/day", level);
    }
    Graphics_drawStringCentered(&g_sContext, (int8_t *) string, AUTO_STRING_LENGTH, 64, 60, OPAQUE_TEXT);
    sprintf(string, "Next: humidity");
    Graphics_drawStringCentered(&g_sContext, (int8_t *) string, AUTO_STRING_LENGTH, 64, 90, OPAQUE_TEXT);
}

// Prints current target humidity and next setting on the display
void printHumSettings(int level){
    Graphics_clearDisplay(&g_sContext);
    char string[21];
    printf("[UI] Printing target humidity level: %d\n",level);
    
    sprintf(string, "SETTINGS");
    Graphics_drawStringCentered(&g_sContext, (int8_t *) string, AUTO_STRING_LENGTH, 64, 30, OPAQUE_TEXT);
    sprintf(string, "Humidity level:");
    Graphics_drawStringCentered(&g_sContext, (int8_t *) string, AUTO_STRING_LENGTH, 64, 50, OPAQUE_TEXT);
    if (level > HUM_MAX || level < 0) {
        level = HUM_MAX * (level > HUM_MAX);
        printf("[UI] ERROR: humidity level out of bounds\n");
    } 
    if (level == HUM_MAX) {
        sprintf(string, "%d%% (MAX)", level);
    } else if (level == 0) {
        sprintf(string, "%d%% (MIN)", level);
    } else {
        sprintf(string, "%d%%", level);
    }  
    Graphics_drawStringCentered(&g_sContext, (int8_t *) string, AUTO_STRING_LENGTH, 64, 60, OPAQUE_TEXT);
    sprintf(string, "Next: temperature");
    Graphics_drawStringCentered(&g_sContext, (int8_t *) string, AUTO_STRING_LENGTH, 64, 90, OPAQUE_TEXT);
}

// Prints current target temperature and next setting on the display
void printTempSettings(int level){
    Graphics_clearDisplay(&g_sContext);
    char string[21];
    printf("[UI] Printing target temperature : %d\n",level);
    
    sprintf(string, "SETTINGS");
    Graphics_drawStringCentered(&g_sContext, (int8_t *) string, AUTO_STRING_LENGTH, 64, 30, OPAQUE_TEXT);
    sprintf(string, "Temperature:");
    Graphics_drawStringCentered(&g_sContext, (int8_t *) string, AUTO_STRING_LENGTH, 64, 50, OPAQUE_TEXT);
    if (level > TEMP_MAX) {
        level = TEMP_MAX;
        printf("[UI] ERROR: temp too high\n");
    } else if (level < TEMP_MIN) {
        level = TEMP_MIN;
        printf("[UI] ERROR: temp too low\n");
    }
    if (level == TEMP_MAX) {
        sprintf(string, "%d C (MAX)", level);
    } else if (level == TEMP_MIN) {
        sprintf(string, "%d C (MIN)", level);
    } else {
        sprintf(string, "%d C", level);
    }  
    Graphics_drawStringCentered(&g_sContext, (int8_t *) string, AUTO_STRING_LENGTH, 64, 60, OPAQUE_TEXT);
    sprintf(string, "Next: quit");
    Graphics_drawStringCentered(&g_sContext, (int8_t *) string, AUTO_STRING_LENGTH, 64, 90, OPAQUE_TEXT);
}

// Prints on the screen the current hardware being used in manual mode
void printCurrentHardware(Hardware hw){
    // No input check is required, since Hardware is between 0 and 3
    char string[12];
    // Includes fan twice to simulate loop
    const char *names[] = {"Pump", "Fan", "Humidifier", "Resistor", "Fan"};
    bool states[] = {pump_state, fan_state, humidifier_state, resistor_state};
    Graphics_clearDisplay(&g_sContext);
    printf("[UI] Printing - Hardware: %s, next: %s\n", names[hw], names[hw+1]);
    sprintf(string, "MANUAL");
    Graphics_drawStringCentered(&g_sContext, (int8_t *) string, AUTO_STRING_LENGTH, 64, 30, OPAQUE_TEXT);
    sprintf(string, "Current HW:");
    Graphics_drawStringCentered(&g_sContext, (int8_t *) string, AUTO_STRING_LENGTH, 64, 50, OPAQUE_TEXT);
    Graphics_drawStringCentered(&g_sContext, (int8_t *) names[hw], AUTO_STRING_LENGTH, 64, 60, OPAQUE_TEXT);
    if (states[hw]) {
        sprintf(string, "ON");
    } else {
        sprintf(string, "OFF");
    }
    Graphics_drawStringCentered(&g_sContext, (int8_t *) string, AUTO_STRING_LENGTH, 64, 70, OPAQUE_TEXT);
    sprintf(string, "Next:");
    Graphics_drawStringCentered(&g_sContext, (int8_t *) string, AUTO_STRING_LENGTH, 64, 90,
    OPAQUE_TEXT);
    Graphics_drawStringCentered(&g_sContext, (int8_t *) names[hw+1], AUTO_STRING_LENGTH, 64, 100, OPAQUE_TEXT);
}

