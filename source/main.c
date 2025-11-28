#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include "LcdDriver/Crystalfontz128x128_ST7735.h"
#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/grlib/grlib.h>
#include <stdint.h>
#include <stdio.h>


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

// States definition
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

// ====== FUNCTIONS ======

// ALL TO BE MOVED TO SEPARATE FILE

void _graphicsInit();
void _hwInit();
void _updateHw(void);

void printSensorData(int temp, int hum);

void fn_INIT(void);
void fn_MANUAL(void);
void fn_AUTOMATIC(void);
void fn_SETTINGS(void);

// Rename and implement for FSM
// Also, move to external file and leave only header (or alternative better organization)

void _init(){
    // reset the states
    // To be implemented later on: read from memory instead of resetting
    fan_state =        false;
    pump_state =       false;
    resistor_state =   false;
    humidifier_state = false;
    humidity_sensor_value =    0;
    temperature_sensor_value = 0;
    _graphicsInit();
    _hwInit();
    _updateHw();

}

void _auto(){
    // TODO!
}

void f3(){
    // TODO
}



State_t current_state = STATE_INIT;

StateMachine_t fsm[] = {
                      {STATE_INIT, fn_INIT},
                      {STATE_MANUAL, fn_MANUAL},
                      {STATE_SETTINGS, fn_SETTINGS},
                      {STATE_AUTOMATIC, fn_AUTOMATIC}
};

void fn_INIT(){
        _init();
        current_state = STATE_AUTOMATIC;
}


void fn_MANUAL(){
        _auto();
        // Read settings value: if it changes,
        current_state = NUM_STATES;
}

void fn_AUTOMATIC(){
        _auto();
        current_state = NUM_STATES;
}

void fn_SETTINGS(){
        f3();
        current_state = NUM_STATES;
}



// ====== MAIN ======

int main(void)
{


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

void TA1_0_IRQHandler(void)
{
    // implement normal mode interrupt functions
    Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE,
            TIMER_A_CAPTURECOMPARE_REGISTER_0);
}

// Implement interrupts for other buttons and input as well

void _graphicsInit()
{
    /* Graphic library context */
    Graphics_Context g_sContext;

    /* Initializes display */
    Crystalfontz128x128_Init();

    /* Set default screen orientation */
    Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP);

    /* Initializes graphics context */
    Graphics_initContext(&g_sContext, &g_sCrystalfontz128x128,
                         &g_sCrystalfontz128x128_funcs);
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED);
    Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
    GrContextFontSet(&g_sContext, &g_sFontFixed6x8);
    Graphics_clearDisplay(&g_sContext);
}

void _hwInit()
{

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

    /* Configuring Timer_A1 for Up Mode */
    Timer_A_configureUpMode(TIMER_A1_BASE, &upConfig);

    /* Enabling interrupts and starting the timer */
    // Interrupt_enableSleepOnIsrExit();
    Interrupt_enableInterrupt(INT_TA1_0);
    Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);

    /* Enabling MASTER interrupts */
    Interrupt_enableMaster();

    /* Halting WDT and disabling master interrupts */
    WDT_A_holdTimer();
    Interrupt_disableMaster();

    // Set output pins using the following:
    GPIO_setAsOutputPin(PUMP_PORT, PUMP_PIN);
    GPIO_setAsOutputPin(HUMIDIFIER_PORT, RESISTOR_PIN);
    GPIO_setAsOutputPin(HUMIDIFIER_PORT, HUMIDIFIER_PIN);

    // Set input pins
    // TODO! using function:
    // GPIO_setAsPeripheralModuleFunctionInputPin();

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
}

void _updateHw(void){
    // TODO!
    // Update hardware based on the variables:
    // fan_state
    // pump_state
    // resistor_state
    // humidifier_state
}
