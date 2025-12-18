#include <stdint.h>
#include <stdio.h>
#include <ti/devices/msp432p4xx/inc/msp.h>
#include "hardware.h"
#include "states.h"
#include "ui.h"
#include "dht22.h"

// Somehow these are required despite being defined in the .h file
#define DHT22_PORT P2
#define DHT22_PIN BIT5

// External variable provided by the CMSIS system file 
// it holds the current CPU frequency
extern uint32_t SystemCoreClock;

// Microsecond delay using SysTick
// Needed because DHT22 uses specific timing 
// Adapt automatically to the current CPU clock speed
static void Delay_us(uint32_t us) {
    // Calculate cycles needed 
    uint32_t cycles = (SystemCoreClock / 1000000) * us;

    // Configure SysTick for a one-shot delay
    SysTick->CTRL = 0;
    SysTick->LOAD = cycles;
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_CLKSOURCE_Msk;

    // wait for COUNTFLAG to be set (timer reached zero)
    while(!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk));

    SysTick->CTRL = 0; // Disable SysTick
}

static void Delay_ms(uint32_t ms) {
    for (uint32_t i = 0; i < ms; ++i)
    {
        Delay_us(1000);
    }
}


void DHT22_Init(void) {

    // Set pin as input
    DHT22_PORT->DIR &= ~DHT22_PIN;
    DHT22_PORT->REN |= DHT22_PIN; // Enable internal pull-up resistor 
    DHT22_PORT->OUT |= DHT22_PIN;   
}

bool DHT22_Read(DHT22_Data_t *data) {
    uint8_t bits[5] = {0};
    uint8_t idx = 0;
    uint32_t timeout;
    uint32_t pulse;

    // --- Start Signal ---
    // Host pulls low for at least 18ms
    DHT22_PORT->DIR |= DHT22_PIN;   // Output
    DHT22_PORT->OUT &= ~DHT22_PIN;  // Low
    Delay_ms(18);                // > 1ms (Datasheet: almeno 1ms, meglio 2ms per sicurezza)
    
    // Host pulls high for 30us then release line
    DHT22_PORT->OUT |= DHT22_PIN;   // High
    Delay_us(30);               // 20-40us
    DHT22_PORT->DIR &= ~DHT22_PIN;  // Input (passa controllo al sensore)

    // --- Disable Interrupts --- 
    __disable_irq();

    // --- Response Check ---
    // Wait low (80us)
    timeout = 1000; 
    while ((DHT22_PORT->IN & DHT22_PIN) && --timeout);
    if (timeout == 0) { __enable_irq(); return false; }

    // Wait high (80us)
    timeout = 1000;
    while (!(DHT22_PORT->IN & DHT22_PIN) && --timeout);
    if (timeout == 0) { __enable_irq(); return false; }

    // Wait low again (80us)
    timeout = 1000; 
    while ((DHT22_PORT->IN & DHT22_PIN) && --timeout);
    if (timeout == 0) { __enable_irq(); return false; }

    // --- Read Data 40 BIT ---
    for (int i = 0; i < 40; ++i)
    {
        // Wait for start of bit
        timeout = 1000;
        while(!(DHT22_PORT->IN & DHT22_PIN) && --timeout);
        if (timeout == 0){ __enable_irq(); return false;}

        // Measure the duration of the high state
        pulse = 0;  
        while((DHT22_PORT->IN & DHT22_PIN)){
            pulse++;
            if (pulse > 1000){ __enable_irq(); return false; }
        }

        // Determine bit value (0 or 1)
        // '0' is ~26-28us, '1' is ~70us.
        // The threshold depends on CPU speed. For 48MHz, a loop count > 30 
        // safely distinguishes the long pulse from the short one.
        idx = i / 8;
        bits[idx] <<= 1;
        if (pulseLen > 30) { 
            bits[idx] |= 1;
        }
    }

    // --- Enable Interrupts --- (critical section end)
    __enable_irq();

    // --- Checksum ---
    // Sum of the first 4 bytes must be equal to the 5th byte
        if ((uint8_t)(bits[0] + bits[1] + bits[2] + bits[3]) != bits[4]) {
        return false;
    }

    // --- Conversion to Fixed Point ---
    // Humidity: bits[0] and bits[1]
    data->humidity = (bits[0] << 8) | bits[1];

    // Temperature: bits[2] and bits[3]
    int16_t tempRaw = (bits[2] << 8) | bits[3];
    // Negative temperature
    if (tempRaw & 0x8000) {
        tempRaw &= 0x7FFF;
        data->temperature = -tempRaw;
    } else { data->temperature = tempRaw; }
    return true;
}
