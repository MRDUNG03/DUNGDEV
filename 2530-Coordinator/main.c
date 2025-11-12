#include "OSAL.h"
#include "OSAL_PwrMgr.h"
#include "ZComDef.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"
#include "ZDConfig.h"
#include "zcl.h"
#include "zcl_general.h"
#include "bdb.h"
#include "bdb_interface.h"
#include "gp_interface.h"
#include "hal_drivers.h"
#include "hal_key.h"
#include "hal_led.h"
#include "hal_uart.h"
#include "CoordinatorApp.h"

// Application Task ID
uint8 CoordinatorApp_TaskID;

// Application events
#define COORD_DATA_PROCESS_EVENT     0x0001
#define COORD_HEARTBEAT_EVENT        0x0002
#define COORD_NETWORK_STATUS_EVENT   0x0004
#define COORD_PERMIT_JOIN_EVENT      0x0008
#define COORD_DATA_LOG_EVENT         0x0010

// Application constants
#define COORD_ENDPOINT               1
#define COORD_PROFID                 0x0104  // Home Automation
#define COORD_DEVICEID               0x0005  // Configuration Tool
#define COORD_DEVICE_VERSION         0
#define COORD_FLAGS                  0

#define SENSOR_CLUSTER_ID            0x0001  // Raw sensor data from End Devices
#define COORD_DATA_CLUSTER_ID        0x0002  // Aggregated data from Router
#define HEARTBEAT_INTERVAL           30000   // 30 seconds heartbeat
#define PERMIT_JOIN_INTERVAL         60000   // 1 minute permit join
#define NETWORK_STATUS_INTERVAL      120000  // 2 minutes network status
#define DATA_LOG_INTERVAL            10000   // 10 seconds data logging

// Data structures
typedef struct {
    float temperature;
    float humidity;
    uint16 lux;
    float dustDensity;
    uint16 srcAddr;
    uint8 rssi;
    uint32 timestamp;
    uint8 dataType;  // 0=raw from End Device, 1=aggregated from Router
} CoordSensorData_t;

typedef struct {
    uint8 deviceCount;
    float avgTemperature;
    float avgHumidity;
    uint16 avgLux;
    float avgDustDensity;
    uint8 minRssi;
    uint8 maxRssi;
    uint32 timestamp;
    uint16 routerAddr;
} CoordAggregatedData_t;

typedef struct {
    uint16 shortAddr;
    uint8 ieee[8];
    uint8 deviceType;  // 0=End Device, 1=Router
    uint32 lastSeen;
    uint8 rssi;
    uint16 dataCount;
} NetworkDevice_t;

#define MAX_NETWORK_DEVICES 20
static NetworkDevice_t networkDevices[MAX_NETWORK_DEVICES];
static uint8 networkDeviceCount = 0;
static uint32 totalDataReceived = 0;

// Function prototypes
static void CoordinatorApp_Init(uint8 task_id);
static uint16 CoordinatorApp_ProcessEvent(uint8 task_id, uint16 events);
static void CoordinatorApp_InitDevice(void);
static void CoordinatorApp_ProcessIncomingData(afIncomingMSGPacket_t *pkt);
static void CoordinatorApp_ProcessRawSensorData(afIncomingMSGPacket_t *pkt);
static void CoordinatorApp_ProcessAggregatedData(afIncomingMSGPacket_t *pkt);
static void CoordinatorApp_SendHeartbeat(void);
static void CoordinatorApp_PermitJoin(void);
static void CoordinatorApp_NetworkStatus(void);
static void CoordinatorApp_LogData(void);
static void CoordinatorApp_ProcessZDOMsgs(zdoIncomingMsg_t *inMsg);
static void CoordinatorApp_ProcessZCLMsgs(zclIncomingMsg_t *pInMsg);
static void CoordinatorApp_UpdateNetworkDevice(uint16 srcAddr, uint8 deviceType, uint8 rssi);
static void CoordinatorApp_PrintNetworkDevices(void);
static void CoordinatorApp_ForwardToUART(uint8 *data, uint8 len);

// ZCL cluster list
const cId_t CoordinatorApp_InClusterList[] = {
    ZCL_CLUSTER_ID_GEN_BASIC,
    ZCL_CLUSTER_ID_GEN_IDENTIFY,
    SENSOR_CLUSTER_ID,
    COORD_DATA_CLUSTER_ID
};

const cId_t CoordinatorApp_OutClusterList[] = {
    ZCL_CLUSTER_ID_GEN_BASIC,
    ZCL_CLUSTER_ID_GEN_IDENTIFY
};

// Simple descriptor
SimpleDescriptionFormat_t CoordinatorApp_SimpleDesc = {
    COORD_ENDPOINT,
    COORD_PROFID,
    COORD_DEVICEID,
    COORD_DEVICE_VERSION,
    COORD_FLAGS,
    sizeof(CoordinatorApp_InClusterList) / sizeof(CoordinatorApp_InClusterList[0]),
    (cId_t *)CoordinatorApp_InClusterList,
    sizeof(CoordinatorApp_OutClusterList) / sizeof(CoordinatorApp_OutClusterList[0]),
    (cId_t *)CoordinatorApp_OutClusterList
};

// Task initialization table
const pTaskEventHandlerFn tasksArr[] = {
    macEventLoop,
    nwk_event_loop,
    Hal_ProcessEvent,
    APS_event_loop,
    ZDApp_event_loop,
    zcl_event_loop,
    bdb_event_loop,
    gp_event_loop,
    CoordinatorApp_ProcessEvent
};

const uint8 tasksCnt = sizeof(tasksArr) / sizeof(tasksArr[0]);
uint16 *tasksEvents;

void osalInitTasks(void) {
    uint8 taskID = 0;

    tasksEvents = (uint16 *)osal_mem_alloc(sizeof(uint16) * tasksCnt);
    osal_memset(tasksEvents, 0, (sizeof(uint16) * tasksCnt));

    macTaskInit(taskID++);
    nwk_init(taskID++);
    Hal_Init(taskID++);
    APS_Init(taskID++);
    ZDApp_Init(taskID++);
    zcl_Init(taskID++);
    bdb_Init(taskID++);
    gp_Init(taskID++);
    
    CoordinatorApp_TaskID = taskID;
    CoordinatorApp_Init(taskID);
}

static void CoordinatorApp_Init(uint8 task_id) {
    CoordinatorApp_TaskID = task_id;
    
    // Initialize network device list
    osal_memset(networkDevices, 0, sizeof(networkDevices));
    networkDeviceCount = 0;
    totalDataReceived = 0;
    
    // Initialize device
    CoordinatorApp_InitDevice();
    
    // Register for ZDO callbacks
    ZDO_RegisterForZDOMsg(CoordinatorApp_TaskID, Device_annce);
    ZDO_RegisterForZDOMsg(CoordinatorApp_TaskID, NWK_addr_rsp);
    ZDO_RegisterForZDOMsg(CoordinatorApp_TaskID, IEEE_addr_rsp);
    ZDO_RegisterForZDOMsg(CoordinatorApp_TaskID, Node_Desc_rsp);
    ZDO_RegisterForZDOMsg(CoordinatorApp_TaskID, Simple_Desc_rsp);
    ZDO_RegisterForZDOMsg(CoordinatorApp_TaskID, Active_EP_rsp);
    ZDO_RegisterForZDOMsg(CoordinatorApp_TaskID, Match_Desc_rsp);
    ZDO_RegisterForZDOMsg(CoordinatorApp_TaskID, End_Device_Bind_rsp);
    ZDO_RegisterForZDOMsg(CoordinatorApp_TaskID, Bind_rsp);
    ZDO_RegisterForZDOMsg(CoordinatorApp_TaskID, Unbind_rsp);
    ZDO_RegisterForZDOMsg(CoordinatorApp_TaskID, Mgmt_Leave_rsp);
    ZDO_RegisterForZDOMsg(CoordinatorApp_TaskID, Mgmt_Direct_Join_rsp);
    ZDO_RegisterForZDOMsg(CoordinatorApp_TaskID, Mgmt_Permit_Join_rsp);
    ZDO_RegisterForZDOMsg(CoordinatorApp_TaskID, Mgmt_NWK_Disc_rsp);
    ZDO_RegisterForZDOMsg(CoordinatorApp_TaskID, Mgmt_Lqi_rsp);
    ZDO_RegisterForZDOMsg(CoordinatorApp_TaskID, Mgmt_Rtg_rsp);
    ZDO_RegisterForZDOMsg(CoordinatorApp_TaskID, Mgmt_Bind_rsp);
    
    // Initialize BDB for Coordinator
    bdb_StartCommissioning(BDB_COMMISSIONING_MODE_NWK_FORMATION |
                          BDB_COMMISSIONING_MODE_NWK_STEERING |
                          BDB_COMMISSIONING_MODE_FINDING_BINDING);
    
    // Start periodic tasks
    osal_start_timerEx(CoordinatorApp_TaskID, COORD_HEARTBEAT_EVENT, HEARTBEAT_INTERVAL);
    osal_start_timerEx(CoordinatorApp_TaskID, COORD_PERMIT_JOIN_EVENT, PERMIT_JOIN_INTERVAL);
    osal_start_timerEx(CoordinatorApp_TaskID, COORD_NETWORK_STATUS_EVENT, NETWORK_STATUS_INTERVAL);
    osal_start_timerEx(CoordinatorApp_TaskID, COORD_DATA_LOG_EVENT, DATA_LOG_INTERVAL);
}

static uint16 CoordinatorApp_ProcessEvent(uint8 task_id, uint16 events) {
    afIncomingMSGPacket_t *MSGpkt;
    (void)task_id;
    
    if(events & SYS_EVENT_MSG) {
        MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive(CoordinatorApp_TaskID);
        while(MSGpkt) {
            switch(MSGpkt->hdr.event) {
                case AF_INCOMING_MSG_CMD:
                    CoordinatorApp_ProcessIncomingData(MSGpkt);
                    break;
                    
                case ZDO_CB_MSG:
                    CoordinatorApp_ProcessZDOMsgs((zdoIncomingMsg_t *)MSGpkt);
                    break;
                    
                case ZCL_INCOMING_MSG:
                    CoordinatorApp_ProcessZCLMsgs((zclIncomingMsg_t *)MSGpkt);
                    break;
                    
                default:
                    break;
            }
            
            osal_msg_deallocate((uint8 *)MSGpkt);
            MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive(CoordinatorApp_TaskID);
        }
        
        return (events ^ SYS_EVENT_MSG);
    }
    
    if(events & COORD_DATA_PROCESS_EVENT) {
        // Process queued data
        return (events ^ COORD_DATA_PROCESS_EVENT);
    }
    
    if(events & COORD_HEARTBEAT_EVENT) {
        CoordinatorApp_SendHeartbeat();
        
        // Schedule next heartbeat
        osal_start_timerEx(CoordinatorApp_TaskID, COORD_HEARTBEAT_EVENT, HEARTBEAT_INTERVAL);
        
        return (events ^ COORD_HEARTBEAT_EVENT);
    }
    
    if(events & COORD_PERMIT_JOIN_EVENT) {
        CoordinatorApp_PermitJoin();
        
        // Schedule next permit join
        osal_start_timerEx(CoordinatorApp_TaskID, COORD_PERMIT_JOIN_EVENT, PERMIT_JOIN_INTERVAL);
        
        return (events ^ COORD_PERMIT_JOIN_EVENT);
    }
    
    if(events & COORD_NETWORK_STATUS_EVENT) {
        CoordinatorApp_NetworkStatus();
        CoordinatorApp_PrintNetworkDevices();
        
        // Schedule next network status
        osal_start_timerEx(CoordinatorApp_TaskID, COORD_NETWORK_STATUS_EVENT, NETWORK_STATUS_INTERVAL);
        
        return (events ^ COORD_NETWORK_STATUS_EVENT);
    }
    
    if(events & COORD_DATA_LOG_EVENT) {
        CoordinatorApp_LogData();
        
        // Schedule next data log
        osal_start_timerEx(CoordinatorApp_TaskID, COORD_DATA_LOG_EVENT, DATA_LOG_INTERVAL);
        
        return (events ^ COORD_DATA_LOG_EVENT);
    }
    
    return 0;
}

static void CoordinatorApp_InitDevice(void) {
    halUARTCfg_t uartConfig;
    
    // Register endpoint
    afRegister(&CoordinatorApp_SimpleDesc);
    
    // Initialize UART for data output
    uartConfig.configured = TRUE;
    uartConfig.baudRate = HAL_UART_BR_115200;
    uartConfig.flowControl = FALSE;
    uartConfig.flowControlThreshold = 48;
    uartConfig.rx.maxBufSize = 256;
    uartConfig.tx.maxBufSize = 256;
    uartConfig.idleTimeout = 6;
    uartConfig.intEnable = TRUE;
    uartConfig.callBackFunc = NULL;
    
    HalUARTOpen(HAL_UART_PORT_0, &uartConfig);
    
    // Configure as Coordinator
    ZDO_Config_Node_Descriptor.LogicalType = ZG_DEVICETYPE_COORDINATOR;
    ZDO_Config_Node_Descriptor.ComplexDescriptorAvailable = FALSE;
    ZDO_Config_Node_Descriptor.UserDescriptorAvailable = FALSE;
    ZDO_Config_Node_Descriptor.Reserved = 0;
    ZDO_Config_Node_Descriptor.APSFlags = 0;
    ZDO_Config_Node_Descriptor.FrequencyBand = NODEFREQ_2400;
    ZDO_Config_Node_Descriptor.CapabilityFlags = CAPINFO_DEVICETYPE_FFD | CAPINFO_POWER_AC |
                                                CAPINFO_RCVR_ON_IDLE | CAPINFO_ALLOC_ADDR;
    ZDO_Config_Node_Descriptor.ManufacturerCode[0] = 0x00;
    ZDO_Config_Node_Descriptor.ManufacturerCode[1] = 0x00;
    ZDO_Config_Node_Descriptor.MaxBufferSize = MAX_BUFFER_SIZE;
    ZDO_Config_Node_Descriptor.MaxInTransferSize[0] = MAX_TRANSFER_SIZE;
    ZDO_Config_Node_Descriptor.MaxInTransferSize[1] = MAX_TRANSFER_SIZE >> 8;
    ZDO_Config_Node_Descriptor.ServerMask = 0x0000;
    ZDO_Config_Node_Descriptor.MaxOutTransferSize[0] = MAX_TRANSFER_SIZE;
    ZDO_Config_Node_Descriptor.MaxOutTransferSize[1] = MAX_TRANSFER_SIZE >> 8;
    ZDO_Config_Node_Descriptor.DescriptorCapability = 0x00;
    
    // Print startup message
    HalUARTWrite(HAL_UART_PORT_0, (uint8*)"=== Zigbee Coordinator Started ===\r\n", 38);
    HalUARTWrite(HAL_UART_PORT_0, (uint8*)"Waiting for network formation...\r\n", 34);
}

static void CoordinatorApp_ProcessIncomingData(afIncomingMSGPacket_t *pkt) {
    totalDataReceived++;
    
    if(pkt->clusterId == SENSOR_CLUSTER_ID) {
        // Raw sensor data from End Device or forwarded by Router
        CoordinatorApp_ProcessRawSensorData(pkt);
    }
    else if(pkt->clusterId == COORD_DATA_CLUSTER_ID) {
        // Aggregated data from Router
        CoordinatorApp_ProcessAggregatedData(pkt);
    }
    
    // Update network device info
    uint8 deviceType = (pkt->clusterId == COORD_DATA_CLUSTER_ID) ? 1 : 0; // 1=Router, 0=End Device
    CoordinatorApp_UpdateNetworkDevice(pkt->srcAddr.addr.shortAddr, deviceType, pkt->rssi);
    
    // Toggle LED to indicate data received
    HalLedSet(HAL_LED_1, HAL_LED_MODE_TOGGLE);
}

static void CoordinatorApp_ProcessRawSensorData(afIncomingMSGPacket_t *pkt) {
    CoordSensorData_t sensorData;
    uint8 *pData = pkt->cmd.Data;
    uint8 buffer[128];
    uint8 len;
    
    if(pkt->cmd.DataLength >= 11) {
        // Extract raw sensor data
        if(pkt->cmd.DataLength >= 16) {
            // Data forwarded by Router (includes source address)
            sensorData.srcAddr = BUILD_UINT16(pData[0], pData[1]);
            sensorData.temperature = *(float*)&pData[2];
            sensorData.humidity = *(float*)&pData[6];
            sensorData.lux = BUILD_UINT16(pData[10], pData[11]);
            sensorData.dustDensity = *(float*)&pData[12];
            sensorData.rssi = pData[16];
        } else {
            // Direct data from End Device
            sensorData.srcAddr = pkt->srcAddr.addr.shortAddr;
            sensorData.temperature = *(float*)&pData[0];
            sensorData.humidity = *(float*)&pData[4];
            sensorData.lux = BUILD_UINT16(pData[8], pData[9]);
            sensorData.dustDensity = *(float*)&pData[10];
            sensorData.rssi = pkt->rssi;
        }
        
        sensorData.timestamp = osal_GetSystemClock();
        sensorData.dataType = 0; // Raw data
        
        // Display on UART
        len = sprintf((char*)buffer, 
                      "RAW,0x%04X,%.1f,%.1f,%u,%.2f,%d,%lu\r\n",
                      sensorData.srcAddr, sensorData.temperature, sensorData.humidity,
                      sensorData.lux, sensorData.dustDensity, sensorData.rssi, 
                      sensorData.timestamp);
        
        HalUARTWrite(HAL_UART_PORT_0, buffer, len);
        
        // Also display in human readable format
        len = sprintf((char*)buffer,
                      "Device 0x%04X: T=%.1f°C, H=%.1f%%, L=%u lux, D=%.2f mg/m³, RSSI=%d\r\n",
                      sensorData.srcAddr, sensorData.temperature, sensorData.humidity,
                      sensorData.lux, sensorData.dustDensity, sensorData.rssi);
        
        HalUARTWrite(HAL_UART_PORT_0, buffer, len);
    }
}

static void CoordinatorApp_ProcessAggregatedData(afIncomingMSGPacket_t *pkt) {
    CoordAggregatedData_t aggData;
    uint8 *pData = pkt->cmd.Data;
    uint8 buffer[128];
    uint8 len;
    
    if(pkt->cmd.DataLength >= 11) {
        // Extract aggregated data from Router
        aggData.deviceCount = pData[0];
        aggData.avgTemperature = *(float*)&pData[1];
        aggData.avgHumidity = *(float*)&pData[5];
        aggData.avgLux = BUILD_UINT16(pData[9], pData[10]);
        aggData.avgDustDensity = *(float*)&pData[11];
        aggData.minRssi = pData[15];
        aggData.maxRssi = pData[16];
        aggData.routerAddr = pkt->srcAddr.addr.shortAddr;
        aggData.timestamp = osal_GetSystemClock();
        
        // Display on UART in CSV format
        len = sprintf((char*)buffer,
                      "AGG,0x%04X,%d,%.1f,%.1f,%u,%.2f,%d,%d,%lu\r\n",
                      aggData.routerAddr, aggData.deviceCount,
                      aggData.avgTemperature, aggData.avgHumidity, aggData.avgLux,
                      aggData.avgDustDensity, aggData.minRssi, aggData.maxRssi,
                      aggData.timestamp);
        
        HalUARTWrite(HAL_UART_PORT_0, buffer, len);
        
        // Also display in human readable format
        len = sprintf((char*)buffer,
                      "Router 0x%04X: %d devices, Avg T=%.1f°C, H=%.1f%%, L=%u, D=%.2f, RSSI=%d-%d\r\n",
                      aggData.routerAddr, aggData.deviceCount,
                      aggData.avgTemperature, aggData.avgHumidity, aggData.avgLux,
                      aggData.avgDustDensity, aggData.minRssi, aggData.maxRssi);
        
        HalUARTWrite(HAL_UART_PORT_0, buffer, len);
    }
}

static void CoordinatorApp_UpdateNetworkDevice(uint16 srcAddr, uint8 deviceType, uint8 rssi) {
    uint8 i;
    
    // Find existing device or add new one
    for(i = 0; i < networkDeviceCount; i++) {
        if(networkDevices[i].shortAddr == srcAddr) {
            // Update existing device
            networkDevices[i].lastSeen = osal_GetSystemClock();
            networkDevices[i].rssi = rssi;
            networkDevices[i].dataCount++;
            return;
        }
    }
    
    // Add new device if space available
    if(networkDeviceCount < MAX_NETWORK_DEVICES) {
        networkDevices[networkDeviceCount].shortAddr = srcAddr;
        networkDevices[networkDeviceCount].deviceType = deviceType;
        networkDevices[networkDeviceCount].lastSeen = osal_GetSystemClock();
        networkDevices[networkDeviceCount].rssi = rssi;
        networkDevices[networkDeviceCount].dataCount = 1;
        networkDeviceCount++;
    }
}

static void CoordinatorApp_SendHeartbeat(void) {
    uint8 buffer[64];
    uint8 len;
    
    len = sprintf((char*)buffer, 
                  "HEARTBEAT,%d,%lu,%lu\r\n", 
                  networkDeviceCount, totalDataReceived, osal_GetSystemClock());
    HalUARTWrite(HAL_UART_PORT_0, buffer, len);
    
    // Toggle LED to indicate heartbeat
    HalLedSet(HAL_LED_2, HAL_LED_MODE_TOGGLE);
}

static void CoordinatorApp_PermitJoin(void) {
    // Enable permit join for 60 seconds
    NLME_PermitJoiningRequest(60);
    
    uint8 buffer[32];
    uint8 len = sprintf((char*)buffer, "PERMIT_JOIN,60\r\n");
    HalUARTWrite(HAL_UART_PORT_0, buffer, len);
}

static void CoordinatorApp_NetworkStatus(void) {
    uint8 buffer[64];
    uint8 len;
    
    len = sprintf((char*)buffer,
                  "NETWORK,0x%04X,%d,%d\r\n",
                  _NIB.nwkPanId, _NIB.nwkLogicalChannel, networkDeviceCount);
    HalUARTWrite(HAL_UART_PORT_0, buffer, len);
}

static void CoordinatorApp_PrintNetworkDevices(void) {
    uint8 i;
    uint8 buffer[64];
    uint8 len;
    
    HalUARTWrite(HAL_UART_PORT_0, (uint8*)"=== Network Devices ===\r\n", 26);
    
    for(i = 0; i < networkDeviceCount; i++) {
        len = sprintf((char*)buffer,
                      "DEV,0x%04X,%s,%d,%u\r\n",
                      networkDevices[i].shortAddr,
                      (networkDevices[i].deviceType == 1) ? "Router" : "EndDev",
                      networkDevices[i].rssi,
                      networkDevices[i].dataCount);
        HalUARTWrite(HAL_UART_PORT_0, buffer, len);
    }
}

static void CoordinatorApp_LogData(void) {
    // Log summary data periodically
    uint8 buffer[64];
    uint8 len;
    
    len = sprintf((char*)buffer,
                  "LOG,%d,%lu,%lu\r\n",
                  networkDeviceCount, totalDataReceived, osal_GetSystemClock());
    HalUARTWrite(HAL_UART_PORT_0, buffer, len);
}

static void CoordinatorApp_ProcessZDOMsgs(zdoIncomingMsg_t *inMsg) {
    uint8 buffer[64];
    uint8 len;
    
    switch(inMsg->clusterID) {
        case Device_annce:
            len = sprintf((char*)buffer, 
                          "JOIN,0x%04X\r\n",
                          BUILD_UINT16(inMsg->asdu[0], inMsg->asdu[1]));
            HalUARTWrite(HAL_UART_PORT_0, buffer, len);
            break;
            
        case End_Device_Bind_rsp:
            if(ZDO_ParseBindRsp(inMsg) == ZSuccess) {
                HalUARTWrite(HAL_UART_PORT_0, (uint8*)"BIND_SUCCESS\r\n", 14);
            }
            break;
            
        default:
            break;
    }
}

static void CoordinatorApp_ProcessZCLMsgs(zclIncomingMsg_t *pInMsg) {
    switch(pInMsg->zclHdr.commandID) {
        case ZCL_CMD_READ:
            // Handle read commands
            break;
            
        case ZCL_CMD_WRITE:
            // Handle write commands
            break;
            
        default:
            break;
    }
}
