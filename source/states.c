#include "states.h"
#include "hardware.h"




void _auto(){
    // TODO!
}

void _settings(){
    // TODO
}

void fn_INIT(){
        _init();
        current_state = STATE_AUTOMATIC;
}


void fn_MANUAL(){
        _auto();
        // Change from num_states to real state, used as a way to raise "not implemented" error
        current_state = NUM_STATES;
}

void fn_AUTOMATIC(){
        _auto();
        // Change from num_states to real state, used as a way to raise "not implemented" error
        current_state = NUM_STATES;
}

void fn_SETTINGS(){
        _settings();
        // Change from num_states to real state, used as a way to raise "not implemented" error
        current_state = NUM_STATES;
}



