#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include "LcdDriver/Crystalfontz128x128_ST7735.h"
#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/grlib/grlib.h>
#include <stdint.h>
#include <stdio.h>

#include "hardware.h"
#include "states.h"


// ====== VARIABLES & CONSTANTS ======

State_t current_state = STATE_INIT;

// Dummy variables for now
int target_water_ml = 100;
int target_humidity_pct = 50;
int target_temp_c = 25;

#define WATER_MAX 500
#define HUM_MAX   100
#define TEMP_MAX  40


// ====== FUNCTIONS ======

void fn_INIT(){
        init();
        current_state = STATE_AUTOMATIC;
}


void fn_MANUAL(){
        manual();
        // Change from num_states to real state, used as a way to raise "not implemented" error
        current_state = NUM_STATES;
}

void fn_AUTOMATIC(){
        automatic();
        // Change from num_states to real state, used as a way to raise "not implemented" error
        current_state = NUM_STATES;
}


void manual(){
    // TODO!
}
// Renamed from "auto", probably a reserved word
void automatic(){
    // TODO!
}

void settings(){
    // TODO
}

// Dummy implementation of settings

void fn_SET_WATER(void){
    // TODO!
//    Mettere in pausa il timer dell’irrigazione
//    Interrompere resistenza e ventola finché non finiscono le impostazioni
//    Mostrare a schermo la quantità di acqua impostata attualmente (misura da definire, per ora int)
//    Leggi l’input dei bottoni: aumenta la quantità di acqua (B1) o diminuisci (B2), non cambiare se se il livello e a 0 o al massimo passa a IMPOSTAZIONI-UMIDITÀ se B3
//    Stampa periodicamente il livello attuale; stampa a schermo “OFF” o “MAX” se il livello e a 0 o al massimo, usa funzione dedicata (tipo printWaterSetting)

}


void fn_SET_HUMIDITY(void){
    // TODO!
//    Mostrare a schermo il livello di umidità target attuale(int)
//    Leggi l’input dei bottoni: aumenta la quantità di acqua (B1) o diminuisci (B2), non cambiare se se il livello e a 0 o al massimo passa a IMPOSTAZIONI-TEMPERATURA se B3
//    Stampa periodicamente il livello attuale; stampa a schermo “OFF” o “MAX” se il livello e a 0 o al massimo, usa funzione dedicata

}


void fn_SET_TEMP(void){
    // TODO!
//    Mostrare a schermo il livello di umidità target attuale(int)
//    Leggi l’input dei bottoni: aumenta la quantità di acqua (B1) o diminuisci (B2), non cambiare se se il livello e a 0 o al massimo passa a AUTO se B3
//    Stampa periodicamente il livello attuale; stampa a schermo “OFF” o “MAX” se il livello e a 0 o al massimo, usa funzione dedicata

}



