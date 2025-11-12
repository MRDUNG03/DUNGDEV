#include "hal_soft_i2c.h"
#include "hal_types.h"
#include "hal_delay.h"
#include <ioCC2530.h>

#define SDA_PIN    P1_4
#define SCL_PIN    P1_5

#ifndef BV
#define BV(x) (1 << (x))
#endif

// Fixed I2C macros - operation order is critical for proper I2C timing
#define SDA_HIGH()    do { P1DIR &= ~BV(4); P1_4 = 1; } while(0)  // Set input mode first, then pull high
#define SDA_LOW()     do { P1_4 = 0; P1DIR |= BV(4); } while(0)   // Set low value, then output mode
#define SCL_HIGH()    do { P1DIR &= ~BV(5); P1_5 = 1; } while(0)  // Set input mode first, then pull high
#define SCL_LOW()     do { P1_5 = 0; P1DIR |= BV(5); } while(0)   // Set low value, then output mode
#define SDA_READ()    (P1_4)

// Improved delay function for stable I2C timing
static void I2C_Delay(void) {
    // Optimized delay with NOP instructions for precise timing
    volatile uint8 i = 50;  // Reduced iterations but with better timing control
    while(i--) {
        __asm("nop");
        __asm("nop");
    }
}

void I2C_Init(void) {
    // Configure pins for GPIO function and enable pull-up resistors
    P1SEL &= ~(BV(4) | BV(5));  // Set GPIO function (not peripheral)
    P1INP &= ~(BV(4) | BV(5));  // Enable pull-up/pull-down resistors
    P2INP &= ~BV(5);            // Configure Port 1 for pull-up mode
    
    // Set initial idle state - both lines high
    SDA_HIGH();
    SCL_HIGH();
    I2C_Delay();
}

static void I2C_Start(void) {
    // Generate I2C START condition: SDA goes low while SCL is high
    SDA_HIGH();     // Ensure SDA is high
    I2C_Delay();
    SCL_HIGH();     // Ensure SCL is high
    I2C_Delay();
    SDA_LOW();      // SDA goes low while SCL is high = START condition
    I2C_Delay();
    SCL_LOW();      // Pull SCL low to begin data transmission
    I2C_Delay();
}

static void I2C_Stop(void) {
    // Generate I2C STOP condition: SDA goes high while SCL is high
    SDA_LOW();      // Ensure SDA is low
    I2C_Delay();
    SCL_HIGH();     // Release SCL (goes high)
    I2C_Delay();
    SDA_HIGH();     // SDA goes high while SCL is high = STOP condition
    I2C_Delay();
}

static uint8 I2C_WriteByte(uint8 data) {
    uint8 i;
    uint8 ack;
    
    // Send 8 bits, MSB first
    for (i = 0; i < 8; i++) {
        SCL_LOW();              // Ensure clock is low before changing data
        I2C_Delay();
        
        // Set data bit on SDA line
        if (data & 0x80)        // Check MSB (bit 7)
            SDA_HIGH();
        else
            SDA_LOW();
        
        I2C_Delay();            // Setup time for data
        SCL_HIGH();             // Clock high to latch data
        I2C_Delay();            // Hold time for data
        
        data <<= 1;             // Shift to next bit
    }
    
    // Read ACK bit from slave
    SCL_LOW();                  // Pull clock low
    I2C_Delay();
    SDA_HIGH();                 // Release SDA line for slave to control
    I2C_Delay();
    SCL_HIGH();                 // Clock high for ACK bit
    I2C_Delay();
    ack = !SDA_READ();          // ACK = 0 (slave pulls SDA low), NACK = 1
    SCL_LOW();                  // Pull clock low
    I2C_Delay();
    
    return ack;                 // Return 1 for ACK, 0 for NACK
}

static uint8 I2C_ReadByte(uint8 ack) {
    uint8 data = 0;
    uint8 i;
    
    SDA_HIGH();                 // Release SDA line for slave to control
    
    // Read 8 bits from slave
    for (i = 0; i < 8; i++) {
        SCL_LOW();              // Pull clock low
        I2C_Delay();
        SCL_HIGH();             // Clock high to read data
        I2C_Delay();
        
        data <<= 1;             // Shift previous bits left
        if (SDA_READ())         // Read current bit
            data |= 0x01;       // Set LSB if SDA is high
    }
    
    // Send ACK/NACK to slave
    SCL_LOW();                  // Pull clock low
    I2C_Delay();
    
    if (ack)
        SDA_LOW();              // Send ACK (pull SDA low)
    else
        SDA_HIGH();             // Send NACK (keep SDA high)
    
    I2C_Delay();
    SCL_HIGH();                 // Clock high for ACK/NACK
    I2C_Delay();
    SCL_LOW();                  // Pull clock low
    I2C_Delay();
    SDA_HIGH();                 // Release SDA line
    
    return data;
}

uint8 I2C_Write(uint8 addr, uint8 *data, uint8 len) {
    uint8 i;
    
    I2C_Start();                // Generate START condition
    
    // Send 7-bit address with write bit (0)
    if (!I2C_WriteByte(addr << 1)) {
        I2C_Stop();             // Generate STOP if address not acknowledged
        return 1;               // Return error code: Address NACK
    }
    
    // Send data bytes
    for (i = 0; i < len; i++) {
        if (!I2C_WriteByte(data[i])) {
            I2C_Stop();         // Generate STOP if data not acknowledged
            return 2;           // Return error code: Data NACK
        }
    }
    
    I2C_Stop();                 // Generate STOP condition
    return 0;                   // Return success
}

uint8 I2C_Read(uint8 addr, uint8 *data, uint8 len) {
    uint8 i;
    
    I2C_Start();                // Generate START condition
    
    // Send 7-bit address with read bit (1)
    if (!I2C_WriteByte((addr << 1) | 1)) {
        I2C_Stop();             // Generate STOP if address not acknowledged
        return 1;               // Return error code: Address NACK
    }
    
    // Read data bytes
    for (i = 0; i < len; i++) {
        // Send ACK for all bytes except the last one (send NACK for last byte)
        data[i] = I2C_ReadByte(i < (len - 1));
    }
    
    I2C_Stop();                 // Generate STOP condition
    return 0;                   // Return success
}

// Debug function to check pin status
uint8 I2C_GetPinStatus(void) {
    // Return current state of SDA and SCL pins
    // Bit 1: SDA state, Bit 0: SCL state
    return (SDA_READ() << 1) | (P1_5);
}

// Function to check if I2C bus is ready for communication
uint8 I2C_BusReady(void) {
    // Both SDA and SCL should be high when bus is idle
    return (SDA_READ() && P1_5);
}

// Function to recover from bus lock-up condition
void I2C_BusRecover(void) {
    uint8 i;
    
    // Generate clock pulses to release any stuck slave
    SDA_HIGH();                 // Release SDA
    for (i = 0; i < 9; i++) {   // Send up to 9 clock pulses
        SCL_LOW();
        I2C_Delay();
        SCL_HIGH();
        I2C_Delay();
        if (SDA_READ()) {       // If SDA is released, stop
            break;
        }
    }
    
    // Generate STOP condition to reset bus state
    I2C_Stop();
}

// Function to scan for I2C devices (useful for debugging)
uint8 I2C_ScanDevice(uint8 addr) {
    uint8 result;
    
    I2C_Start();
    result = I2C_WriteByte(addr << 1);  // Try to write to device
    I2C_Stop();
    
    return result;              // Return 1 if device responds, 0 if no response
}
