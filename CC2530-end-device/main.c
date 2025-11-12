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
#include "SensorApp.h"
#include "sht30.h"
#include "ldr.h"
#include "GP2Y.h"

// Application Task ID
uint8 SensorApp_TaskID;

// Application events
#define SENSOR_READ_EVENT        0x0001
#define SENSOR_SEND_EVENT        0x0002
#define SENSOR_REJOIN_EVENT      0x0004

// Application constants
#define SENSOR_ENDPOINT          1
#define SENSOR_PROFID            0x0104  // Home Automation
#define SENSOR_DEVICEID          0x0302  // Temperature Sensor
#define SENSOR_DEVICE_VERSION    0
#define SENSOR_FLAGS             0

#define SENSOR_CLUSTER_ID        0x0001  // Custom cluster for sensor data
#define SENSOR_READ_INTERVAL     300000  // 5 minutes

// Sensor data structure
typedef struct {
    float temperature;
    float humidity;
    uint16 lux;
    float dustDensity;
} SensorData_t;

static SensorData_t sensorData;

// Function prototypes
static void SensorApp_Init(uint8 task_id);
static uint16 SensorApp_ProcessEvent(uint8 task_id, uint16 events);
static void SensorApp_InitDevice(void);
static void SensorApp_ReadSensors(void);
static void SensorApp_SendData(void);
static void SensorApp_ProcessZDOMsgs(zdoIncomingMsg_t *inMsg);
static void SensorApp_ProcessZCLMsgs(zclIncomingMsg_t *pInMsg);

// ZCL cluster list
const cId_t SensorApp_InClusterList[] = {
    ZCL_CLUSTER_ID_GEN_BASIC,
    ZCL_CLUSTER_ID_GEN_IDENTIFY
};

const cId_t SensorApp_OutClusterList[] = {
    SENSOR_CLUSTER_ID
};

// Simple descriptor
SimpleDescriptionFormat_t SensorApp_SimpleDesc = {
    SENSOR_ENDPOINT,
    SENSOR_PROFID,
    SENSOR_DEVICEID,
    SENSOR_DEVICE_VERSION,
    SENSOR_FLAGS,
    sizeof(SensorApp_InClusterList) / sizeof(SensorApp_InClusterList[0]),
    (cId_t *)SensorApp_InClusterList,
    sizeof(SensorApp_OutClusterList) / sizeof(SensorApp_OutClusterList[0]),
    (cId_t *)SensorApp_OutClusterList
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
    SensorApp_ProcessEvent
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
    
    SensorApp_TaskID = taskID;
    SensorApp_Init(taskID);
}

static void SensorApp_Init(uint8 task_id) {
    SensorApp_TaskID = task_id;
    
    // Initialize device
    SensorApp_InitDevice();
    
    // Register for ZDO callbacks
    ZDO_RegisterForZDOMsg(SensorApp_TaskID, End_Device_Bind_rsp);
    ZDO_RegisterForZDOMsg(SensorApp_TaskID, Match_Desc_rsp);
    ZDO_RegisterForZDOMsg(SensorApp_TaskID, Device_annce);
    
    // Initialize BDB
    bdb_StartCommissioning(BDB_COMMISSIONING_MODE_NWK_STEERING |
                          BDB_COMMISSIONING_MODE_FINDING_BINDING);
    
    // Enable power management
    osal_pwrmgr_device(PWRMGR_BATTERY);
    
    // Start sensor reading timer
    osal_start_timerEx(SensorApp_TaskID, SENSOR_READ_EVENT, 10000); // First read after 10s
}

static uint16 SensorApp_ProcessEvent(uint8 task_id, uint16 events) {
    afIncomingMSGPacket_t *MSGpkt;
    (void)task_id;
    
    if(events & SYS_EVENT_MSG) {
        MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive(SensorApp_TaskID);
        while(MSGpkt) {
            switch(MSGpkt->hdr.event) {
                case ZDO_CB_MSG:
                    SensorApp_ProcessZDOMsgs((zdoIncomingMsg_t *)MSGpkt);
                    break;
                    
                case ZCL_INCOMING_MSG:
                    SensorApp_ProcessZCLMsgs((zclIncomingMsg_t *)MSGpkt);
                    break;
                    
                default:
                    break;
            }
            
            osal_msg_deallocate((uint8 *)MSGpkt);
            MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive(SensorApp_TaskID);
        }
        
        return (events ^ SYS_EVENT_MSG);
    }
    
    if(events & SENSOR_READ_EVENT) {
        // Wake up sensors for reading
        osal_pwrmgr_task_state(SensorApp_TaskID, PWRMGR_HOLD);
        
        SensorApp_ReadSensors();
        osal_set_event(SensorApp_TaskID, SENSOR_SEND_EVENT);
        
        // Schedule next reading
        osal_start_timerEx(SensorApp_TaskID, SENSOR_READ_EVENT, SENSOR_READ_INTERVAL);
        
        return (events ^ SENSOR_READ_EVENT);
    }
    
    if(events & SENSOR_SEND_EVENT) {
        SensorApp_SendData();
        
        // Release power management hold after sending data
        osal_pwrmgr_task_state(SensorApp_TaskID, PWRMGR_CONSERVE);
        
        return (events ^ SENSOR_SEND_EVENT);
    }
    
    if(events & SENSOR_REJOIN_EVENT) {
        // Rejoin network if needed
        bdb_StartCommissioning(BDB_COMMISSIONING_MODE_NWK_STEERING);
        return (events ^ SENSOR_REJOIN_EVENT);
    }
    
    return 0;
}

static void SensorApp_InitDevice(void) {
    // Register endpoint
    afRegister(&SensorApp_SimpleDesc);
    
    // Initialize sensors
    SHT30_Init();
    LDR_Init();
    GP2Y_Init();
    
    // Configure as End Device
    ZDO_Config_Node_Descriptor.LogicalType = ZG_DEVICETYPE_ENDDEVICE;
    ZDO_Config_Node_Descriptor.Reserved = 0;
    ZDO_Config_Node_Descriptor.APSFlags = 0;
    ZDO_Config_Node_Descriptor.FrequencyBand = NODEFREQ_2400;
    ZDO_Config_Node_Descriptor.CapabilityFlags = 0;
    ZDO_Config_Node_Descriptor.ManufacturerCode[0] = 0x00;
    ZDO_Config_Node_Descriptor.ManufacturerCode[1] = 0x00;
    ZDO_Config_Node_Descriptor.MaxBufferSize = MAX_BUFFER_SIZE;
    ZDO_Config_Node_Descriptor.MaxInTransferSize[0] = MAX_TRANSFER_SIZE;
    ZDO_Config_Node_Descriptor.MaxInTransferSize[1] = MAX_TRANSFER_SIZE >> 8;
    ZDO_Config_Node_Descriptor.ServerMask = 0x0000;
    ZDO_Config_Node_Descriptor.MaxOutTransferSize[0] = MAX_TRANSFER_SIZE;
    ZDO_Config_Node_Descriptor.MaxOutTransferSize[1] = MAX_TRANSFER_SIZE >> 8;
    ZDO_Config_Node_Descriptor.DescriptorCapability = 0x00;
}

static void SensorApp_ReadSensors(void) {
    // Power up sensors briefly for reading
    SHT30_Init();
    LDR_Init();
    GP2Y_Init();
    
    // Brief delay for sensor stabilization
    osal_start_timerEx(SensorApp_TaskID, SENSOR_SEND_EVENT, 100);
    
    // Read SHT30 temperature and humidity
    SHT30_ReadData(&sensorData.temperature, &sensorData.humidity);
    
    // Read LDR light sensor
    sensorData.lux = LDR_ReadLux();
    
    // Read GP2Y dust sensor
    sensorData.dustDensity = GP2Y_ReadDustDensity();
    
    // Toggle LED briefly to indicate reading
    HalLedSet(HAL_LED_1, HAL_LED_MODE_FLASH);
}

static void SensorApp_SendData(void) {
    uint8 buffer[16];
    uint8 *pBuf = buffer;
    
    // Pack sensor data into buffer
    *pBuf++ = LO_UINT16(*(uint16*)&sensorData.temperature);
    *pBuf++ = HI_UINT16(*(uint16*)&sensorData.temperature);
    *pBuf++ = LO_UINT16(*(uint16*)&sensorData.humidity);
    *pBuf++ = HI_UINT16(*(uint16*)&sensorData.humidity);
    *pBuf++ = LO_UINT16(sensorData.lux);
    *pBuf++ = HI_UINT16(sensorData.lux);
    *pBuf++ = LO_UINT16(*(uint16*)&sensorData.dustDensity);
    *pBuf++ = HI_UINT16(*(uint16*)&sensorData.dustDensity);
    
    // Send data to coordinator/router using AF_DISCV_ROUTE for automatic routing
    if(AF_DataRequest(&SensorApp_DstAddr, (endPointDesc_t*)&SensorApp_SimpleDesc,
                      SENSOR_CLUSTER_ID, (pBuf - buffer), buffer,
                      &SensorApp_TaskID, 0, AF_DISCV_ROUTE | AF_ACK_REQUEST) == afStatus_SUCCESS) {
        HalLedSet(HAL_LED_2, HAL_LED_MODE_BLINK);
    }
}

static void SensorApp_ProcessZDOMsgs(zdoIncomingMsg_t *inMsg) {
    switch(inMsg->clusterID) {
        case End_Device_Bind_rsp:
            if(ZDO_ParseBindRsp(inMsg) == ZSuccess) {
                // Binding successful
            }
            break;
            
        case Match_Desc_rsp:
            // Process match descriptor response
            break;
            
        case Device_annce:
            // Device announcement
            break;
            
        default:
            break;
    }
}

static void SensorApp_ProcessZCLMsgs(zclIncomingMsg_t *pInMsg) {
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

// Destination address for sending data
afAddrType_t SensorApp_DstAddr = {
    .addrMode = (afAddrMode_t)Addr16Bit,
    .addr.shortAddr = 0x0000,  // Send to Coordinator
    .endPoint = SENSOR_ENDPOINT
};
