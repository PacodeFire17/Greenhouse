#ifndef HARDWARE_H
#define HARDWARE_H

// Expose status variables so main.c can read them
extern bool fan_state;
extern bool pump_state;
extern bool resistor_state;
extern bool humidifier_state;
extern uint_fast8_t humidity_sensor_value;
extern uint_fast8_t temperature_sensor_value;
extern volatile bool timer_flag;
extern volatile uint8_t button_events;



// Functions declarations

// initialization
void init(void);
void hwInit(void);
// Change hardware state based on logic
void updateHw(void);
// Pause/resume
void pauseHw(void);
void resumeHw(void);

#endif
