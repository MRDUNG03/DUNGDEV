#include "hal_types.h"
#include "ioCC2530.h"
#include "sht30.h"
#include "hal_soft_i2c.h"
#include "hal_delay.h"

#define SHT30_ADDR         0x44 
#define SHT30_CMD_MEAS_H   0x2C
#define SHT30_CMD_MEAS_L   0x06

void SHT30_Init(void) {
    I2C_Init();
    delay_ms(50);
}

// Optimized version with minimal stack usage
uint8 sht30_read_temp_humi(float *temperature, float *humidity) {
   
    static uint8 txBuf[2];
    static uint8 rxBuf[6];
    uint16 rawTemp, rawHumi;
    
    txBuf[0] = SHT30_CMD_MEAS_H;
    txBuf[1] = SHT30_CMD_MEAS_L;
    
    if (I2C_Write(SHT30_ADDR, txBuf, 2) != 0) {
        return 1;
    }
    
    delay_ms(20);

    if (I2C_Read(SHT30_ADDR, rxBuf, 6) != 0) {
        return 2;
    }
    
    rawTemp = ((uint16)rxBuf[0] << 8) | rxBuf[1];
    rawHumi = ((uint16)rxBuf[3] << 8) | rxBuf[4];
    
    // Simplified calculation to reduce stack usage
    *temperature = -45.0f + (175.0f * rawTemp) / 65535.0f;
    *humidity = (100.0f * rawHumi) / 65535.0f;
    
    return 0;
}
