#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include "LcdDriver/Crystalfontz128x128_ST7735.h"
#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/grlib/grlib.h>

#include <stdint.h>
#include <stdio.h>
#include "hardware.h"
#include "states.h"
#include "ui.h"



// ====== VARIABLES & CONSTANTS ======

State_t current_state = STATE_INIT;

// Sample values for now
int target_water_ml = 100;
int target_humidity_pct = 50;
int target_temp_c = 25;

#define WATER_MAX   500
#define HUM_MAX     100
#define TEMP_MAX    40
#define TEMP_MIN    25
#define WATER_STEP  10
#define HUM_STEP    5
#define TEMP_STEP   1


// ====== FUNCTIONS ======

void fn_INIT(){
        init();
        current_state = STATE_AUTOMATIC;
}


void fn_MANUAL(){
        manual();
        // Change from num_states to real state, used as a way to raise "not implemented" error
        current_state = NUM_STATES;
}


// ===== TEST ====

void fn_AUTOMATIC(void){

    // 1. Simulate / Read sensors
    readSensors();

    // 2. LOGIC
    int buffer = 1;
    bool state_changed = false;

    // check temperature
    if (temperature_sensor_value > (target_temp_c + buffer)){
        if (!fan_state){
            fan_state = true;
            resistor_state = false;
            state_changed = true;
            printf("LOGIC: Too Hot (%d > %d) -> FAN ON", temperature_sensor_value, target_temp_c);
        }
    }
    else if (temperature_sensor_value < (target_temp_c - buffer)){
        if (!resistor_state){
            fan_state = false;
            resistor_state = true;
            state_changed = true;
            printf("LOGIC: Too Cold (%d < %d) -> RESISTOR ON", temperature_sensor_value, target_temp_c);
        }
    }

    //Check Humidity
    //Only check this occasionally based on timer_flag
    if (timer_flag){
        timer_flag = false;

        if (humidity_sensor_value < target_humidity_pct){
            if (!pump_state){
                pump_state = true;
                state_changed = true;
                printf("LOGIC: Dry Soil (%d < %d) --> PUMP ON", humidity_sensor_value, target_humidity_pct);
            }
        }
        else if (pump_state){
            pump_state = false;
            state_changed = true;
            printf("LOGIC: Soil Ok --> PUMP OFF");
        }
    }

    //Debug print to console every timer tick
    printf("STATUS: Temp = %d (Tg %d) | Hum = %d (Tg %d)", temperature_sensor_value, target_temp_c, humidity_sensor_value, target_humidity_pct);

    // 3. Apply to Hardware
    if(state_changed) updateHw();
}

// ==== END TEST ====



/*
void fn_AUTOMATIC(void){
    //automatic();
    // Change from num_states toreal state, used as a way to raise "not implemented" error
    //current_state = NUM_STATES;
}
*/


void manual(){
    // TODO!
}
// Renamed from "auto", probably a reserved word
void automatic(){
    // TODO!
}

void settings(){
    // TODO!
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
            current_state = STATE_AUTOMATIC;
            resumeHw();
            break;
    }
}

// Dummy implementation of settings

void fn_SET_WATER(void){
    // TODO!
//    Mettere in pausa il timer dell’irrigazione
//    Interrompere resistenza e ventola finché non finiscono le impostazioni
//    Mostrare a schermo la quantità di acqua impostata attualmente (misura da definire, per ora int)
//    Leggi l’input dei bottoni: aumenta la quantità di acqua (B1) o diminuisci (B2), non cambiare se se il livello e a 0 o al massimo passa a IMPOSTAZIONI-UMIDITÀ se B3
//    Stampa periodicamente il livello attuale; stampa a schermo “OFF” o “MAX” se il livello e a 0 o al massimo, usa funzione dedicata (tipo printWaterSetting)

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

//#define EVT_B1_PRESS  0x01  // 0001 // Settings
//#define EVT_B2_PRESS  0x02  // 0010 // Up (board)
//#define EVT_B3_PRESS  0x04  // 0100 // Down (board)


void fn_SET_HUMIDITY(void){
    // TODO!
//    Mostrare a schermo il livello di umidità target attuale(int)
//    Leggi l’input dei bottoni: aumenta la quantità di acqua (B1) o diminuisci (B2), non cambiare se se il livello e a 0 o al massimo passa a IMPOSTAZIONI-TEMPERATURA se B3
//    Stampa periodicamente il livello attuale; stampa a schermo “OFF” o “MAX” se il livello e a 0 o al massimo, usa funzione dedicata

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
//    Mostrare a schermo il livello di umidità target attuale(int)
//    Leggi l’input dei bottoni: aumenta la quantità di acqua (B1) o diminuisci (B2), non cambiare se se il livello e a 0 o al massimo passa a AUTO se B3
//    Stampa periodicamente il livello attuale; stampa a schermo “OFF” o “MAX” se il livello e a 0 o al massimo, usa funzione dedicata


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



