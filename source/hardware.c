#ifdef TEST_MODE
    #include <stdint.h>
    #include <stdio.h>
    #include <stdbool.h>
    #include "hardware.h"
    #include "dht22.h"
    #include "states.h"
#else
    #include <ti/devices/msp432p4xx/driverlib/driverlib.h>
    #include "LcdDriver/Crystalfontz128x128_ST7735.h"
    #include <ti/devices/msp432p4xx/inc/msp.h>
    #include <ti/grlib/grlib.h>
    #include <stdint.h>
    #include <stdio.h>
    #include "hardware.h"   
    #include "dht22.h"
    #include "states.h"
#endif


// ====== VARIABLES & CONSTANTS ======

// Ports (absolutely arbitrary and to be redefined, except for buttons)
const uint_fast8_t B1_PORT =                    GPIO_PORT_P5;   // S1 button
const uint_fast8_t B2_PORT =                    GPIO_PORT_P3;   // S2 button
const uint_fast8_t B3_PORT =                    GPIO_PORT_P4;   // S3 button (joystick) (J1.5 = 4.1)
const uint_fast8_t FAN_PORT =                   GPIO_PORT_P2;
const uint_fast8_t PUMP_PORT =                  GPIO_PORT_P2;
const uint_fast8_t LEVER_PORT =                 GPIO_PORT_P6;
const uint_fast8_t RESISTOR_PORT =              GPIO_PORT_P3;
const uint_fast8_t HUMIDIFIER_PORT =            GPIO_PORT_P4;

// Pins (equally arbitrary)
const uint_fast16_t B1_PIN =                    GPIO_PIN1;  // S1 button
const uint_fast16_t B2_PIN =                    GPIO_PIN5;  // S2 button
const uint_fast16_t B3_PIN =                    GPIO_PIN1;  // S3 button (joystick)
const uint_fast16_t FAN_PIN =                   GPIO_PIN5;  
const uint_fast16_t PUMP_PIN =                  GPIO_PIN7;  // Changed to higher power pin
const uint_fast16_t LEVER_PIN =                 GPIO_PIN4;
const uint_fast16_t RESISTOR_PIN =              GPIO_PIN2;
const uint_fast16_t HUMIDIFIER_POWER_PIN =      GPIO_PIN3;  // Not used due to power issues
const uint_fast16_t HUMIDIFIER_SIGNAL_PIN =     GPIO_PIN7;  

// Status flags
bool fan_state =        false;
bool pump_state =       false;
bool resistor_state =   false;
bool humidifier_state = false;
bool pump_is_watering = false;
bool pump_timer_state = true;
bool previous_humidifier_state = false;
volatile bool dht22_error_flag = false;
volatile int16_t humidity_sensor_value =    25;
volatile int16_t temperature_sensor_value = 25;

//timer A1 count 100Hz (10ms)
#define TIMER_PERIOD 7500

// Cycles (3s) between watering in seconds. Currently set for 12h
#define PUMP_TIMER_PERIOD 14400

// whatever graphics context is
Graphics_Context g_sContext;

// Button handler
volatile uint8_t button_events = EVT_NONE;

// Timer flags 
volatile bool timer_flag = false;
volatile bool three_s_flag = false;

// Prevents multiple button presses
volatile uint8_t b1_debounce_countdown = 0; 
volatile uint8_t b2_debounce_countdown = 0;
volatile uint8_t b3_debounce_countdown = 0;

// Data storage declaration
#pragma NOINIT(settings_store)
Settings_t settings_store;

// Duration of a pulse to toggle humidifier status (ms) - minimum tested is 50, +1 just to be sure
const uint_fast8_t hum_pulse_duration_ms = 51;

// Dual purpose pump counter (coutns between watering cycles and for watering duration), wait before starting to prevent underflow during initialization
volatile uint16_t pump_timer = 10;

// ====== FUNCTIONS ======

void hwInit(void) {
    // Halt watchdog timer and disable interrupts 
    WDT_A_holdTimer();
    Interrupt_disableMaster();

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
    GPIO_setAsOutputPin(HUMIDIFIER_PORT, HUMIDIFIER_POWER_PIN);
    GPIO_setAsOutputPin(HUMIDIFIER_PORT, HUMIDIFIER_SIGNAL_PIN);
    GPIO_setOutputLowOnPin(HUMIDIFIER_PORT, HUMIDIFIER_POWER_PIN);
    GPIO_setOutputLowOnPin(HUMIDIFIER_PORT, HUMIDIFIER_SIGNAL_PIN);

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

    // B3 = P4.1
    GPIO_setAsInputPinWithPullUpResistor(B3_PORT, B3_PIN);
    GPIO_interruptEdgeSelect(B3_PORT, B3_PIN, GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_clearInterruptFlag(B3_PORT, B3_PIN);
    GPIO_enableInterrupt(B3_PORT, B3_PIN);  

    // Lever
    GPIO_setAsInputPinWithPullUpResistor(LEVER_PORT, LEVER_PIN);

    // Enable button interrupts (use NVIC interrupt numbers)
    Interrupt_enableInterrupt(INT_PORT5);
    Interrupt_enableInterrupt(INT_PORT3);
    Interrupt_enableInterrupt(INT_PORT4);

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
    Timer32_enableInterrupt(TIMER32_1_BASE);
    Interrupt_enableInterrupt(INT_T32_INT2);
    Timer32_startTimer(TIMER32_1_BASE, false);

    // Re-enable Interrupts and watchdog
    Interrupt_enableMaster();
    WDT_A_startTimer();
}

// Function to interrupt all active hardware
void pauseHw(void){
    fan_state = false;
    pump_state = false;
    resistor_state = false;
    humidifier_state = false;
    stopFan();
    stopResistor();
    stopPump();
    // Block pump timer
    pump_timer_state = false;
    // Should be last since it takes 2*hum_pulse_duration_ms
    if (humidifier_state == 1)         
        stopHum(); 
}

void resumeHw(void){
    // Updating will automatically resume everything
    updateHw();
    pump_timer_state = true;
}


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

    if (humidifier_state != previous_humidifier_state) {
        if (humidifier_state) {
            startHum();
        } else {
            stopHum();
        }
        previous_humidifier_state = humidifier_state;
    }

    // Service watchdog timer
    WDT_A_clearTimer();
}


// Full harware initialization function
void init(){
    fan_state =        false;
    pump_state =       false;
    resistor_state =   false;
    humidifier_state = false;
    pump_timer_state = true;
    humidity_sensor_value =    0;
    temperature_sensor_value = 0;
    graphicsInit();
    hwInit();
    updateHw();
}

// Interrupt handler for timer 1
void TA1_0_IRQHandler(void){
    // clear hardware flag 
    Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);

    // Debounce managing
    if (b1_debounce_countdown > 0) {
        b1_debounce_countdown--;
    }
    if (b2_debounce_countdown > 0) {
        b2_debounce_countdown--;
    }
    if (b3_debounce_countdown > 0) {
        b3_debounce_countdown--;
    }

    // timer_flag == true --> period of time has passed
    timer_flag = true;

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

    // Useful prints for debugging:

    // printf("[DEBUG] | Temp: %d | Hum: %d | State: %d | Lever: %d | Watering: %d | Pump timer: %5d | Pump timer state: %d |\n", 
    // temperature_sensor_value, humidity_sensor_value, current_state, checkLever(), pump_state ,pump_timer, pump_timer_state);

    // printf("[DEBUG] | Pump: %d | Hum: %d | Res: %d | Fan: %d | current/target temp: %d C/%d C | current/target hum: %d %%/%d %% |\n", 
    // pump_state, humidifier_state, resistor_state, fan_state, temperature_sensor_value, target_temp_c, humidity_sensor_value, target_humidity_pct);
    
    // target_water_ml -> ml/day
    // 3s/cycle
    // 300ml/min -> 300/20 = 15ml/cycle
    // Supposing we water the plant every 12h (12*3600/3=14400 cycles)
    // Cycles/watering = target_water_ml/2/15
    // For 150ml, cycles = 150/30 = 5

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

// Interrupt handler for port 4 (B3)
void PORT4_IRQHandler(void){
    uint32_t status = GPIO_getEnabledInterruptStatus(B3_PORT);
    GPIO_clearInterruptFlag(B3_PORT, status);

    // Accept input only if debounce countdown is 0
    if(b3_debounce_countdown == 0){
        // B3 is on P1.5
        if (status & B3_PIN){
            // printf("[ISR] Button 3 valid press\n");
            button_events |= EVT_B3_PRESS;
            // Start debounce lockout if valid press detected
            b3_debounce_countdown = 20;  // 20 ticks * 10ms = 200ms
        }
    }
    WDT_A_clearTimer();
}

// Interrupt handler for port 3 (B2)
void PORT3_IRQHandler(void){
    uint32_t status = GPIO_getEnabledInterruptStatus(B2_PORT);
    GPIO_clearInterruptFlag(B2_PORT, status);

    // Accept input only if debounce countdown is 0
    if(b2_debounce_countdown == 0){
        // B2 is on P3.5
        if (status & B2_PIN){
            // printf("[ISR] Button 2 valid press\n");
            button_events |= EVT_B2_PRESS;
            b2_debounce_countdown = 20; 
        }
    }
    WDT_A_clearTimer();
}

// Interrupt handler for port 5 (B1)
void PORT5_IRQHandler(void){
    uint32_t status = GPIO_getEnabledInterruptStatus(B1_PORT);
    GPIO_clearInterruptFlag(B1_PORT, status);

    // Accept input only if debounce countdown is 0
    if(b1_debounce_countdown == 0){
        // B1 is on P5.1
        if (status & B1_PIN){
            // printf("[ISR] Button 1 valid press\n");
            button_events |= EVT_B1_PRESS;
            b1_debounce_countdown = 20;
        }
    }
    WDT_A_clearTimer();
}

void readSensors(void){
    DHT22_Data_t data;
    //If value returned by DHT22_Read = false -> dht22_error_flag = true
    dht22_error_flag = !DHT22_Read(&data);
    if (!dht22_error_flag){
        temperature_sensor_value = data.temperature;
        humidity_sensor_value = data.humidity;
    }
//    printf("[HARDWARE] Reading sensor data: %3d, %3d. Error: %d\n", data.temperature, data.humidity, dht22_error_flag);
}

// ---- Hardware start/stop functions ----
// Required for abstraction and to manage the board for the humidifier

// Starts the humidifier circuit with a pulse
void startHum(void){
//    printf("[HARDWARE] Starting hum\n");
    // The module has two settings: continuous (first click) and alternate 10s on/5s off
    // https://ae01.alicdn.com/kf/S64754aa461d14f20ac57202706dfa4397.jpg
    // https://ae01.alicdn.com/kf/Scc9a0b2f94fb432ebde817d22de69baei.jpg
    WDT_A_holdTimer();
    Interrupt_disableMaster();
    GPIO_setOutputHighOnPin(HUMIDIFIER_PORT, HUMIDIFIER_SIGNAL_PIN);
    Delay_ms(hum_pulse_duration_ms);
    GPIO_setOutputLowOnPin(HUMIDIFIER_PORT, HUMIDIFIER_SIGNAL_PIN);
    Interrupt_enableMaster();
    WDT_A_startTimer();
}

// Stops the humidifier, ideally by cutting power, now by sending a stop signal
void stopHum(void){
//    printf("[HARDWARE] Stopping hum\n");
    WDT_A_holdTimer();
    Interrupt_disableMaster();

    GPIO_setOutputHighOnPin(HUMIDIFIER_PORT, HUMIDIFIER_SIGNAL_PIN);
    Delay_ms(hum_pulse_duration_ms);
    GPIO_setOutputLowOnPin(HUMIDIFIER_PORT, HUMIDIFIER_SIGNAL_PIN);
    Delay_ms(1);
    GPIO_setOutputHighOnPin(HUMIDIFIER_PORT, HUMIDIFIER_SIGNAL_PIN);
    Delay_ms(hum_pulse_duration_ms);
    GPIO_setOutputLowOnPin(HUMIDIFIER_PORT, HUMIDIFIER_SIGNAL_PIN);

    Interrupt_enableMaster();
    WDT_A_startTimer();
}

// Activates the water pump
// Reminder: this used to be swapped
void startPump(void){
    GPIO_setOutputLowOnPin(PUMP_PORT, PUMP_PIN);
}

// Deactivates the water pump
void stopPump(void){
    GPIO_setOutputHighOnPin(PUMP_PORT, PUMP_PIN);

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
// This defaults to AUTO if there is no lever   
bool checkLever(void){
    uint8_t lever_state = GPIO_getInputPinValue(LEVER_PORT, LEVER_PIN);
    // HIGH = auto (true), LOW = manual (false) 
    // When the lever closes/switches, it shorts the pin to GND and pin reads LOW.
    return (lever_state != 0); 
}

void sys_init_logic(void) {
    // Initialize Hardware
    hwInit();
    graphicsInit(); // Even if no screen, keep it to prevent errors

    // RESTORE SETTINGS
    // Check if magic number matches (meaning we wrote to it previously)
    if (settings_store.magic_number == 0xCAFEBABE) {
        // printf("SYSTEM: Restoring settings from memory...\n");
        target_temp_c = settings_store.stored_temp;
        target_humidity_pct = settings_store.stored_hum;
        target_water_ml = settings_store.stored_water;
    } else {
        // printf("SYSTEM: First boot (or power loss). Setting defaults.\n");
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

