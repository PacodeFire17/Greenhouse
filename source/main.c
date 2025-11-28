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



// ====== VARIABLES & CONSTANTS ======

// Definition of hardware pins desired status
// Check when running that this types are correct

// moved to hardware.c

// ====== STATE MACHINE ======



StateMachine_t fsm[] = {
                      {STATE_INIT, fn_INIT},
                      {STATE_MANUAL, fn_MANUAL},
                      {STATE_SETTINGS, fn_SETTINGS},
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

// Can we move this?
void TA1_0_IRQHandler(void)
{
    // implement normal mode interrupt functions
    Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE,
            TIMER_A_CAPTURECOMPARE_REGISTER_0);
}

// Implement interrupts for other buttons and input as well


