#ifndef GP2Y_H
#define GP2Y_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "hal_types.h"

// GP2Y1010AU0F Pin definitions
#define GP2Y_LED_PIN            P1_0    // LED control pin
#define GP2Y_ADC_CHANNEL        HAL_ADC_CHANNEL_6  // P0.6 for analog input
#define GP2Y_ADC_RESOLUTION     HAL_ADC_RESOLUTION_12
#define GP2Y_ADC_REFERENCE      HAL_ADC_REF_AVDD

// GP2Y timing constants (microseconds)
#define GP2Y_SAMPLE_TIME        280   // Sampling time after LED on
#define GP2Y_DELTA_TIME         40    // Time between LED off and next cycle
#define GP2Y_SLEEP_TIME         9680  // Sleep time in cycle

// GP2Y constants for dust density calculation
#define GP2Y_NO_DUST_VOLTAGE    600   // No dust voltage in mV
#define GP2Y_VCC_VOLTAGE        3300  // Supply voltage in mV
#define GP2Y_DUST_FACTOR        0.17  // Dust density factor (mg/m3 per mV)

// Function prototypes
extern void GP2Y_Init(void);
extern uint16 GP2Y_ReadRaw(void);
extern float GP2Y_ReadVoltage(void);
extern float GP2Y_ReadDustDensity(void);

#ifdef __cplusplus
}
#endif

#endif /* GP2Y_H */
