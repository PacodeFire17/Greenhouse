#ifdef TEST_MODE
    #include <ti/devices/msp432p4xx/driverlib/driverlib.h>
    #include "LcdDriver/Crystalfontz128x128_ST7735.h"
    #include <ti/devices/msp432p4xx/inc/msp.h>
    #include <ti/grlib/grlib.h>

    #include <stdint.h>
    #include <stdio.h>
    #include "hardware.h"
    #include "states.h"

// Ports
const uint_fast8_t FAN_PORT =                   GPIO_PORT_P2;
const uint_fast8_t PUMP_PORT =                  GPIO_PORT_P2;
const uint_fast8_t LEVER_PORT =                 GPIO_PORT_P6;
const uint_fast8_t SWITCH_PORT =                GPIO_PORT_P2;
const uint_fast8_t RESISTOR_PORT =              GPIO_PORT_P3;

// Pins (equally arbitrary)
const uint_fast16_t FAN_PIN =                   GPIO_PIN7;
const uint_fast16_t PUMP_PIN =                  GPIO_PIN1;
const uint_fast16_t LEVER_PIN =                 GPIO_PIN4;
const uint_fast16_t SWITCH_PIN =                GPIO_PIN2;
const uint_fast16_t RESISTOR_PIN =              GPIO_PIN2;

    void delay_us(uint32_t us) {
        SysTick->LOAD = (us * 3) - 1; // 3 MHz (MSP432 default clock speed)
        SysTick->VAL = 0;
        SysTick->CTRL |= 0x00000001; // Enable SysTick timer
        while ((SysTick->CTRL & 0x00010000) == 0); // Wait for COUNTFLAG
        SysTick->CTRL &= ~0x00000001; // Disable SysTick timer
    }

    void delay_ms(uint32_t ms) {
        uint32_t i;
        for (i = 0; i < ms; i++) {
            delay_us(1000);
        }
    }


    int main(void){
        hwInit();

        // --- FAN TESTS ---
        int i;
        int delay = 5000;

        for (i = 0; i < 5; i++) {
            printf("FAN CYCLE %d\n", i + 1);
            printf("Starting fan\n");
            startFan();
            delay_ms(delay);
            printf("Stopping fan\n");
            stopFan();
            delay_ms(delay);
        }

        // --- PUMP TESTS ---
        for (i = 0; i < 5; i++) {
            printf("PUMP CYCLE %d\n", i + 1);
            printf("Starting pump\n");
            startPump();
            delay_ms(delay);
            printf("Stopping pump\n");
            stopPump();
            delay_ms(delay);
        }

        // --- RESISTOR TESTS ---
        for (i = 0; i < 5; i++) {
            printf("RESISTOR CYCLE %d\n", i + 1);
            printf("Turning resistor on\n");
            turnOnResistor();
            delay_ms(delay);
            printf("Turning resistor off\n");
            turnOffResistor();
            delay_ms(delay);
        }

        printf("Finished all hardware tests\n");
    }
#endif