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
}

void printSensorData(int temp, int hum) {
    printf("Printing\nTemperature: %d\t humidity: %d\n", temp, hum);
    Graphics_clearDisplay(&g_sContext);
    char string[20];
    sprintf(string, "Temperature: %d", temp);
    Graphics_drawStringCentered(&g_sContext, (int8_t *) string, AUTO_STRING_LENGTH, 64, 50, OPAQUE_TEXT);
    sprintf(string, "Humidity: %d", hum);
    Graphics_drawStringCentered(&g_sContext, (int8_t *) string, AUTO_STRING_LENGTH, 64, 70, OPAQUE_TEXT);

}

// Prints current water level and displays next setting
void printWaterSettings(int level){
    Graphics_clearDisplay(&g_sContext);
    char string[21];
    printf("Printing target water level: %d\n",level);
    
    sprintf(string, "Water level:");
    Graphics_drawStringCentered(&g_sContext, (int8_t *) string, AUTO_STRING_LENGTH, 64, 30, OPAQUE_TEXT);
    if (level > WATER_MAX || level < 0) {
        level = WATER_MAX * (level > WATER_MAX);
        printf("ERROR: water level out of bounds\n");
    } 
    if (level == WATER_MAX) {
        sprintf(string, "%d ml/day (MAX)", level);
    } else if (level == 0) {
        sprintf(string, "%d ml/day (MIN)", level);
    } else {
        sprintf(string, "%d ml/day", level);
    }
    Graphics_drawStringCentered(&g_sContext, (int8_t *) string, AUTO_STRING_LENGTH, 64, 50, OPAQUE_TEXT);
    sprintf(string, "Next: humidity");
    Graphics_drawStringCentered(&g_sContext, (int8_t *) string, AUTO_STRING_LENGTH, 64, 70, OPAQUE_TEXT);
}

void printHumSettings(int level){
    Graphics_clearDisplay(&g_sContext);
    char string[21];
    printf("Printing target humidity level: %d\n",level);
    
    sprintf(string, "Humidity level:");
    Graphics_drawStringCentered(&g_sContext, (int8_t *) string, AUTO_STRING_LENGTH, 64, 30, OPAQUE_TEXT);
    if (level > HUM_MAX || level < 0) {
        level = HUM_MAX * (level > HUM_MAX);
        printf("ERROR: humidity level out of bounds\n");
    } 
    if (level == HUM_MAX) {
        sprintf(string, "%d%% (MAX)", level);
    } else {
        sprintf(string, "%d%%", level);
    }  
    Graphics_drawStringCentered(&g_sContext, (int8_t *) string, AUTO_STRING_LENGTH, 64, 50, OPAQUE_TEXT);
    sprintf(string, "Next: temperature");
    Graphics_drawStringCentered(&g_sContext, (int8_t *) string, AUTO_STRING_LENGTH, 64, 70, OPAQUE_TEXT);
}

void printTempSettings(int level){
    Graphics_clearDisplay(&g_sContext);
    char string[21];
    printf("Printing target temperature : %d\n",level);
    
    sprintf(string, "Temperature:");
    Graphics_drawStringCentered(&g_sContext, (int8_t *) string, AUTO_STRING_LENGTH, 64, 30, OPAQUE_TEXT);
    if (level > TEMP_MAX) {
        level = TEMP_MAX;
        printf("ERROR: temp too high\n");
    } else if (level < TEMP_MIN) {
        level = TEMP_MIN;
        printf("ERROR: temp too low\n");
    }
    if (level == TEMP_MAX) {
        sprintf(string, "%d C (MAX)", level);
    } else if (level == TEMP_MIN) {
        sprintf(string, "%d C (MIN)", level);
    } else {
        sprintf(string, "%d C", level);
    }  
    Graphics_drawStringCentered(&g_sContext, (int8_t *) string, AUTO_STRING_LENGTH, 64, 50, OPAQUE_TEXT);
    sprintf(string, "Next: quit");
    Graphics_drawStringCentered(&g_sContext, (int8_t *) string, AUTO_STRING_LENGTH, 64, 70, OPAQUE_TEXT);
}

// Prints on the screen the current hardware being used in manual mode
void printCurrentHardware(Hardware hw){
    // No input check is required, since Hardware is between 0 and 3
    // Adding fan again to simulate loop
    char string[12];
    const char *names[] = {"Pump", "Fan", "Humidifier", "Resistor", "Fan"};
    Graphics_clearDisplay(&g_sContext);
    printf("Printing: \tHardware: %s, next: %s\n", names[hw], names[hw+1]);
    
    sprintf(string, "Current HW:");
    Graphics_drawStringCentered(&g_sContext, (int8_t *) string, AUTO_STRING_LENGTH, 64, 30,
    OPAQUE_TEXT);
    Graphics_drawStringCentered(&g_sContext, (int8_t *)names[hw], 
                                AUTO_STRING_LENGTH, 64, 50, OPAQUE_TEXT);
    sprintf(string, "Next:");
    Graphics_drawStringCentered(&g_sContext, (int8_t *) string, AUTO_STRING_LENGTH, 64, 70,
    OPAQUE_TEXT);
    Graphics_drawStringCentered(&g_sContext, (int8_t *)names[hw+1], 
                                AUTO_STRING_LENGTH, 64, 90, OPAQUE_TEXT);
    
}

