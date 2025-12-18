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

void setInput(void){
    DHT22_PORT->DIR &= ~DHT22_PIN;
}

void setOutput(void){
    DHT22_PORT->DIR |= DHT22_PIN;
}

void setLow(void){
    DHT22_PORT->OUT &= ~DHT22_PIN
}

void setHigh(void){
    DHT22_PORT->OUT |= DHT22_PIN;
}


void Delay_ms(uint32_t ms) {

    // dichiarando la variabile fuori dal loop mi risolve un errore di compilazione, probabilmente prche' il mio CCS usa C89/C90
    // da controllare se sul vostro compare lo stesso errore rimettendo la dichiarazione all'interno e poi eliminare questo commento
    uint32_t i = 0;
    for (i = 0; i < ms; ++i)
    {
        Delay_us(1000);
    }
}


void DHT22_Init(void) {

    // Set pin as input
    setInput();
    DHT22_PORT->REN |= DHT22_PIN; // Enable internal pull-up resistor 
    setHigh();   
}

bool DHT22_Read(DHT22_Data_t *data) {
    uint8_t bits[5] = {0};
    uint8_t idx = 0;
    uint32_t timeout;
    uint32_t pulse;

    // --- Start Signal ---
    // Host pulls low for at least 1ms
    setOutput();
    setLow();
    Delay_ms(2);    // > 1ms (Datasheet: at least 1ms, better 2ms)
    
    // Host pulls high for 30us then release line
    setHigh();
    Delay_us(30);               // 20-40us
    setInput();  

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

    // --- Read Data 40 BIT ---

    //guardare commento in Delay_ms
    int i = 0;
    for (i = 0; i < 40; ++i)
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
        if (pulse > 30) { 
            bits[idx] |= 1;
        }
    }

    // --- Enable Interrupts --- (critical section end)
    __enable_irq();
    Delay_ms(5);

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
        tempRaw = -tempRaw;
    }

    // Divide by 10 to store data into an int8_t
    data->temperature = tempRaw / 10;
    data->humidity = (data->humidity) / 10;

    //Check if data is within expected range (invalid otherwise)
    if (data->temperature < 0 || data ->temperature > 100
            || data->humidity < 0 || data->humidity > 100) return false;

    return true;
}
