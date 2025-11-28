#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include "LcdDriver/Crystalfontz128x128_ST7735.h"
#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/grlib/grlib.h>
#include <stdint.h>
#include <stdio.h>

#include "hardware.h"
#include "states.h"


// ====== VARIABLES & CONSTANTS ======

State_t current_state = STATE_INIT;


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

void fn_AUTOMATIC(){
        automatic();
        // Change from num_states to real state, used as a way to raise "not implemented" error
        current_state = NUM_STATES;
}

void fn_SETTINGS(){
        settings();
        // Change from num_states to real state, used as a way to raise "not implemented" error
        current_state = NUM_STATES;
}

void manual(){
    // TODO!
}
// Renamed from "auto", probably a reserved word
void automatic(){
    // TODO!
}

void settings(){
    // TODO
}



