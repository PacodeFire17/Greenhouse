#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "hardware.h"
#include <dht22.h>      
#include "states.h"   
#include "ui.h"     

//  Implement global variables (MOCK STORAGE) 
// here I recreate the variables just for the test
bool fan_state = false;
bool pump_state = false;
bool resistor_state = false;
bool humidifier_state = false;
int16_t humidity_sensor_value = 0;
int16_t temperature_sensor_value = 0;
volatile uint8_t button_events = EVT_NONE;
volatile bool three_s_flag = false;

// Mock extra for the test
bool mock_lever_position = true; // true = AUTO

// Implement MOCK function 
// this function replace the function in hardware.h and ui.h 

void init(void) { printf("   [MOCK HW] Init sistema.\n"); }
void hwInit(void) { }
void graphicsInit(void) { }

void startFan(void) { 
    if(!fan_state) printf("\033[0;32m   [OUT] >> Ventola ACCESA\033[0m\n"); 
    fan_state = true; 
}
void stopFan(void) { 
    if(fan_state) printf("\033[0;31m   [OUT] >> Ventola SPENTA\033[0m\n"); 
    fan_state = false; 
}
void startPump(void) { pump_state = true; printf("   [OUT] >> Pompa ON\n"); }
void stopPump(void) { pump_state = false; printf("   [OUT] >> Pompa OFF\n"); }
void startHum(void) { humidifier_state = true; printf("   [OUT] >> Hum ON\n"); }
void stopHum(void) { humidifier_state = false; printf("   [OUT] >> Hum OFF\n"); }
void startResistor(void) { resistor_state = true; printf("   [OUT] >> Res ON\n"); }
void stopResistor(void) { resistor_state = false; printf("   [OUT] >> Res OFF\n"); }

void updateHw(void) {
    if(fan_state) startFan(); else stopFan();
    // ecc...
}

void pauseHw(void) { printf("   [SYS] Hardware Paused.\n"); }
void resumeHw(void) { printf("   [SYS] Hardware Resumed.\n"); }

bool checkLever(void) { return mock_lever_position; }

void readSensors(void) {
    printf("   [SENS] Lettura simulata: T=%d, H=%d\n", temperature_sensor_value, humidity_sensor_value);
}

// UI Mock
void printCurrentHardware(Hardware h) { printf("   [LCD] Hardware Selected: %d\n", h); }
void printSensorData(int t, int h) {} 
void printWaterSettings(int v) { printf("   [LCD] Water Set: %d\n", v); }
void printHumSettings(int v) { printf("   [LCD] Hum Set: %d\n", v); }
void printTempSettings(int v) { printf("   [LCD] Temp Set: %d\n", v); }


// === UTILITY TEST ===
void assert_true(bool condition, const char* message) {
    if(condition) printf("\033[0;32m[PASS] %s\033[0m\n", message);
    else {
        printf("\033[0;31m[FAIL] %s\033[0m\n", message);
        exit(1); // Ferma il test se fallisce
    }
}

// Helper function for resetting context, rendering tests self-contained
void reset_manual_context(void) {
    fan_state = false;
    pump_state = false;
    humidifier_state = false;
    resistor_state = false;

    button_events = 0;
    current_hw = PUMP;
}

// === MAIN TEST === 
int main(void) {
    printf("=== AVVIO TEST SUITE (Linux/GCC Architecture) ===\n");

    // ================= AUTOMATIC =================
    printf("\n--- Test 1: Controllo Temperatura ---\n");
    current_state = STATE_AUTOMATIC;
    target_temp_c = 25;
    fan_state = false;

    // test high temperature
    temperature_sensor_value = 30; 
    three_s_flag = true; 
    automatic(); 
    assert_true(fan_state == true, "Ventola accesa per T > Target");

    // ================= SETTINGS =================
    printf("\n--- Test 2: Cambio Acqua (Menu) ---\n");
    current_state = STATE_SET_WATER;
    target_water_ml = 100;
    
    printf("[DEBUG] Valore PRE-funzione: %d ml (Atteso: 100m)l\n", target_water_ml);

    button_events = EVT_B2_PRESS;
    fn_SET_WATER();     

    assert_true(target_water_ml == 110, "Acqua incrementata di 10ml");

    // ================= MANUAL =================
    printf("\n--- Test 3: Manual PUMP ON ---\n");
    reset_manual_context();
    current_hw = PUMP;
    button_events = EVT_B2_PRESS;
    manual();
    assert_true(pump_state == true, "Pump ON with B2");

    printf("\n--- Test 4: Manual PUMP -> FAN ---\n");
    reset_manual_context();
    current_hw = PUMP;
    pump_state = true;
    button_events = EVT_B1_PRESS;
    manual();
    assert_true(current_hw == FAN, "Switched to FAN");
    assert_true(pump_state == false, "Pump OFF on switch");


    // ================= STATE TRANSITIONS =================

    // ====== LEVER TRANSITIONS ======
    printf("\n--- Test 5: Lever MANUAL -> AUTOMATIC ---\n");
    current_state = STATE_MANUAL;
    mock_lever_position = true;   // AUTO
    lever_status_set();
    assert_true(current_state == STATE_AUTOMATIC,"Lever switches MANUAL -> AUTOMATIC");

    printf("\n--- Test 6: Lever AUTOMATIC -> MANUAL ---\n");
    current_state = STATE_AUTOMATIC;
    mock_lever_position = false;  // MANUAL
    lever_status_set();
    assert_true(current_state == STATE_MANUAL, "Lever switches AUTOMATIC -> MANUAL");

    // ====== SETTINGS MENU TRANSITIONS ======
    printf("\n--- Test 7: STATE_SET_WATER -> STATE_SET_HUM ---\n");
    reset_manual_context();
    current_state = STATE_SET_WATER;
    button_events = EVT_B1_PRESS;
    fn_SET_WATER();
    assert_true(current_state == STATE_SET_HUM, "SET_WATER -> SET_HUM via B1");

    printf("\n--- Test 8: STATE_SET_HUM -> STATE_SET_TEMP ---\n");
    reset_manual_context();
    current_state = STATE_SET_HUM;
    button_events = EVT_B1_PRESS;
    fn_SET_HUMIDITY();
    assert_true(current_state == STATE_SET_TEMP, "SET_HUM -> SET_TEMP via B1");

    printf("\n--- Test 9: SET_TEMP -> AUTO ---\n");
    reset_manual_context();
    current_state = STATE_SET_TEMP;
    mock_lever_position = true;  // AUTO
    button_events = EVT_B1_PRESS;
    fn_SET_TEMP();
    assert_true(current_state == STATE_AUTOMATIC, "SET_TEMP -> AUTOMATIC via B1");

    printf("\n--- Test 10: SET_TEMP -> MANUAL ---\n");
    reset_manual_context();
    current_state = STATE_SET_TEMP;
    mock_lever_position = false;  // MANUAL
    button_events = EVT_B1_PRESS;
    fn_SET_TEMP();
    assert_true(current_state == STATE_MANUAL,"SET_TEMP -> MANUAL");

    // ================= END =================
    printf("\n=== TUTTI I TEST PASSATI ===\n");
    return 0;
}
