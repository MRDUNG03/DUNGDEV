#ifndef Z_STACK_CONFIG_H
#define Z_STACK_CONFIG_H

// Z-Stack Configuration for CC2530 Coordinator

// Device Configuration
#define ZG_DEVICE_COORDINATOR               1
#define ZSTACK_DEVICE_BUILD                 1
#define COORDINATOR_ONLY                    1

// Network Configuration
#define DEFAULT_CHANLIST                    0x07FFF800  // Channels 11-26
#define ZDAPP_CONFIG_PAN_ID                 0xFFFF      // Any PAN ID
#define ZDAPP_CONFIG_EXTENDED_PAN_ID        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00

// Security Configuration
#define SECURE                              0
#define ZG_SECURE_ENABLED                   FALSE
#define ZG_GLOBAL_LINK_KEY                  TRUE

// Application Configuration
#define MT_TASK                             1
#define MT_APP_FUNC                         1
#define MT_SYS_FUNC                         1
#define MT_ZDO_FUNC                         1
#define MT_ZDO_MGMT                         1
#define MT_UTIL_FUNC                        1
#define MT_DEBUG_FUNC                       1
#define MT_AF_FUNC                          1
#define MT_SAPI_FUNC                        0
#define MT_UART_DEFAULT_BAUDRATE            HAL_UART_BR_115200

// Memory Configuration
#define MAXMEMHEAP                          12288
#define MAX_BUFFER_SIZE                     160
#define MAX_TRANSFER_SIZE                   320

// HAL Configuration
#define HAL_UART                            TRUE
#define HAL_UART_DMA                        1
#define HAL_UART_ISR                        2
#define HAL_ADC                             FALSE
#define HAL_DMA                             TRUE
#define HAL_FLASH                           TRUE
#define HAL_AES                             TRUE
#define HAL_AES_DMA                         TRUE
#define HAL_LCD                             FALSE
#define HAL_LED                             TRUE
#define HAL_KEY                             TRUE
#define HAL_TIMER                           TRUE
#define HAL_I2C                             FALSE

// Power Management
#define POWER_SAVING                        0
#define OSAL_CBTIMER_NUM_TASKS              6

// BDB Configuration
#define BDB_FINDING_BINDING_CAPABILITY_ENABLED     1
#define BDB_NETWORK_STEERING_CAPABILITY_ENABLED    1
#define BDB_NETWORK_FORMATION_CAPABILITY_ENABLED   1
#define BDB_TOUCHLINK_CAPABILITY_ENABLED           0

// ZCL Configuration
#define ZCL_READ                            1
#define ZCL_WRITE                           1
#define ZCL_BASIC                           1
#define ZCL_IDENTIFY                        1
#define ZCL_REPORTING_DEVICE                0

// Coordinator specific
#define NWK_AUTO_POLL                       FALSE
#define CONCENTRATOR_ENABLE                 TRUE
#define CONCENTRATOR_DISCOVERY_TIME         120
#define CONCENTRATOR_RADIUS                 0x0A
#define MAX_HOPS                            20
#define MAX_RREQ_ENTRIES                    16
#define APSC_ACK_WAIT_DURATION_POLLED       3000

// Network parameters
#define ZDNWKMGR_MIN_TRANSMISSIONS          3
#define MAX_NEIGHBOR_ENTRIES                24
#define MAX_RTG_ENTRIES                     60
#define NWK_MAX_BINDING_ENTRIES             8
#define APS_MAX_BINDING_ENTRIES             8

// Coordinator network formation
#define ZDAPP_CONFIG_SCAN_DURATION          4
#define DEFAULT_START_DELAY                 0
#define POLL_RATE                           2000

// Data handling
#define MAX_DATA_RETRIES                    3
#define DATA_TIMEOUT                        30000

#endif // Z_STACK_CONFIG_H
