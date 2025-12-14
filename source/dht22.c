#include <stdint.h>
#include <stdio.h>
#include "hardware.h"
#include "states.h"
#include "ui.h"
#include "dht22.h"

// Helper: Delay bloccanti usando SysTick
// Necessario perché il DHT22 richiede timing in us precisi
static void SysTick_Init_Delay(void) {
    SysTick->CTRL = 0;
    SysTick->LOAD = 0x00FFFFFF;
    SysTick->VAL = 0;
    SysTick->CTRL = 0x00000005; // Clock CPU, no interrupt
}

static void Delay_us(uint32_t us) {
    // Tuning per 3MHz (default clock dopo reset). 
    // Se hai alzato il clock a 48MHz, moltiplica * 48
    SysTick->LOAD = (us * 3) - 1; 
    SysTick->VAL = 0;
    while ((SysTick->CTRL & 0x00010000) == 0);
}

static void Delay_ms(uint32_t ms) {
    uint32_t i;
    for (i = 0; i < ms; i++) {
        Delay_us(1000);
    }
}

void DHT22_Init(void) {
    SysTick_Init_Delay();
    
    // Configura P2.5 come output alto (Idle state) con pull-up interno
    // NOTA: Usa una resistenza esterna da 4.7k tra VCC e DATA per stabilità
    DHT_PORT->DIR |= DHT_PIN;
    DHT_PORT->OUT |= DHT_PIN;
    DHT_PORT->REN |= DHT_PIN; // Abilita resistenza interna
}

bool DHT22_Read(DHT22_Data_t *data) {
    uint8_t bits[5] = {0};
    uint32_t timeout;
    uint32_t pulseWidth;
    int i, j;
    
    data->valid = false;

    // --- START SIGNAL ---
    DHT_PORT->DIR |= DHT_PIN;   // Output
    DHT_PORT->OUT &= ~DHT_PIN;  // Low
    Delay_ms(2);                // > 1ms (Datasheet: almeno 1ms, meglio 2ms per sicurezza)
    
    DHT_PORT->OUT |= DHT_PIN;   // High
    Delay_us(30);               // 20-40us
    
    DHT_PORT->DIR &= ~DHT_PIN;  // Input (passa controllo al sensore)

    // --- RESPONSE CHECK ---
    // Attendi low (80us)
    timeout = 1000; while ((DHT_PORT->IN & DHT_PIN) && --timeout);
    if (timeout == 0) return false;

    // Attendi high (80us)
    timeout = 1000; while (!(DHT_PORT->IN & DHT_PIN) && --timeout);
    if (timeout == 0) return false;

    // Attendi fine preambolo (low 50us prima del primo bit)
    timeout = 1000; while ((DHT_PORT->IN & DHT_PIN) && --timeout);
    if (timeout == 0) return false;

    // --- LETTURA 40 BIT ---
    for (j = 0; j < 5; j++) {
        uint8_t result = 0;
        for (i = 0; i < 8; i++) {
            // Attesa inizio bit (fine dello stato low di 50us)
            timeout = 1000;
            while (!(DHT_PORT->IN & DHT_PIN) && --timeout);
            if (timeout == 0) return false;

            // Misura durata stato alto
            pulseWidth = 0;
            while ((DHT_PORT->IN & DHT_PIN)) {
                pulseWidth++;
                if (pulseWidth > 1000) return false; // Timeout
            }

            // Shift e decisione bit
            // Soglia empirica: a 3MHz, un '0' (~26us) dura pochi cicli, un '1' (~70us) molti di più.
            // La soglia '3' deriva dal test pratico precedente.
            result <<= 1;
            if (pulseWidth > 3) {
                result |= 1;
            }
        }
        bits[j] = result;
    }

    // --- CHECKSUM ---
    // Somma dei primi 4 byte deve essere uguale al 5°
    if ((uint8_t)(bits[0] + bits[1] + bits[2] + bits[3]) != bits[4]) {
        return false;
    }

    // --- CONVERSIONE (Fixed Point) ---
    // Umidità: bits[0] e bits[1]
    data->humidity = (bits[0] << 8) | bits[1];

    // Temperatura: bits[2] e bits[3]
    int16_t tempRaw = (bits[2] << 8) | bits[3];
    
    // Gestione temperatura negativa (Bit 15 è il segno nel DHT22)
    if (tempRaw & 0x8000) {
        tempRaw &= 0x7FFF;
        data->temperature = -tempRaw;
    } else {
        data->temperature = tempRaw;
    }

    data->valid = true;
    return true;
}