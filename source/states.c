#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include "LcdDriver/Crystalfontz128x128_ST7735.h"
#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/grlib/grlib.h>

#include <stdint.h>
#include <stdio.h>
#include "hardware.h"
#include "states.h"
#include "ui.h"
#include "dht22.h"



// ====== VARIABLES & CONSTANTS ======

State_t current_state = STATE_INIT;

// Sample values for now
int target_water_ml = 100;      
int target_humidity_pct = 50;       // !! TENERE CONTO CHE I VALORI DAL DHT22 SONO = target_humidity_pct * 10
int target_temp_c = 25;             // !! TENERE CONTO CHE I VALORI DAL DHT22 SONO = target_temp_c * 10

#define WATER_MAX   500
#define HUM_MAX     100
#define TEMP_MAX    40
#define TEMP_MIN    25
#define WATER_STEP  10
#define HUM_STEP    5
#define TEMP_STEP   1

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


// #define EVT_B1_PRESS  0x01  // 0001 // Settings
// #define EVT_B2_PRESS  0x02  // 0010 // Up (board)
// #define EVT_B3_PRESS  0x04  // 0100 // Down (board)

void manual(){
    switch (current_hw) {
        case PUMP:
            // Case Up: turn on pump
            if (button_events & EVT_B2_PRESS){ 
                pump_state = 1;
                button_events &= ~EVT_B2_PRESS;
            }
            // Case Down: turn off pump
            // By default this turns off the device if both buttons are pressed
            if (button_events & EVT_B3_PRESS){ 
                pump_state = 0;
                button_events &= ~EVT_B3_PRESS;
            }
            // Case Settings: switch to next hardware and turn off 
            // Change setting last to prevent other hardware from firing accidentally
            if (button_events & EVT_B1_PRESS) {
                pump_state = false;
                current_hw = FAN;
                // Reset button states to prevent any potential bug
                button_events &= 0;
            }
            break;
        case FAN:
            // Analogous to pump
            // Case Up: turn on fan
            if (button_events & EVT_B2_PRESS){ 
                fan_state = 1;
                button_events &= ~EVT_B2_PRESS;
            }
            // Case Down: turn off fan
            if (button_events & EVT_B3_PRESS){ 
                fan_state = 0;
                button_events &= ~EVT_B3_PRESS;
            }
            // Case Settings: switch to next hardware and turn off 
            if (button_events & EVT_B1_PRESS) {
                fan_state = false;
                current_hw = HUMIDIFIER;
                button_events &= 0;
            }
            break;
        case HUMIDIFIER:
            // Case Up: turn on humidifier
            if (button_events & EVT_B2_PRESS){ 
                humidifier_state = 1;
                button_events &= ~EVT_B2_PRESS;
            }
            // Case Down: turn off humidifier
            if (button_events & EVT_B3_PRESS){ 
                humidifier_state = 0;
                button_events &= ~EVT_B3_PRESS;
            }
            // Case Settings: switch to next hardware and turn off 
            if (button_events & EVT_B1_PRESS) {
                humidifier_state = false;
                current_hw = RESISTOR;
                button_events &= 0;
            }
            break;
        case RESISTOR:
            // Case Up: turn on resistor
            if (button_events & EVT_B2_PRESS){ 
                resistor_state = 1;
                button_events &= ~EVT_B2_PRESS;
            }
            // Case Down: turn off fan
            if (button_events & EVT_B3_PRESS){ 
                resistor_state = 0;
                button_events &= ~EVT_B3_PRESS;
            }
            // Case Settings: switch to next hardware and turn off 
            if (button_events & EVT_B1_PRESS) {
                resistor_state = false;
                // Cycle back to pump
                current_hw = PUMP;
                button_events &= 0;
            }
            break;
        default:
            break;
    }
    // Display the current hardware and turn on/off hw
    printCurrentHardware(current_hw);
    updateHw();
}

void automatic(){
    // Change from num_states toreal state, used as a way to raise "not implemented" error
    //current_state = NUM_STATES;

    // sensor reading (managed by 2s timer)
    readSensors();

    // control logic (FIXED POINT (*10))
    // TEMPERATURE
    if (temperature_sensor_value > (target_temp_c * 10)) {
        fan_state = true;
    } else {
        fan_state = false;
    }

    // HUMIDITY
    if (humidity_sensor_value < (target_humidity_pct * 10)) {
        humidifier_state = true; 
    } else {
        humidifier_state = false;
    }

    updateHw();

    // refresh of LCD
    static int ui_refresh = 0;
    if (++ui_refresh >= 50) { // Aggiorna ogni 50 cicli (circa 0.5s)
        ui_refresh = 0;
        // convert fixed point for easier reading
        printSensorData(temperature_sensor_value / 10, humidity_sensor_value / 10);
    }
}

void settings(){
    // TODO!
}

// Sets current state depending on the value read by the status lever
void lever_status_set(void){
    int old_state = current_state;
    if (checkLever()){
        current_state = STATE_AUTOMATIC;
        if (current_state != old_state){
            // If there is something to do when changing to auto can be done here
        }
    } else {
        current_state = STATE_MANUAL;
        if (current_state != old_state){
            // When state changes from auto to manual, pause hardware
            pauseHw();
        }
    }
}

void fn_next_state(void) {
    switch (current_state) {
        case STATE_SET_WATER:
            current_state = STATE_SET_HUM;
            break;

        case STATE_SET_HUM:
            current_state = STATE_SET_TEMP;
            break;

        default: // handle both set_temp and all others in case of error
            lever_status_set();
            resumeHw();
            break;
    }
}

// Dummy implementation of settings

void fn_SET_WATER(void){
    // TODO!
//    Mettere in pausa il timer dell�irrigazione
//    Interrompere resistenza e ventola finch� non finiscono le impostazioni
//    Mostrare a schermo la quantit� di acqua impostata attualmente (misura da definire, per ora int)
//    Leggi l�input dei bottoni: aumenta la quantit� di acqua (B1) o diminuisci (B2), non cambiare se se il livello e a 0 o al massimo passa a IMPOSTAZIONI-UMIDIT� se B3
//    Stampa periodicamente il livello attuale; stampa a schermo �OFF� o �MAX� se il livello e a 0 o al massimo, usa funzione dedicata (tipo printWaterSetting)

    bool is_updated = false;
    pauseHw();
    // up
    if (button_events & EVT_B2_PRESS) {
        is_updated = true;
        target_water_ml += WATER_STEP;
        if (target_water_ml > WATER_MAX)
            target_water_ml = WATER_MAX;

        // Clear the B1 flag
        button_events &= ~EVT_B2_PRESS;
    }
    // down
    if (button_events & EVT_B3_PRESS) {
        is_updated = true;
        // catch overflow
        if (target_water_ml <= WATER_STEP)
            target_water_ml = 0;
        else
            target_water_ml -= WATER_STEP;

        // Clear the B2 flag
        button_events &= ~EVT_B3_PRESS;
    }
    // Settings
    if (button_events & EVT_B1_PRESS) {
        is_updated = true;
        // Move to next state
        fn_next_state();

        // Clear the B2 flag
        button_events &= ~EVT_B1_PRESS;
    }
    // update screen only if there is an update
    if (is_updated){
        printWaterSettings(target_water_ml);
    }
}

void fn_SET_HUMIDITY(void){
    // TODO!
//    Mostrare a schermo il livello di umidit� target attuale(int)
//    Leggi l�input dei bottoni: aumenta la quantit� di acqua (B1) o diminuisci (B2), non cambiare se se il livello e a 0 o al massimo passa a IMPOSTAZIONI-TEMPERATURA se B3
//    Stampa periodicamente il livello attuale; stampa a schermo �OFF� o �MAX� se il livello e a 0 o al massimo, usa funzione dedicata

    bool is_updated = false;
    pauseHw();
    if (button_events & EVT_B2_PRESS) {
        is_updated = true;
        target_humidity_pct += HUM_STEP;
        if (target_humidity_pct > HUM_MAX)
            target_humidity_pct = HUM_MAX;

        // Clear the B2 flag
        button_events &= ~EVT_B2_PRESS;
    }
    if (button_events & EVT_B3_PRESS) {
        is_updated = true;
        // catch overflow
        if (target_humidity_pct <= HUM_STEP)
            target_humidity_pct = 0;
        else
            target_humidity_pct -= HUM_STEP;

        // Clear the B3 flag
        button_events &= ~EVT_B3_PRESS;
    }
    if (button_events & EVT_B1_PRESS) {
        is_updated = true;
        // Move to next state
        fn_next_state();

        // Clear the B1 flag
        button_events &= ~EVT_B1_PRESS;
    }
    // update screen only if there is an update
    if (is_updated){
        printHumSettings(target_humidity_pct);
    }
}

void fn_SET_TEMP(void){
    // TODO!
//    Mostrare a schermo il livello di umidit� target attuale(int)
//    Leggi l�input dei bottoni: aumenta la quantit� di acqua (B1) o diminuisci (B2), non cambiare se se il livello e a 0 o al massimo passa a AUTO se B3
//    Stampa periodicamente il livello attuale; stampa a schermo �OFF� o �MAX� se il livello e a 0 o al massimo, usa funzione dedicata


    bool is_updated = false;
    pauseHw();
    if (button_events & EVT_B2_PRESS) {
        is_updated = true;
        target_temp_c += TEMP_STEP;
        if (target_temp_c > TEMP_MAX)
            target_temp_c = TEMP_MAX;

        // Clear the B2 flag
        button_events &= ~EVT_B2_PRESS;
    }
    if (button_events & EVT_B3_PRESS) {
        is_updated = true;
        target_temp_c -= TEMP_STEP;
        if (target_temp_c < TEMP_MIN)
            target_temp_c = TEMP_MIN;

        // Clear the B3 flag
        button_events &= ~EVT_B3_PRESS;
    }
    if (button_events & EVT_B1_PRESS) {
        is_updated = true;
        // Move to next state
        fn_next_state();

        // Clear the B1 flag
        button_events &= ~EVT_B1_PRESS;
    }
    // update screen only if there is an update
    if (is_updated){
        printTempSettings(target_temp_c);
    }
}

