#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <ti/grlib/grlib.h>
#include "LcdDriver/Crystalfontz128x128_ST7735.h"
#include <stdio.h>

// Rename and implement for FSM:
void f1(){
    ...
}

void f2(){
    ...
}

void f3(){
    ...
}

typedef enum {
    STATE_INIT,
    STATE_MANUAL,
    STATE_SETTINGS,
    STATE_AUTOMATIC,
    NUM_STATES
}State_t;

typedef struct{
    State_t state;
    void (*state_function)(void);
} StateMachine_t;

void fn_INIT(void);
void fn_MANUAL(void);
void fn_AUTOMATIC(void);
void fn_SETTINGS(void);

State_t current_state = STATE_INIT;

StateMachine_t fsm[] = {
                      {STATE_INIT, fn_INIT},
                      {STATE_MANUAL, fn_MANUAL},
                      {STATE_SETTINGS, fn_SETTINGS},
                      {STATE_AUTOMATIC, fn_AUTOMATIC}
};

void fn_INIT(){
       	f1();
        current_state = ...;
}

void fn_MANUAL(){
        f2();
        current_state = ...;
}

void fn_AUTOMATIC(){
        f3();
        current_state = ...;
}

void fn_SETTINGS(){
        f1();
        current_state = STATE_GREEN;
}


/* Application Defines  */
#define TIMER_PERIOD    0x2DC6

/* Timer_A UpMode Configuration Parameter */
const Timer_A_UpModeConfig upConfig =
{
        TIMER_A_CLOCKSOURCE_SMCLK,              // SMCLK Clock Source
        TIMER_A_CLOCKSOURCE_DIVIDER_64,         // SMCLK/64 = 3MHz/64
        TIMER_PERIOD,                           // every half second
        TIMER_A_TAIE_INTERRUPT_DISABLE,         // Disable Timer interrupt
        TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE ,    // Enable CCR0 interrupt
        TIMER_A_DO_CLEAR                        // Clear value
};

int main(void)
{
    /* Stop WDT  */
    WDT_A_holdTimer();

    // configure ports
    ...
    // better to define a function for this

    /* Configuring Timer_A1 for Up Mode */
    Timer_A_configureUpMode(TIMER_A1_BASE, &upConfig);

    /* Enabling interrupts and starting the timer */
    // Interrupt_enableSleepOnIsrExit();
    Interrupt_enableInterrupt(INT_TA1_0);
    Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);

    /* Enabling MASTER interrupts */
    Interrupt_enableMaster();

    /* Sleeping when not in use */
    while (1)
    {
        PCM_gotoLPM0();
        if(current_state < NUM_STATES){
            (*fsm[current_state].state_function)();
        }
        else{
            /* serious error */
        }
    }
}

void TA1_0_IRQHandler(void)
{
    // implement normal mode interrupt functions
    Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE,
            TIMER_A_CAPTURECOMPARE_REGISTER_0);
}

// Implement interrupts for other buttons and input as well
