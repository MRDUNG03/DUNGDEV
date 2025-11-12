#ifndef COORDINATORAPP_H
#define COORDINATORAPP_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "ZComDef.h"

// Application events
#define COORD_DATA_PROCESS_EVENT     0x0001
#define COORD_HEARTBEAT_EVENT        0x0002
#define COORD_NETWORK_STATUS_EVENT   0x0004
#define COORD_PERMIT_JOIN_EVENT      0x0008
#define COORD_DATA_LOG_EVENT         0x0010

// Application task ID
extern uint8 CoordinatorApp_TaskID;

// Function prototypes
extern void CoordinatorApp_Init(uint8 task_id);
extern uint16 CoordinatorApp_ProcessEvent(uint8 task_id, uint16 events);

#ifdef __cplusplus
}
#endif

#endif /* COORDINATORAPP_H */
