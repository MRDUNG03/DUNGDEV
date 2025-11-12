#ifndef SHT30_H
#define SHT30_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "hal_types.h"

// SHT30 I2C address
#define SHT30_I2C_ADDR          0x44

// SHT30 commands
#define SHT30_CMD_MEASURE       0x2C06  // High repeatability measurement
#define SHT30_CMD_RESET         0x30A2  // Soft reset
#define SHT30_CMD_HEATER_ON     0x306D  // Enable heater
#define SHT30_CMD_HEATER_OFF    0x3066  // Disable heater

// Function prototypes
extern void SHT30_Init(void);
extern uint8 SHT30_ReadData(float *temperature, float *humidity);
extern uint8 SHT30_Reset(void);
extern uint8 SHT30_EnableHeater(void);
extern uint8 SHT30_DisableHeater(void);

#ifdef __cplusplus
}
#endif

#endif /* SHT30_H */
