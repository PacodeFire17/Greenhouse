#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include "LcdDriver/Crystalfontz128x128_ST7735.h"
#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/grlib/grlib.h>
#include <stdint.h>
#include <stdio.h>

#include "hardware.h"
#include "states.h"


// ====== VARIABLES & CONSTANTS ======

// Definition of hardware pins desired status
// Check when running that this types are correct

// Ports
#define FAN_PORT =           GPIO_PORT_P0;
const uint_fast8_t PUMP_PORT =          GPIO_PORT_P1;
const uint_fast8_t RESISTOR_PORT =      GPIO_PORT_P2;
const uint_fast8_t HUMIDIFIER_PORT =    GPIO_PORT_P3;
const uint_fast8_t HUMIDITY_SENSOR_PORT =       GPIO_PORT_P4;
const uint_fast8_t TEMPERATURE_SENSOR_PORT =    GPIO_PORT_P5;

// Pins
const uint_fast16_t FAN_PIN =           GPIO_PIN0;
const uint_fast16_t PUMP_PIN =          GPIO_PIN1;
const uint_fast16_t RESISTOR_PIN =      GPIO_PIN2;
const uint_fast16_t HUMIDIFIER_PIN =    GPIO_PIN3;
const uint_fast16_t HUMIDITY_SENSOR_PIN =       GPIO_PIN4;
const uint_fast16_t TEMPERATURE_SENSOR_PIN =    GPIO_PIN5;

// Status
bool fan_state =        false;
bool pump_state =       false;
bool resistor_state =   false;
bool humidifier_state = false;
uint_fast8_t humidity_sensor_value =    0;
uint_fast8_t temperature_sensor_value = 0;

    // Mi sembrava che ci fosse un modo in C per non dire il tipo di una costante
    // This grants approx 4 interrupts a second - version given by professor in accelerometer_lcd.c
// #define TIMER_PERIOD    0x2DC6
    // Slower version, approx. 0.7/sec
    // Should be ok for all our interrupt purposes
#define TIMER_PERIOD    0xFFFF

// ====== STATE MACHINE ======

State_t current_state = STATE_INIT;

StateMachine_t fsm[] = {
                      {STATE_INIT, fn_INIT},
                      {STATE_MANUAL, fn_MANUAL},
                      {STATE_SETTINGS, fn_SETTINGS},
                      {STATE_AUTOMATIC, fn_AUTOMATIC}
};

// ====== MAIN ======

int main(void)
{
    init()

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

// Non so se si può spostare questa implementazione?
void TA1_0_IRQHandler(void)
{
    // implement normal mode interrupt functions
    Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE,
            TIMER_A_CAPTURECOMPARE_REGISTER_0);
}

// Implement interrupts for other buttons and input as well


