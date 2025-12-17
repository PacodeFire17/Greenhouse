#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include "LcdDriver/Crystalfontz128x128_ST7735.h"
#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/grlib/grlib.h>

#include <stdint.h>
#include <stdio.h>
#include "hardware.h"
#include "states.h"
#include "ui.h"
#include "dht22.h"

// ====== VARIABLES & CONSTANTS ======

// Ports (absolutely arbitrary and to be redefined, except for buttons)
const uint_fast8_t B1_PORT =                    GPIO_PORT_P5;   //S1 button
const uint_fast8_t B2_PORT =                    GPIO_PORT_P3;   //S2 button
const uint_fast8_t B3_PORT =                    GPIO_PORT_P1;
const uint_fast8_t FAN_PORT =                   GPIO_PORT_P1;
const uint_fast8_t PUMP_PORT =                  GPIO_PORT_P2;
const uint_fast8_t LEVER_PORT =                 GPIO_PORT_P3;
const uint_fast8_t SWITCH_PORT =                GPIO_PORT_P2;
const uint_fast8_t RESISTOR_PORT =              GPIO_PORT_P3;
const uint_fast8_t HUMIDIFIER_PORT =            GPIO_PORT_P4;

// Pins (equally arbitrary)
const uint_fast16_t B1_PIN =                    GPIO_PIN1;  //S1 button,before 1.1, now canged to 5.1
const uint_fast16_t B2_PIN =                    GPIO_PIN5;  //S2 button, before 1.4, now changed to 3.5
const uint_fast16_t B3_PIN =                    GPIO_PIN5;  //don't change
const uint_fast16_t FAN_PIN =                   GPIO_PIN0;
const uint_fast16_t PUMP_PIN =                  GPIO_PIN1;
const uint_fast16_t LEVER_PIN =                 GPIO_PIN1;
const uint_fast16_t SWITCH_PIN =                GPIO_PIN2;
const uint_fast16_t RESISTOR_PIN =              GPIO_PIN2;
const uint_fast16_t HUMIDIFIER_PIN =            GPIO_PIN3;

// Status flags
bool fan_state =        false;
bool pump_state =       false;
bool resistor_state =   false;
bool humidifier_state = false;
bool pump_is_watering = false;
bool pump_timer_state = false;
int16_t humidity_sensor_value =    0;
int16_t temperature_sensor_value = 0;

// TODO!
// Check before release that the pins and ports defined match the hardware

volatile bool dht22_error_flag = false;

//timer A1 count 100Hz (10ms)
#define TIMER_PERIOD 7500

// Cycles (3s) between watering in seconds. Currently set for 12h
#define PUMP_TIMER_PERIOD 14400

// whatever graphics context is
Graphics_Context g_sContext;

// Button handler in port 1
volatile uint8_t button_events = EVT_NONE;

// variable timer flag 
volatile bool timer_flag = false;
volatile bool three_s_flag = false;

// contatore per il blocco dei rimbalzi (time lockout)
volatile uint8_t debounce_countdown = 0; 

// Duration of a pulse to toggle humidifier status (ms)
const uint_fast8_t hum_pulse_duration_ms = 10;

// Dual purpose pump counter (coutns between watering cycles and for watering duration)
volatile uint16_t pump_timer = 0;

// ====== FUNCTIONS ======

void hwInit(void) {

    // Halt watchdog timer and disable interrupts 
    WDT_A_holdTimer();
    Interrupt_disableMaster();

    // CORE SYSTEM (POWER & FLASH)
    // Set the core voltage level to VCORE1
    PCM_setCoreVoltageLevel(PCM_VCORE1);
    // Set 2 flash wait states for Flash bank 0 and 1
    FlashCtl_setWaitState(FLASH_BANK0, 2);
    FlashCtl_setWaitState(FLASH_BANK1, 2);    

    // CLOCK SYSTEM 
    // Initializes Clock System
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);
    CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    CS_initClockSignal(CS_HSMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    // GPIO (OUTPUTS)
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

    // GPIO (INPUTS & INTERRUPTS)
    // B1 = S1 = P5.1
    GPIO_setAsInputPinWithPullUpResistor(B1_PORT, B1_PIN);
    GPIO_interruptEdgeSelect(B1_PORT, B1_PIN, GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_clearInterruptFlag(B1_PORT, B1_PIN);
    GPIO_enableInterrupt(B1_PORT, B1_PIN);

    // B2 = S2 = P3.5
    GPIO_setAsInputPinWithPullUpResistor(B2_PORT, B2_PIN);
    GPIO_interruptEdgeSelect(B2_PORT, B2_PIN, GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_clearInterruptFlag(B2_PORT, B2_PIN);
    GPIO_enableInterrupt(B2_PORT, B2_PIN);

    // B3 = P1.5
    GPIO_setAsInputPinWithPullUpResistor(B3_PORT, B3_PIN);
    GPIO_interruptEdgeSelect(B3_PORT, B3_PIN, GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_clearInterruptFlag(B3_PORT, B3_PIN);
    GPIO_enableInterrupt(B3_PORT, B3_PIN);  

    // Enable global interrupt  
    Interrupt_enableInterrupt(INT_PORT1);

    // PERIPHERALS (TIMERS)
    // Timer
    const Timer_A_UpModeConfig upConfig =
    {
            TIMER_A_CLOCKSOURCE_SMCLK,              // f = 48 MHz
            TIMER_A_CLOCKSOURCE_DIVIDER_64,         // timer_f = 750 kHz
            TIMER_PERIOD,                           // 7500 tick = 10ms (100Hz)
            TIMER_A_TAIE_INTERRUPT_DISABLE,
            TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE,
            TIMER_A_DO_CLEAR
    };

    // Interrupts for timer
    Timer_A_configureUpMode(TIMER_A1_BASE, &upConfig);
    Interrupt_enableInterrupt(INT_TA1_0);
    Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);

    // VARIABLES
    fan_state = false;
    pump_state = false;
    resistor_state = false;
    humidifier_state = false;
    pump_timer_state = true;

    // HUMIDIFY & TEMPERATURE SENSOR INIT 
    DHT22_Init();

    // Timer32_1 -> every 3 seconds
    // Timer32 runs from MCLK; with prescaler 256 -> 48MHz/256 = 187500 Hz
    // Count for 3s: 187500 * 3 = 562500
    Timer32_initModule(TIMER32_1_BASE, TIMER32_PRESCALER_256, TIMER32_32BIT, TIMER32_PERIODIC_MODE);
    Timer32_setCount(TIMER32_1_BASE, 562500);
    Interrupt_enableInterrupt(INT_T32_INT2);
    Timer32_startTimer(TIMER32_1_BASE, true);

    // Re-enable Interrupts and watchdog
    Interrupt_enableMaster();
    WDT_A_startTimer();
}

// Function to interrupt all active hardware
void pauseHw(void){
    stopFan();
    stopResistor();
    stopPump();
    // Block pump timer
    pump_timer_state = false;
    // Should be last since it takes 2*hum_pulse_duration_ms
    stopHum(); 

    // TODO!: verify: is it actually correct to stop this?
    //disable interrupt timer (stop counter)
    Interrupt_disableInterrupt(INT_TA1_0);
}

void resumeHw(void){
    // Updating will automatically resume everything
    updateHw();
    pump_timer_state = true;
    //reume interrupt timer
    Interrupt_enableInterrupt(INT_TA1_0);
}

// Function to update hardware based on:
// fan_state
// pump_state
// resistor_state
// humidifier_state
void updateHw(void){
    // Update hardware based on current state variables
    if (fan_state) 
        startFan();
    else 
        stopFan();

    if (pump_state) 
        startPump();
    else 
        stopPump();

    if (resistor_state) 
        startResistor();
    else 
        stopResistor();

    if (humidifier_state) 
        startHum();
    else 
        stopHum();

    // Service watchdog timer
    WDT_A_clearTimer();
}

// Full harware initialization function
void init(){
    // reset the states
    // To be implemented later on: read from memory instead of resetting
    fan_state =        false;
    pump_state =       false;
    resistor_state =   false;
    humidifier_state = false;
    humidity_sensor_value =    0;
    temperature_sensor_value = 0;
    DHT22_Init();
    graphicsInit();
    hwInit();
    updateHw();
}

// Interrupt handler for timer 1
void TA1_0_IRQHandler(void){
    // clear hardware flag 
    Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);

    //gestione debounce: decrementa se maggiore di 0
    if (debounce_countdown > 0) {
        debounce_countdown--;
    }

    // timer_flag == true --> period of time has passed
    timer_flag = true;

    // implement normal mode interrupt functions
    // TODO!

    // Service watchdog timer
    WDT_A_clearTimer();
}

// Interrupt handler for Timer32_1 (3-second interval)
// This timer handles the updating of the values of the sensor (temperature_sensor_value, humidity_sensor_value)
// And the pump logic, both the long timers and the start/stop to prevent settings from messing up daily watering
void T32_INT2_IRQHandler(void){
    // Clear Timer32 interrupt flag
    Timer32_clearInterruptFlag(TIMER32_1_BASE);

    // Set flag to true; 
    // The real call of the sensor read is outside the interrupt, in automatic mode
    three_s_flag = true;
    
    // target_water_ml -> ml/day
    // 3s/cycle
    // 300ml/min -> 300/20 = 15ml/cycle
    // Supposing we water the plant every 12h (12*3600/3=14400 cycles)
    // Cycles/watering = target_water_ml/2/15
    // For 150ml, cycles = 150/30 = 50

    // Only perform this logic if the pump is not paused
    if (pump_timer_state) {
        pump_timer--;
        if (pump_timer <= 0){
            if (pump_is_watering) {
                // Case pump begins new wait period
                pump_state = false;
                // Wait 12h
                pump_timer = PUMP_TIMER_PERIOD;
            } else {
                // if target_water_ml is 0, skip directly. This ensures no unexpected behavior, such as pump turning on for a cycle
                if (target_water_ml < WATER_STEP){
                    pump_timer = PUMP_TIMER_PERIOD;
                    pump_is_watering = true;
                } else {
                    pump_state = true;
                    // Calculate every time in case this changed
                    pump_timer = target_water_ml / WATER_STEP;
                }
            }
            pump_is_watering = !pump_is_watering;
        }
    }

    // Service watchdog timer
    WDT_A_clearTimer();
}

// Interrupt handler for port 1
void PORT1_IRQHandler(void){
    //legge lo stato di chi ha generato l'interrupt su tutta la porta
    uint32_t status = GPIO_getEnabledInterruptStatus(GPIO_PORT_P1);

    //clear all active interrupt flags
    GPIO_clearInterruptFlag(GPIO_PORT_P1, status);

    //SE E SOLO SE countdown arrivato a 0, accettiamo altri input, altrimenti ignoriamo il rimbalzo meccanico
    if(debounce_countdown == 0){

        bool valid_press = false;

        // Use or to stack events when many happen at the same time
        if (status & B1_PIN) {
            button_events |= EVT_B1_PRESS;
            valid_press = true;
        }

        if (status & B2_PIN) {
            button_events |= EVT_B2_PRESS;
            valid_press = true;
        }

        if (status & B3_PIN){
            button_events |= EVT_B3_PRESS;
            valid_press = true;
        }

        //se abbiamo registrato una pressione valida, block for 50ms
        // 5 ticks * 10ms = 50ms
        if(valid_press){
            debounce_countdown = 5;
        }
    }
    // Service watchdog timer
    WDT_A_clearTimer();
}

void readSensors(void){
    DHT22_Data_t data;
    //attempt to read
    if (DHT22_Read(&data)){
       //check for corrupt data
        if (data.temperature > -40 && data.temperature < 80 &&
            data.humidity >= 0 && data.humidity <= 100) {
            temperature_sensor_value = data.temperature;
            humidity_sensor_value = data.humidity;
            dht22_error_flag = false;  //valid reading --> update global variables
        } else {
            dht22_error_flag = true;
        }
    }
    else {
        dht22_error_flag = true;
    }
}

// ---- Hardware start/stop functions ----
// Required for abstraction and to manage the board for the humidifier
// Also needed to turn on/off status led if we implement them

// Starts the humidifier circuit with a pulse
void startHum(void){
    // TODO!
    // Test is required here: a brief pulse should be enough to start the board
    // The module has two settings: continuous (first click) and alternate 10s on/5s off
    // https://ae01.alicdn.com/kf/S64754aa461d14f20ac57202706dfa4397.jpg
    // https://ae01.alicdn.com/kf/Scc9a0b2f94fb432ebde817d22de69baei.jpg
    GPIO_setOutputHighOnPin(HUMIDIFIER_PORT, HUMIDIFIER_PIN);
    Delay_ms(hum_pulse_duration_ms);
    GPIO_setOutputLowOnPin(HUMIDIFIER_PORT, HUMIDIFIER_PIN);
}

// Stops the humidifier with two pulses
void stopHum(void){
    GPIO_setOutputHighOnPin(HUMIDIFIER_PORT, HUMIDIFIER_PIN);
    Delay_ms(hum_pulse_duration_ms);
    GPIO_setOutputLowOnPin(HUMIDIFIER_PORT, HUMIDIFIER_PIN);
    Delay_ms(hum_pulse_duration_ms);
    GPIO_setOutputHighOnPin(HUMIDIFIER_PORT, HUMIDIFIER_PIN);
    Delay_ms(hum_pulse_duration_ms);
    GPIO_setOutputLowOnPin(HUMIDIFIER_PORT, HUMIDIFIER_PIN);
}

// Activates the water pump
void startPump(void){
    // L'implementazione precedente è inutile in quanto dead code
    // (vedi definizione di pump state) e logicamente sbagliata
    GPIO_setOutputHighOnPin(PUMP_PORT, PUMP_PIN);
}

// Deactivates the water pump
void stopPump(void){
    GPIO_setOutputLowOnPin(PUMP_PORT, PUMP_PIN);
}

// Activates the cooling fan
void startFan(void){
    GPIO_setOutputHighOnPin(FAN_PORT, FAN_PIN);
}

// Deactivates the cooling fan
void stopFan(void){
    GPIO_setOutputLowOnPin(FAN_PORT, FAN_PIN);
}

// Activates the heating resistor
void startResistor(void){
    GPIO_setOutputHighOnPin(RESISTOR_PORT, RESISTOR_PIN);
}

// Deactivates the heating resistor
void stopResistor(void){
    GPIO_setOutputLowOnPin(RESISTOR_PORT, RESISTOR_PIN);
}

// Checks the status of the manual/auto lever.
// Returns 1 if it is in auto, 0 if in manual.
// Defaults to 1 in case of reading error
bool checkLever(void){
    // TODO!
    return false;
}
