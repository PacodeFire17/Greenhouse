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

    void delay_us(uint32_t us) {
        // 48 MHz clock -> 48 ticks per us
        SysTick->LOAD = (us * 48) - 1;
        SysTick->VAL = 0;
        // Enable SysTick timer with Process Clock (Bit 2 = 1)
        SysTick->CTRL = SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_CLKSOURCE_Msk;
        while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0); // Wait for COUNTFLAG
        SysTick->CTRL = 0; // Disable SysTick timer
    }

    void delay_ms(uint32_t ms) {
        uint32_t i;
        for (i = 0; i < ms; i++) {
            delay_us(1000);
        }
    }


    int main(void){
        int pulse_duration_ms = 50;
        // Halt watchdog timer
        WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

        // Initialize Clock System to 48 MHz
        PCM_setCoreVoltageLevel(PCM_VCORE1);
        FlashCtl_setWaitState(FLASH_BANK0, 2);
        FlashCtl_setWaitState(FLASH_BANK1, 2);
        CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);
        CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
        CS_initClockSignal(CS_HSMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
        CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);

        // Output config
        GPIO_setAsOutputPin(HUMIDIFIER_PORT, HUMIDIFIER_SIGNAL_PIN_);
        // Turn off all
        GPIO_setOutputLowOnPin(HUMIDIFIER_PORT, HUMIDIFIER_SIGNAL_PIN);
        printf("Initialized, waiting 5s\n");
        delay_ms(5000);


        // --- CYCLING 5 times on/off ---
        int i;
        for (i = 0; i < 5; i++ ) {
            printf("STARTING CYCLE\n");

            // Turn on humidifier
            startHum();
            printf(" Should be on. \n Waiting 5 sec:\n");
            delay_ms(500);
            printf("Waited\nTurning off. \n");


            // Turn off humidifier
            stopHum();
            printf("Should be off now.\n Waiting 5 sec:\n");
            delay_ms(5000);

            printf("Waited\n");
        }
        printf("Finished tests\n");
    }
#endif
