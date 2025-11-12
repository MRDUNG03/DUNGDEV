#ifndef LDR_H
#define LDR_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "hal_types.h"

// LDR ADC configuration
#define LDR_ADC_CHANNEL         HAL_ADC_CHANNEL_7  // P0.7
#define LDR_ADC_RESOLUTION      HAL_ADC_RESOLUTION_12
#define LDR_ADC_REFERENCE       HAL_ADC_REF_AVDD

// LDR constants for lux calculation
#define LDR_RESISTOR_VALUE      10000  // 10K ohm pull-up resistor
#define LDR_VCC_VOLTAGE         3300   // 3.3V supply voltage in mV

// Function prototypes
extern void LDR_Init(void);
extern uint16 LDR_ReadRaw(void);
extern uint16 LDR_ReadLux(void);
extern float LDR_ReadVoltage(void);

#ifdef __cplusplus
}
#endif

#endif /* LDR_H */
