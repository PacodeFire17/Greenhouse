#ifndef APP_STATES_H
#define APP_STATES_H

#ifdef TEST_MODE
    #include <stdint.h>
    #include <stdio.h>
    #include <stdbool.h>
    #include "hardware.h"

//    extern Hardware current_hw;
#else
    #include <ti/devices/msp432p4xx/driverlib/driverlib.h>
    #include "LcdDriver/Crystalfontz128x128_ST7735.h"
    #include <ti/devices/msp432p4xx/inc/msp.h>
    #include <ti/grlib/grlib.h>
    #include <stdint.h>
    #include <stdio.h>
    #include "hardware.h"
#endif

// ------ 
// Button press states as Flags
#define EVT_NONE      0x00
#define EVT_B1_PRESS  0x01  // 0001 // Updated: Up (board)
#define EVT_B2_PRESS  0x02  // 0010 // Down
#define EVT_B3_PRESS  0x04  // 0100 // Settings (joystick)

//------
// FSM states
typedef enum
{
    STATE_INIT = 0,
    STATE_MANUAL,
    STATE_AUTOMATIC,
    STATE_SET_WATER,
    STATE_SET_HUM,
    STATE_SET_TEMP,
    NUM_STATES
}State_t;

// Definitions moved for accessibility
#define WATER_MAX   510
#define HUM_MAX     100
#define TEMP_MAX    35
#define TEMP_MIN    20
#define WATER_STEP  30              // Do not change to a non multiple of 30 to prevent errors; see T32_INT2_IRQHandler in hardware.c, CAMBIATO DA 30 A 10 PERCHE IL TEST LOGICO NON FUNZIONAVA (TOLLO) Se la logica funziona ma il test no, il problema � il test, non la logica
#define HUM_STEP    5
#define TEMP_STEP   1

typedef struct{
    State_t state;
    void (*state_function)(void);
} StateMachine_t;

// Extern variables
extern volatile State_t current_state;
extern volatile uint8_t button_events; //ISR variable
extern int target_water_ml;
extern int target_humidity_pct;
extern int target_temp_c;
extern int target_water_ml;      
extern int target_humidity_pct;  
extern int target_temp_c;  
// !
extern Hardware current_hw;

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
void lever_status_set(void);

#endif
