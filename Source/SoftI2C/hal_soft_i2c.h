#ifndef HAL_SOFT_I2C_H
#define HAL_SOFT_I2C_H

#include "hal_types.h"

// Function prototypes

/**
 * @brief Initialize I2C interface
 * Configures GPIO pins and sets initial state
 */
void I2C_Init(void);

/**
 * @brief Write data to I2C device
 * @param addr 7-bit device address
 * @param data Pointer to data buffer
 * @param len Number of bytes to write
 * @return 0 = success, 1 = address NACK, 2 = data NACK
 */
uint8 I2C_Write(uint8 addr, uint8 *data, uint8 len);

/**
 * @brief Read data from I2C device
 * @param addr 7-bit device address
 * @param data Pointer to receive buffer
 * @param len Number of bytes to read
 * @return 0 = success, 1 = address NACK
 */
uint8 I2C_Read(uint8 addr, uint8 *data, uint8 len);

/**
 * @brief Get current pin status for debugging
 * @return Bit 1: SDA state, Bit 0: SCL state
 */
uint8 I2C_GetPinStatus(void);

/**
 * @brief Check if I2C bus is ready
 * @return 1 = bus ready, 0 = bus busy or error
 */
uint8 I2C_BusReady(void);

/**
 * @brief Recover from bus lock-up condition
 * Generates clock pulses to release stuck slaves
 */
void I2C_BusRecover(void);

/**
 * @brief Scan for I2C device at specified address
 * @param addr 7-bit device address to scan
 * @return 1 = device found, 0 = no device
 */
uint8 I2C_ScanDevice(uint8 addr);

#endif
