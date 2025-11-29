#ifndef APP_STATES_H
#define APP_STATES_H

// FSM states
typedef enum
{
    STATE_INIT,
    STATE_MANUAL,
    STATE_AUTOMATIC,
    STATE_SET_WATER,
    STATE_SET_HUM,
    STATE_SET_TEMP,
    NUM_STATES
}State_t;

// Button press states as Flags
#define EVT_NONE      0x00
#define EVT_B1_PRESS  0x01  // 0001 // Settings
#define EVT_B2_PRESS  0x02  // 0010 // Up (board)
#define EVT_B3_PRESS  0x04  // 0100 // Down (board)

extern volatile uint8_t button_events;

typedef struct{
    State_t state;
    void (*state_function)(void);
} StateMachine_t;

// Extern variables
extern State_t current_state;

extern int target_water_ml;
extern int target_humidity_pct;
extern int target_temp_c;

// Functions
void fn_SET_WATER(void);
void fn_SET_HUMIDITY(void);
void fn_SET_TEMP(void);
void fn_INIT(void);
void fn_MANUAL(void);
void fn_AUTOMATIC(void);
void fn_SETTINGS(void);

void manual(void);
// Renamed from "auto", probably a reserved word
void automatic(void);
void settings(void);

#endif
