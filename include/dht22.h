#ifndef DHT22_H
#define DHT22_H

#include <ti/devices/msp432p4xx/inc/msp.h>
#include <stdint.h>
#include <stdbool.h>

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
