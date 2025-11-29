#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include "LcdDriver/Crystalfontz128x128_ST7735.h"
#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/grlib/grlib.h>
#include <stdint.h>
#include <stdio.h>

// IMPORTANT:
// add the include folder to the arm compiler options or it will fail

#include "hardware.h"
#include "states.h"
#include "ui.h"



// ====== VARIABLES & CONSTANTS ======

// moved to hardware.c

// ====== STATE MACHINE ======



StateMachine_t fsm[] = {
                      {STATE_INIT, fn_INIT},
                      {STATE_MANUAL, fn_MANUAL},
                      {STATE_SET_WATER, fn_SET_WATER},
                      {STATE_SET_HUM, fn_SET_HUMIDITY},
                      {STATE_SET_TEMP, fn_SET_TEMP},
                      {STATE_AUTOMATIC, fn_AUTOMATIC}
};

// ====== MAIN ======

int main(void)
{
    init();

    /* Sleeping when not in use */
    while (1)
    {
        PCM_gotoLPM0();
        if(current_state < NUM_STATES){
            (*fsm[current_state].state_function)();
        }
        else{
            // This means something was left not implemented
            /* serious error */
        }
    }
}



