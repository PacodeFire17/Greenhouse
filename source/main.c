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



// --- PERSISTENCE (NOINIT) ---
// This instructs the linker NOT to initialize this variable to 0 on boot.
// It keeps its value through a Warm Reset (Software reset or reset button).

#pragma NOINIT(settings_store)
typedef struct {
    uint32_t magic_number; // Used to check if data is valid
    int stored_temp;
    int stored_hum;
    int stored_water;
} Settings_t;

Settings_t settings_store;

// --- STATE MACHINE ---
StateMachine_t fsm[] = {
    {STATE_INIT, fn_INIT},
    {STATE_MANUAL, fn_MANUAL},
    {STATE_AUTOMATIC, fn_AUTOMATIC},
    {STATE_SET_WATER, fn_SET_WATER},
    {STATE_SET_HUM, fn_SET_HUMIDITY},
    {STATE_SET_TEMP, fn_SET_TEMP}
};

// --- MODIFIED INIT FUNCTION ---
// Move this usually from hardware.h or keep local, 
// but ensure it's called at start.

void sys_init_logic(void) {
    
    // Initialize Hardware
    hwInit();
    graphicsInit(); // Even if no screen, keep it to prevent errors

    // RESTORE SETTINGS
    // Check if magic number matches (meaning we wrote to it previously)
    if (settings_store.magic_number == 0xCAFEBABE) {
        printf("SYSTEM: Restoring settings from memory...\n");
        target_temp_c = settings_store.stored_temp;
        target_humidity_pct = settings_store.stored_hum;
        target_water_ml = settings_store.stored_water;
    } else {
        printf("SYSTEM: First boot (or power loss). Setting defaults.\n");
        // Set Defaults
        target_temp_c = 25;
        target_humidity_pct = 50;
        target_water_ml = 150;
        
        // Save these defaults immediately
        settings_store.stored_temp = target_temp_c;
        settings_store.stored_hum = target_humidity_pct;
        settings_store.stored_water = target_water_ml;
        settings_store.magic_number = 0xCAFEBABE;
    }
    
    updateHw();
}

int main(void)
{
    // Call our new init logic
    sys_init_logic();
    
    // Set initial state
    current_state = STATE_AUTOMATIC;

    printf("[MAIN] Main Loop Started.\n");

    while (1)
    {
        // Go to sleep, wake up on Timer Interrupt (every 10ms)
        PCM_gotoLPM0();
        
        // Execute State Machine
        if(current_state < NUM_STATES){
            (*fsm[current_state].state_function)();
        }
        
        // SAVE SETTINGS
        // Update the store whenever we loop, or ideally only when changed.
        // For simplicity, we sync them here so if you reset, they are saved.
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
