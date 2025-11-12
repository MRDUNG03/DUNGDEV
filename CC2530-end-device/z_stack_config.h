#ifndef Z_STACK_CONFIG_H
#define Z_STACK_CONFIG_H

// Z-Stack Configuration for CC2530 End Device

// Device Configuration
#define ZG_DEVICE_ENDDEVICE                 1
#define ZSTACK_DEVICE_BUILD                 1
#define ENDDEVICE_ONLY                      1

// Network Configuration
#define DEFAULT_CHANLIST                    0x07FFF800  // Channels 11-26
#define ZDAPP_CONFIG_PAN_ID                 0xFFFF      // Any PAN ID
#define ZDAPP_CONFIG_EXTENDED_PAN_ID        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00

// Security Configuration
#define SECURE                              0
#define ZG_SECURE_ENABLED                   FALSE
#define ZG_GLOBAL_LINK_KEY                  TRUE

// Application Configuration
#define MT_TASK                             0
#define MT_APP_FUNC                         0
#define MT_SYS_FUNC                         0
#define MT_ZDO_FUNC                         0
#define MT_ZDO_MGMT                         0
#define MT_UTIL_FUNC                        0
#define MT_DEBUG_FUNC                       0
#define MT_AF_FUNC                          0
#define MT_SAPI_FUNC                        0
#define MT_UART_DEFAULT_BAUDRATE            HAL_UART_BR_115200

// Memory Configuration
#define MAXMEMHEAP                          4096
#define MAX_BUFFER_SIZE                     80
#define MAX_TRANSFER_SIZE                   160

// HAL Configuration
#define HAL_UART                            TRUE
#define HAL_UART_DMA                        1
#define HAL_UART_ISR                        2
#define HAL_ADC                             TRUE
#define HAL_DMA                             TRUE
#define HAL_FLASH                           TRUE
#define HAL_AES                             TRUE
#define HAL_AES_DMA                         TRUE
#define HAL_LCD                             FALSE
#define HAL_LED                             TRUE
#define HAL_KEY                             TRUE
#define HAL_TIMER                           TRUE
#define HAL_I2C                             TRUE

// Power Management
#define POWER_SAVING                        1
#define OSAL_CBTIMER_NUM_TASKS              1

// BDB Configuration
#define BDB_FINDING_BINDING_CAPABILITY_ENABLED     1
#define BDB_NETWORK_STEERING_CAPABILITY_ENABLED    1
#define BDB_NETWORK_FORMATION_CAPABILITY_ENABLED   0
#define BDB_TOUCHLINK_CAPABILITY_ENABLED           0

// ZCL Configuration
#define ZCL_READ                            1
#define ZCL_WRITE                           1
#define ZCL_BASIC                           1
#define ZCL_IDENTIFY                        1
#define ZCL_REPORTING_DEVICE                1

#endif // Z_STACK_CONFIG_H
