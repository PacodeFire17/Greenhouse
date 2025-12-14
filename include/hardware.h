#ifndef HARDWARE_H
#define HARDWARE_H

#include <stdbool.h>
#include <stdint.h>

// Expose status variables so main.c can read them
extern bool fan_state;
extern bool pump_state;
extern bool resistor_state;
extern bool humidifier_state;
extern int16_t humidity_sensor_value;
extern int16_t temperature_sensor_value;
extern volatile bool timer_flag;
extern volatile uint8_t button_events;

// Function declaration
void init(void);
void hwInit(void);
void updateHw(void);
void pauseHw(void);
void resumeHw(void);

// New: call this in FSM loop 
void readSensors(void);

#endif
