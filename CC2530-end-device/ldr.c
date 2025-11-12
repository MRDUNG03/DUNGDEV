#include "ldr.h"
#include "hal_adc.h"
#include "hal_mcu.h"
#include "OSAL.h"

// Simple power function implementation for CC2530
float simple_pow(float base, float exp) {
    if (exp == 0) return 1.0;
    if (exp == 1) return base;
    if (exp == 2) return base * base;
    
    // For other exponents, use approximation
    float result = 1.0;
    if (exp > 0) {
        for (int i = 0; i < (int)exp; i++) {
            result *= base;
        }
    }
    return result;
}

// LDR characteristics for lux calculation
// These values need to be calibrated for specific LDR
#define LDR_DARK_RESISTANCE     1000000  // 1M ohm in complete darkness
#define LDR_LIGHT_RESISTANCE    1000     // 1K ohm in bright light
#define LDR_GAMMA              0.7       // Gamma correction factor

void LDR_Init(void) {
    // Configure ADC for LDR reading
    HalAdcInit();
    
    // Configure P0.7 as analog input
    P0SEL |= BV(7);  // P0.7 as peripheral (ADC)
    P0DIR &= ~BV(7); // P0.7 as input
    
    // Configure ADC channel
    HalAdcSetReference(LDR_ADC_REFERENCE);
}

uint16 LDR_ReadRaw(void) {
    uint16 adcValue;
    
    // Read ADC value
    adcValue = HalAdcRead(LDR_ADC_CHANNEL, LDR_ADC_RESOLUTION);
    
    return adcValue;
}

float LDR_ReadVoltage(void) {
    uint16 adcValue;
    float voltage;
    
    adcValue = LDR_ReadRaw();
    
    // Convert ADC value to voltage
    // ADC resolution is 12-bit (0-4095)
    voltage = (float)adcValue * LDR_VCC_VOLTAGE / 4095.0;
    
    return voltage;
}

uint16 LDR_ReadLux(void) {
    uint16 adcValue;
    float voltage;
    float ldrResistance;
    float lux;
    
    // Read ADC value
    adcValue = LDR_ReadRaw();
    
    // Avoid division by zero
    if(adcValue == 0) {
        return 0;
    }
    
    // Convert to voltage
    voltage = (float)adcValue * LDR_VCC_VOLTAGE / 4095.0;
    
    // Calculate LDR resistance using voltage divider
    // Vout = Vin * R_LDR / (R_pullup + R_LDR)
    // R_LDR = R_pullup * Vout / (Vin - Vout)
    if(voltage >= LDR_VCC_VOLTAGE) {
        ldrResistance = LDR_LIGHT_RESISTANCE;
    } else {
        ldrResistance = LDR_RESISTOR_VALUE * voltage / (LDR_VCC_VOLTAGE - voltage);
    }
    
    // Ensure minimum resistance
    if(ldrResistance < LDR_LIGHT_RESISTANCE) {
        ldrResistance = LDR_LIGHT_RESISTANCE;
    }
    
    // Convert resistance to lux using empirical formula
    // This is a simplified calculation and may need calibration
    // lux = (R_dark / R_ldr)^gamma * scale_factor
    lux = simple_pow(LDR_DARK_RESISTANCE / ldrResistance, LDR_GAMMA) * 10.0;
    
    // Limit lux to reasonable range
    if(lux > 50000) {
        lux = 50000;
    }
    
    return (uint16)lux;
}
