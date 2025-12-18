#ifndef DHT22_H
#define DHT22_H

#include <ti/devices/msp432p4xx/inc/msp.h>
#include <stdint.h>
#include <stdbool.h>

// Hardware configuration
#define DHT22_PORT P2
#define DHT22_PIN BIT5

// ==== FIXED POINT ARITHMETIC ==== 
// Format: value * 10 (e.g. , 25.5 C -> 255)
#define TO_FIXED(x)   ((int16_t)((x) * 10))
#define FROM_FIXED(x) ((float)(x) / 10.0f)

//stuct for DHT22
typedef struct {    
    int16_t humidity;       // fixed-point format (*10)
    int16_t temperature;    // fixed-point format (*10)
} DHT22_Data_t;

// --- Public Function --- 
void DHT22_Init(void);
bool DHT22_Read(DHT22_Data_t *data);

#endif