#ifndef SENSORAPP_H
#define SENSORAPP_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "ZComDef.h"

// Application events
#define SENSOR_READ_EVENT         0x0001
#define SENSOR_SEND_DATA_EVENT    0x0002

// Application task ID
extern uint8 SensorApp_TaskID;

// Function prototypes
extern void SensorApp_Init(uint8 task_id);
extern uint16 SensorApp_ProcessEvent(uint8 task_id, uint16 events);

#ifdef __cplusplus
}
#endif

#endif /* SENSORAPP_H */
