#ifndef DHT22_H
#define DHT22_H

#include <stdint.h>
#include <stdbool.h>

// ==== FIXED POINT ARITHMETIC ==== 
//paranoia mia per non usare troppo spazio? forse, fatemi sapere

#define TO_FIXED(x)   ((int16_t)((x) * 10))
#define FROM_FIXED(x) ((float)(x) / 10.0f)

#define DHT22_TEMP_MIN TO_FIXED(-100.0) //-1000
#define DHT22_TEMP_MAX TO_FIXED(100.0)  //+1000

#define DHT22_HUM_MIN TO_FIXED(0.0)
#define DHT22_HUM_MAX TO_FIXED(100.0)

//stuct for DHT22
typedef struct 
{    
    int16_t humidity;       //fixed-point format (*10)
    int16_t temperature;    //fixed-point format (*10)
    bool valid;             //validity flag
} DHT22_Data_t;

bool DHT22_Init(void);
bool DHT22_Read(DHT22_Data_t *data);
void DHT22_PrintData(const DHT22_Data_t *data);

#endif