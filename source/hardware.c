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

// Definition of hardware pins desired status
// Check when running that this types are correct

// Ports (absolutely arbitrary and to be redefined, except for buttons)
const uint_fast8_t B1_PORT =                    GPIO_PORT_P1;
const uint_fast8_t B2_PORT =                    GPIO_PORT_P1;
const uint_fast8_t B3_PORT =                    GPIO_PORT_P1;
const uint_fast8_t FAN_PORT =                   GPIO_PORT_P1;
const uint_fast8_t PUMP_PORT =                  GPIO_PORT_P2;
const uint_fast8_t SWITCH_PORT =                GPIO_PORT_P2;
const uint_fast8_t RESISTOR_PORT =              GPIO_PORT_P3;
const uint_fast8_t HUMIDIFIER_PORT =            GPIO_PORT_P4;
const uint_fast8_t HUMIDITY_SENSOR_PORT =       GPIO_PORT_P5;
const uint_fast8_t TEMPERATURE_SENSOR_PORT =    GPIO_PORT_P6;

// Pins (equally arbitrary)
const uint_fast16_t B1_PIN =                    GPIO_PIN0;
const uint_fast16_t B2_PIN =                    GPIO_PIN1;
const uint_fast16_t B3_PIN =                    GPIO_PIN4;
const uint_fast16_t FAN_PIN =                   GPIO_PIN0;
const uint_fast16_t PUMP_PIN =                  GPIO_PIN1;
const uint_fast16_t SWITCH_PIN =                GPIO_PIN2;
const uint_fast16_t RESISTOR_PIN =              GPIO_PIN2;
const uint_fast16_t HUMIDIFIER_PIN =            GPIO_PIN3;
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
    // Should be ok for all our interrupt purposes...
#define TIMER_PERIOD    0xFFFF

// whatever graphics context is
Graphics_Context g_sContext;

// ====== FUNCTIONS ======

void hwInit(void)
{
    WDT_A_holdTimer();

    // Apparently it is good practice to disable interrupts globally while configuring hardware
    Interrupt_disableMaster();

    // Timer
    const Timer_A_UpModeConfig upConfig =
    {
            TIMER_A_CLOCKSOURCE_SMCLK,
            TIMER_A_CLOCKSOURCE_DIVIDER_64,
            TIMER_PERIOD,
            TIMER_A_TAIE_INTERRUPT_DISABLE,
            TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE,
            TIMER_A_DO_CLEAR
    };

    Timer_A_configureUpMode(TIMER_A1_BASE, &upConfig);
    // Interrupts for timer
    Interrupt_enableInterrupt(INT_TA1_0);
    Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);

    // Reinitialize variables before setting pins
    fan_state = false;
    pump_state = false;
    resistor_state = false;
    humidifier_state = false;

    // Fan
    GPIO_setAsOutputPin(FAN_PORT, FAN_PIN);
    GPIO_setOutputLowOnPin(FAN_PORT, FAN_PIN);

    // Pump
    GPIO_setAsOutputPin(PUMP_PORT, PUMP_PIN);
    GPIO_setOutputLowOnPin(PUMP_PORT, PUMP_PIN);

    // Resistor
    GPIO_setAsOutputPin(RESISTOR_PORT, RESISTOR_PIN);
    GPIO_setOutputLowOnPin(RESISTOR_PORT, RESISTOR_PIN);

    // Humidifier
    GPIO_setAsOutputPin(HUMIDIFIER_PORT, HUMIDIFIER_PIN);
    GPIO_setOutputLowOnPin(HUMIDIFIER_PORT, HUMIDIFIER_PIN);

    // Buttons - these are actually hardcoded instead of using the variables since they cannot be moved
    //P1.1
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);
    GPIO_interruptEdgeSelect(GPIO_PORT_P1, GPIO_PIN1, GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN1);
    GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN1);

    // P1.4
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN4);
    GPIO_interruptEdgeSelect(GPIO_PORT_P1, GPIO_PIN4, GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN4)
    GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN4);

    // Enable interrupts
    Interrupt_enableInterrupt(INT_PORT1);


    // Set input pins
    // TODO! should be something like this:
    //    GPIO_setAsInputPin(HUMIDITY_SENSOR_PORT, HUMIDITY_SENSOR_PIN);
    //    GPIO_setAsInputPin(TEMPERATURE_SENSOR_PORT, TEMPERATURE_SENSOR_PIN);

    // If it is instead pull up/down:
    // GPIO_setAsInputPinWithPullUpResistor(...);
    // GPIO_setAsInputPinWithPullDownResistor(...);

    /* Set the core voltage level to VCORE1 */
    PCM_setCoreVoltageLevel(PCM_VCORE1);

    /* Set 2 flash wait states for Flash bank 0 and 1*/
    FlashCtl_setWaitState(FLASH_BANK0, 2);
    FlashCtl_setWaitState(FLASH_BANK1, 2);

    /* Initializes Clock System */
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);
    CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    CS_initClockSignal(CS_HSMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    // Not used, to be replaced with other initialization for what we use
    //    _accelSensorInit();

    // Re-enable Interrupts
    Interrupt_enableMaster();
    // Re-enable watchdog timer as well, not sure how
    // TODO!
}

void pauseHw(void){
    // TODO!
}

void resumeHw(void){
    // TODO!
}

void updateHw(void){
    // TODO!
    // Update hardware based on the variables:
    // fan_state
    // pump_state
    // resistor_state
    // humidifier_state
}



void init(){
    // reset the states
    // To be implemented later on: read from memory instead of resetting
    fan_state =        false;
    pump_state =       false;
    resistor_state =   false;
    humidifier_state = false;
    humidity_sensor_value =    0;
    temperature_sensor_value = 0;
    graphicsInit();
    hwInit();
    updateHw();

}



// variable timer flag 
volatile bool timer_flag = false;

// Timer 1 interrupt
void TA1_0_IRQHandler(void)
{
    // clear hardware flag 
    Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);

    // timer_flag == true --> period of time has passed
    timer_flag = true;

    // implement normal mode interrupt functions
    // TODO!
}



// Button handler in port 1
volatile uint8_t button_events = EVT_NONE;

void PORT1_IRQHandler(void)
{
    uint32_t status = GPIO_getEnabledInterruptStatus(GPIO_PORT_P1);
    GPIO_clearInterruptFlag(GPIO_PORT_P1, status);

    // Use or to stack events when many happen at the same time
    if (status & B1_PIN) {
        button_events |= EVT_B1_PRESS;
    }

    if (status & B2_PIN) {
        button_events |= EVT_B2_PRESS;
    }

    // Assuming B3 is on Pin 4 based on your previous messages
    if (status & GPIO_PIN4) {
        button_events |= EVT_B3_PRESS;
    }
}

