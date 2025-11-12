#include "sht30.h"
#include "hal_drivers.h"
#include "hal_mcu.h"
#include "OSAL.h"

// I2C function prototypes for SHT30
void HalI2CInit(void);
void HalI2CDisable(void);
uint8 HalI2CReceive(uint8 address, uint8 *pBuf, uint8 len);
uint8 HalI2CSend(uint8 address, uint8 *pBuf, uint8 len);

// CRC calculation for data validation
static uint8 SHT30_CalcCRC(uint8 *data, uint8 len);
static uint8 SHT30_CheckCRC(uint8 *data, uint8 len, uint8 crc);

void SHT30_Init(void) {
    // Initialize I2C interface
    HalI2CInit();
    
    // Wait for sensor to be ready
    MicroWait(1000);
    
    // Perform soft reset
    SHT30_Reset();
    
    // Wait after reset
    MicroWait(2000);
}

uint8 SHT30_ReadData(float *temperature, float *humidity) {
    uint8 writeBuffer[2];
    uint8 readBuffer[6];
    uint16 temp_raw, hum_raw;
    
    if(temperature == NULL || humidity == NULL) {
        return FAILURE;
    }
    
    // Prepare measurement command
    writeBuffer[0] = (SHT30_CMD_MEASURE >> 8) & 0xFF;   // Command high byte
    writeBuffer[1] = SHT30_CMD_MEASURE & 0xFF;          // Command low byte
    
    // Send measurement command
    if(HalI2CWrite(SHT30_I2C_ADDR, 2, writeBuffer) != SUCCESS) {
        return FAILURE;
    }
    
    // Wait for measurement completion (max 15ms for high repeatability)
    MicroWait(15000);
    
    // Read 6 bytes: temp_msb, temp_lsb, temp_crc, hum_msb, hum_lsb, hum_crc
    if(HalI2CRead(SHT30_I2C_ADDR, 6, readBuffer) != SUCCESS) {
        return FAILURE;
    }
    
    // Verify CRC for temperature data
    if(!SHT30_CheckCRC(&readBuffer[0], 2, readBuffer[2])) {
        return FAILURE;
    }
    
    // Verify CRC for humidity data
    if(!SHT30_CheckCRC(&readBuffer[3], 2, readBuffer[5])) {
        return FAILURE;
    }
    
    // Convert raw data
    temp_raw = (readBuffer[0] << 8) | readBuffer[1];
    hum_raw = (readBuffer[3] << 8) | readBuffer[4];
    
    // Calculate temperature in Celsius
    *temperature = -45.0 + 175.0 * ((float)temp_raw / 65535.0);
    
    // Calculate humidity in %RH
    *humidity = 100.0 * ((float)hum_raw / 65535.0);
    
    // Limit humidity to valid range
    if(*humidity > 100.0) {
        *humidity = 100.0;
    }
    if(*humidity < 0.0) {
        *humidity = 0.0;
    }
    
    return SUCCESS;
}

uint8 SHT30_Reset(void) {
    uint8 writeBuffer[2];
    
    writeBuffer[0] = (SHT30_CMD_RESET >> 8) & 0xFF;
    writeBuffer[1] = SHT30_CMD_RESET & 0xFF;
    
    if(HalI2CWrite(SHT30_I2C_ADDR, 2, writeBuffer) != SUCCESS) {
        return FAILURE;
    }
    
    // Wait for reset completion
    MicroWait(2000);
    
    return SUCCESS;
}

uint8 SHT30_EnableHeater(void) {
    uint8 writeBuffer[2];
    
    writeBuffer[0] = (SHT30_CMD_HEATER_ON >> 8) & 0xFF;
    writeBuffer[1] = SHT30_CMD_HEATER_ON & 0xFF;
    
    return HalI2CWrite(SHT30_I2C_ADDR, 2, writeBuffer);
}

uint8 SHT30_DisableHeater(void) {
    uint8 writeBuffer[2];
    
    writeBuffer[0] = (SHT30_CMD_HEATER_OFF >> 8) & 0xFF;
    writeBuffer[1] = SHT30_CMD_HEATER_OFF & 0xFF;
    
    return HalI2CWrite(SHT30_I2C_ADDR, 2, writeBuffer);
}

// CRC-8 calculation for SHT30 (polynomial: 0x31)
static uint8 SHT30_CalcCRC(uint8 *data, uint8 len) {
    uint8 crc = 0xFF;
    uint8 i, j;
    
    for(i = 0; i < len; i++) {
        crc ^= data[i];
        for(j = 0; j < 8; j++) {
            if(crc & 0x80) {
                crc = (crc << 1) ^ 0x31;
            } else {
                crc = crc << 1;
            }
        }
    }
    
    return crc;
}

static uint8 SHT30_CheckCRC(uint8 *data, uint8 len, uint8 crc) {
    return (SHT30_CalcCRC(data, len) == crc);
}
