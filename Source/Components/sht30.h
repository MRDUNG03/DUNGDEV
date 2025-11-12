#include "hal_types.h"
#ifndef _SHT30_H_
#define _SHT30_H_

void SHT30_Init(void);
uint8 sht30_read_temp_humi(float *temperature, float *humidity);

#endif
