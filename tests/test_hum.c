#ifdef TEST_MODE
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

    const uint_fast8_t HUMIDIFIER_PORT_ =            GPIO_PORT_P4;
    const uint_fast16_t HUMIDIFIER_POWER_PIN_ =      GPIO_PIN3;
    const uint_fast16_t HUMIDIFIER_SIGNAL_PIN_ =     GPIO_PIN1;

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
        int pulse_duration_ms = 50;
        int pause_duration_ms = 1;
        // Halt watchdog timer
        WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

        // Output config
        GPIO_setAsOutputPin(HUMIDIFIER_PORT_, HUMIDIFIER_POWER_PIN_);
        GPIO_setAsOutputPin(HUMIDIFIER_PORT_, HUMIDIFIER_SIGNAL_PIN_);
        // Turn off all
        GPIO_setOutputLowOnPin(HUMIDIFIER_PORT_, HUMIDIFIER_POWER_PIN_);
        GPIO_setOutputLowOnPin(HUMIDIFIER_PORT_, HUMIDIFIER_SIGNAL_PIN_);



        // --- CYCLING 5 times on/off ---
        int i;
        for (i = 0; i < 5; i++ ) {
            printf("STARTING CYCLE\n");

            // Turn on humidifier
            GPIO_setOutputHighOnPin(HUMIDIFIER_PORT_, HUMIDIFIER_POWER_PIN_);
            delay_ms(1);
            GPIO_setOutputHighOnPin(HUMIDIFIER_PORT_, HUMIDIFIER_SIGNAL_PIN_);
            delay_ms(pulse_duration_ms);
            GPIO_setOutputLowOnPin(HUMIDIFIER_PORT_, HUMIDIFIER_SIGNAL_PIN_);
            printf(" Should be on. \n Waiting 5 sec:\n");
            delay_ms(5000);
            printf("Waited\nTurning off. \n");


            // Turn off humidifier
            GPIO_setOutputLowOnPin(HUMIDIFIER_PORT_, HUMIDIFIER_POWER_PIN_);
            printf("Should be off now.\n Waiting 5 sec:\n");
            delay_ms(5000);

            printf("Waited\n");
        }
        printf("Finished tests\n");
    }
#endif