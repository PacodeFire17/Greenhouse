#ifndef DHT22_H
#define DHT22_H

#ifdef TEST_MODE
    #include <stdint.h>
    #include <stdio.h>
    #include <stdbool.h>
#else
    #include <ti/devices/msp432p4xx/driverlib/driverlib.h>
    #include "LcdDriver/Crystalfontz128x128_ST7735.h"
    #include <ti/devices/msp432p4xx/inc/msp.h>
    #include <ti/grlib/grlib.h>
    #include <stdint.h>
    #include <stdio.h>
    #include <stdbool.h>
#endif

// Hardware configuration
#define DHT22_PORT P2
#define DHT22_PIN BIT5

//DHT22 Data structure
typedef struct {    
    int8_t humidity;       // fixed-point format
    int8_t temperature;    // fixed-point format
} DHT22_Data_t;

// --- Public Function --- 
void Delay_ms(uint32_t ms);
void DHT22_Init(void);
bool DHT22_Read(DHT22_Data_t *data);


#endif
