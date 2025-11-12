#include "GP2Y.h"
#include "hal_adc.h"
#include "hal_mcu.h"
#include "OSAL.h"

// MicroWait function implementation
void MicroWait(uint16 microseconds) {
    // Simple delay implementation for microseconds
    // Clock frequency dependent - adjust for CC2530 32MHz
    volatile uint16 i;
    for (i = 0; i < microseconds * 8; i++) {
        // Each iteration ~0.125us at 32MHz
        __no_operation();
    }
}

void GP2Y_Init(void) {
    // Initialize ADC for dust sensor
    HalAdcInit();
    
    // Configure P0.6 as analog input for sensor output
    P0SEL |= BV(6);  // P0.6 as peripheral (ADC)
    P0DIR &= ~BV(6); // P0.6 as input
    
    // Configure P1.0 as digital output for LED control
    P1SEL &= ~BV(0); // P1.0 as GPIO
    P1DIR |= BV(0);  // P1.0 as output
    P1_0 = 0;        // LED off initially
    
    // Configure ADC reference
    HalAdcSetReference(GP2Y_ADC_REFERENCE);
}

uint16 GP2Y_ReadRaw(void) {
    uint16 adcValue;
    
    // Turn on LED
    P1_0 = 1;
    
    // Wait for sampling time (280 μs)
    MicroWait(GP2Y_SAMPLE_TIME);
    
    // Read ADC value
    adcValue = HalAdcRead(GP2Y_ADC_CHANNEL, GP2Y_ADC_RESOLUTION);
    
    // Wait for delta time (40 μs)
    MicroWait(GP2Y_DELTA_TIME);
    
    // Turn off LED
    P1_0 = 0;
    
    // Wait for sleep time (9.68 ms)
    MicroWait(GP2Y_SLEEP_TIME);
    
    return adcValue;
}

float GP2Y_ReadVoltage(void) {
    uint16 adcValue;
    float voltage;
    
    adcValue = GP2Y_ReadRaw();
    
    // Convert ADC value to voltage in mV
    // ADC resolution is 12-bit (0-4095)
    voltage = (float)adcValue * GP2Y_VCC_VOLTAGE / 4095.0;
    
    return voltage;
}

float GP2Y_ReadDustDensity(void) {
    float voltage;
    float dustDensity;
    
    // Take multiple samples and average
    voltage = 0;
    for(uint8 i = 0; i < 5; i++) {
        voltage += GP2Y_ReadVoltage();
    }
    voltage /= 5.0;
    
    // Calculate dust density in mg/m³
    if(voltage >= GP2Y_NO_DUST_VOLTAGE) {
        dustDensity = (voltage - GP2Y_NO_DUST_VOLTAGE) * GP2Y_DUST_FACTOR;
    } else {
        dustDensity = 0.0;
    }
    
    // Limit to reasonable range
    if(dustDensity > 500.0) {
        dustDensity = 500.0;
    }
    
    return dustDensity;
}
