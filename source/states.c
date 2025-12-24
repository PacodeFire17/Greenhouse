#ifdef TEST_MODE
    #include <stdint.h>
    #include <stdio.h>
    #include <stdbool.h>
    #include "hardware.h"
    #include "ui.h"
    #include "states.h"

    extern volatile State_t current_state;
    extern Hardware current_hw;
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

// ====== VARIABLES & CONSTANTS ======

volatile State_t current_state = STATE_INIT;

// Sample values for now
int target_water_ml = 150;      
int target_humidity_pct = 50;
int target_temp_c = 25;

Hardware current_hw = PUMP;


// ====== FUNCTIONS ======

void fn_INIT(){
    init();
    lever_status_set();
}


void fn_MANUAL(){
    manual();
    lever_status_set();
}



void fn_AUTOMATIC(void){
    automatic();
    lever_status_set();
}


// #define EVT_B1_PRESS  0x01  // 0001 // Updated: Up (board)
// #define EVT_B2_PRESS  0x02  // 0010 // Down
// #define EVT_B3_PRESS  0x04  // 0100 // Settings (joystick)

void manual(){
    bool is_changed = false;
    if (timer_flag){
        timer_flag = false;
        lever_status_set();
    }
    switch (current_hw) {
        case PUMP:
            // Case Up: turn on pump
            if (button_events & EVT_B1_PRESS){ 
                printf("[STATES] Manual - Turning on pump, events: %3d\n", button_events);
                pump_state = 1;
                button_events &= ~EVT_B1_PRESS;
                is_changed = true;
            }
            // Case Down: turn off pump
            // By default this turns off the device if both buttons are pressed
            if (button_events & EVT_B2_PRESS){ 
                
                printf("[STATES] Manual - Turning off pump, events: %3d\n", button_events);
                pump_state = 0;
                button_events &= ~EVT_B2_PRESS;
                is_changed = true;
            }
            // Case Settings: switch to next hardware and turn off 
            // Change setting last to prevent other hardware from firing accidentally
            if (button_events & EVT_B3_PRESS) {
                printf("[STATES] Manual - b1 pressed, passing from pump to fan. events: %3d\n", button_events);
                pump_state = false;
                current_hw = FAN;
                // Reset button states to prevent any potential bug
                button_events &= 0;
                is_changed = true;
            }
            break;
        case FAN:
            // Analogous to pump
            // Case Up: turn on fan
            if (button_events & EVT_B1_PRESS){ 
                fan_state = 1;
                button_events &= ~EVT_B1_PRESS;
                is_changed = true;
            }
            // Case Down: turn off fan
            if (button_events & EVT_B2_PRESS){ 
                fan_state = 0;
                button_events &= ~EVT_B2_PRESS;
                is_changed = true;
            }
            // Case Settings: switch to next hardware and turn off 
            if (button_events & EVT_B3_PRESS) {
                fan_state = false;
                current_hw = HUMIDIFIER;
                button_events &= 0;
                is_changed = true;
            }
            break;
        case HUMIDIFIER:
            // Case Up: turn on humidifier
            if (button_events & EVT_B1_PRESS){ 
                humidifier_state = 1;
                button_events &= ~EVT_B1_PRESS;
                is_changed = true;
            }
            // Case Down: turn off humidifier
            if (button_events & EVT_B2_PRESS){ 
                humidifier_state = 0;
                button_events &= ~EVT_B2_PRESS;
                is_changed = true;
            }
            // Case Settings: switch to next hardware and turn off 
            if (button_events & EVT_B3_PRESS) {
                humidifier_state = false;
                current_hw = RESISTOR;
                button_events &= 0;
                is_changed = true;
            }
            break;
        case RESISTOR:
            // Case Up: turn on resistor
            if (button_events & EVT_B1_PRESS){ 
                resistor_state = 1;
                button_events &= ~EVT_B1_PRESS;
                is_changed = true;
            }
            // Case Down: turn off fan
            if (button_events & EVT_B2_PRESS){ 
                resistor_state = 0;
                button_events &= ~EVT_B2_PRESS;
                is_changed = true;
            }
            // Case Settings: switch to next hardware and turn off 
            if (button_events & EVT_B3_PRESS) {
                resistor_state = false;
                // Cycle back to pump
                current_hw = PUMP;
                button_events &= 0;
                is_changed = true;
            }
            break;
        default:
            break;
    }
    // Display the current hardware and turn on/off hw
    if (is_changed)
        printCurrentHardware(current_hw);
    updateHw();
}

void automatic(){
    // Change from num_states toreal state, used as a way to raise "not implemented" error
    //current_state = NUM_STATES;

    // Sensor value update (managed by interrupt now; moved all logic depending on this value in the if)
    if (three_s_flag) {
        three_s_flag = false;
        readSensors();
        printSensorData(temperature_sensor_value, humidity_sensor_value);
    

        // control logic
        // Added a larger boundary to prevent excessive fluctuation
        // TEMPERATURE
        if (temperature_sensor_value > (target_temp_c + TEMP_STEP)) {
            fan_state = true;
            resistor_state = false;
        } else if (temperature_sensor_value < target_temp_c - TEMP_STEP){
            fan_state = false;
            resistor_state = true;
        }

        // HUMIDITY
        if (humidity_sensor_value < (target_humidity_pct - HUM_STEP)) {
            humidifier_state = true;
        } else if (humidity_sensor_value > target_humidity_pct) {
            humidifier_state = false;
        }
        // Moved Update here to act only when there is a variation of sensor values
        updateHw();
    }
    // Check if settings button has been pressed
    if (button_events & EVT_B3_PRESS) {
        // Clear ALL button events to prevent stale events from bleeding into settings
        button_events = EVT_NONE;
        // Pause HW and proceed to settings
        pauseHw();
        current_state = STATE_SET_WATER;
        printWaterSettings(target_water_ml);
    } 
}

// Sets current state depending on the value read by the status lever
void lever_status_set(void){
    // NUM_STATES is used as a wildcard to let the function know the state must be set from scratch
    if (current_state != STATE_AUTOMATIC && current_state != STATE_MANUAL && current_state != NUM_STATES){
        printf("[STATES] Lever attempted to change state, but not in a changeable state\n");
        return;
    } 
    int old_state = current_state;
    if (checkLever()){
        current_state = STATE_AUTOMATIC;
        if (current_state != old_state){
            // Things to be run when entering automatic state, can be made into a function to call it at startup
            printf("[STATES] Lever changed state to automatic\n");
            // TODO: This can cause inconsistent values to be printed, since it will print outdated values. 
            // It can most likely be ignored with no consequences since temperature and humidity do not change much, but 
            // we must aknowledge this. 
            fan_state = 0;
            pump_state = 0;
            resistor_state = 0;
            humidifier_state = 0;
            printSensorData(temperature_sensor_value, humidity_sensor_value);
            resumeHw();
        }
    } else {
        current_state = STATE_MANUAL;
        if (current_state != old_state){
            // When state changes from auto to manual, pause hardware and print
            printf("[STATES] Lever changed state to manual\n");
            // Reset to default
            current_hw = PUMP;
            pauseHw();
            printCurrentHardware(current_hw);
        }
    }
}

void fn_next_state(void) {
    switch (current_state) {
        case STATE_SET_WATER:
            current_state = STATE_SET_HUM;
            printHumSettings(target_humidity_pct);
            break;

        case STATE_SET_HUM:
            current_state = STATE_SET_TEMP;
            printTempSettings(target_temp_c);
            break;

        default: // handle both set_temp and all others in case of error
            // Using NUM_States allows lever_status_set acts as an "always changed state" button for lever_status_set.
            current_state = NUM_STATES;
            lever_status_set();
            break;
    }
}

// #define EVT_B1_PRESS  0x01  // 0001 // Updated: Up (board)
// #define EVT_B2_PRESS  0x02  // 0010 // Down
// #define EVT_B3_PRESS  0x04  // 0100 // Settings (joystick)

// Functions for changing target settings
void fn_SET_WATER(void){
    bool is_updated = false;
    // pauseHw();
    // up
    if (button_events & EVT_B1_PRESS) {
        is_updated = true;
        target_water_ml += WATER_STEP;
        if (target_water_ml > WATER_MAX)
            target_water_ml = WATER_MAX;

        // Clear the B1 flag
        button_events &= ~EVT_B1_PRESS;
    }
    // down
    if (button_events & EVT_B2_PRESS) {
        is_updated = true;
        // catch overflow
        if (target_water_ml <= WATER_STEP)
            target_water_ml = 0;
        else
            target_water_ml -= WATER_STEP;

        // Clear the B2 flag
        button_events &= ~EVT_B2_PRESS;
    }
    // update screen only if there is an update
    if (is_updated){
        printWaterSettings(target_water_ml);
    }
    // Settings
    if (button_events & EVT_B3_PRESS) {
        // Move to next state
        fn_next_state();

        // Clear the flags
        button_events = EVT_NONE;
    }
}

void fn_SET_HUMIDITY(void){
    bool is_updated = false;    
    // pauseHw();
    if (button_events & EVT_B1_PRESS) {
        is_updated = true;
        target_humidity_pct += HUM_STEP;
        if (target_humidity_pct > HUM_MAX)
            target_humidity_pct = HUM_MAX;

        // Clear the B1 flag
        button_events &= ~EVT_B1_PRESS;
    }
    if (button_events & EVT_B2_PRESS) {
        is_updated = true;
        // catch overflow
        if (target_humidity_pct <= HUM_STEP)
            target_humidity_pct = 0;
        else
            target_humidity_pct -= HUM_STEP;

        // Clear the B2 flag
        button_events &= ~EVT_B2_PRESS;
    }
    // update screen only if there is an update
    if (is_updated){
        printHumSettings(target_humidity_pct);
    }
    if (button_events & EVT_B3_PRESS) {
        // Move to next state
        fn_next_state();

        // Clear the flags
        button_events = EVT_NONE;
    }
}

void fn_SET_TEMP(void){
    bool is_updated = false;
    // pauseHw();
    if (button_events & EVT_B1_PRESS) {
        is_updated = true;
        target_temp_c += TEMP_STEP;
        if (target_temp_c > TEMP_MAX)
            target_temp_c = TEMP_MAX;

        // Clear the B1 flag
        button_events &= ~EVT_B1_PRESS;
    }
    if (button_events & EVT_B2_PRESS) {
        is_updated = true;
        target_temp_c -= TEMP_STEP;
        if (target_temp_c < TEMP_MIN)
            target_temp_c = TEMP_MIN;

        // Clear the B2 flag
        button_events &= ~EVT_B2_PRESS;
    }
    // update screen only if there is an update
    if (is_updated){
        printTempSettings(target_temp_c);
    }
    if (button_events & EVT_B3_PRESS) {
        // Move to next state
        fn_next_state();

        // Clear the flags
        button_events = EVT_NONE;
    }
}
