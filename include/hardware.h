#ifndef HARDWARE_H
#define HARDWARE_H

#ifdef TEST_MODE
    #include <stdint.h>
    #include <stdio.h>
    #include <stdbool.h>
	typedef int Graphics_Context; 	// for test mode
    #include "dht22.h"
#else
    #include <ti/devices/msp432p4xx/driverlib/driverlib.h>
    #include "LcdDriver/Crystalfontz128x128_ST7735.h"
    #include <ti/devices/msp432p4xx/inc/msp.h>
    #include <ti/grlib/grlib.h>
    #include <stdint.h>
    #include <stdio.h>
    #include "dht22.h"
#endif

typedef enum{
	PUMP,
	FAN,
	HUMIDIFIER,
	RESISTOR
}Hardware;

// Function declaration
void init(void);
void hwInit(void);
void updateHw(void);
void pauseHw(void);
void resumeHw(void);
void graphicsInit(void);
bool checkLever(void);

// Operator Function 
void startFan(void);
void stopFan(void);
void startHum(void);
void stopHum(void);
void startPump(void);
void stopPump(void);
void startResistor(void);
void stopResistor(void);

// Sensor Function
void readSensors(void);

// Extern variables
extern bool fan_state;
extern bool pump_state;
extern bool resistor_state;
extern bool humidifier_state;
extern int16_t humidity_sensor_value;
extern int16_t temperature_sensor_value;
extern volatile bool timer_flag;
extern volatile bool three_s_flag;
extern volatile uint8_t button_events;

// Define event 
#define EVT_NONE 0x00
#define EVT_B1_PRESS 0x01
#define EVT_B2_PRESS 0x02
#define EVT_B3_PRESS 0x04


#endif
