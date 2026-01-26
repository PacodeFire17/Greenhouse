#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include "LcdDriver/Crystalfontz128x128_ST7735.h"
#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/grlib/grlib.h>
#include <stdint.h>
#include <stdio.h>
#include "hardware.h"
#include "ui.h"
#include "states.h"
#include "dht22.h"


// ====== STATE MACHINE DECLARATION  ======

StateMachine_t fsm[] = {
    {STATE_INIT, fn_INIT},
    {STATE_MANUAL, fn_MANUAL},
    {STATE_AUTOMATIC, fn_AUTOMATIC},
    {STATE_SET_WATER, fn_SET_WATER},
    {STATE_SET_HUM, fn_SET_HUMIDITY},
    {STATE_SET_TEMP, fn_SET_TEMP}
};

// ====== MAIN LOOP  ======

int main(void)
{
    // Initialization and initial state
    sys_init_logic();
    current_state = STATE_AUTOMATIC;
    // printf("[MAIN] Main Loop Started.\n");
    while (1)
    {
        // Go to low power mode
        PCM_gotoLPM0();
        
        // Execute State Machine
        if(current_state < NUM_STATES){
            (*fsm[current_state].state_function)();
        }
        
        // Save settings if changed
        if (settings_store.stored_temp != target_temp_c || 
            settings_store.stored_hum != target_humidity_pct) 
        {
            settings_store.stored_temp = target_temp_c;
            settings_store.stored_hum = target_humidity_pct;
            settings_store.stored_water = target_water_ml;
            settings_store.magic_number = 0xCAFEBABE;
        }
    }
}
