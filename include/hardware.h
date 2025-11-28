#ifndef HARDWARE_H
#define HARDWARE_H

// Expose status variables so main.c can read them
extern bool fan_state;
extern bool pump_state;
extern bool resistor_state;
extern bool humidifier_state;
extern uint_fast8_t humidity_sensor_value;
extern uint_fast8_t temperature_sensor_value;

// Graphic library context
extern Graphics_Context g_sContext;

// Functions declarations
void init(void);
void hwInit(void);
void updateHw(void);
void drawAccelData(void);
void drawTitle(void);
void graphicsInit(void);

#endif
