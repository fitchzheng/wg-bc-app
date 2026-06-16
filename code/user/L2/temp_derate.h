#ifndef __TEMP_DERATE_H
#define __TEMP_DERATE_H

#include "stdint.h"

typedef struct
{
    float temp;                         // 当前温度
    float set_temp;                     // 设置温度
    float current;
    float temp_current;
    uint8_t enable;
} temp_derate_item_t;

void get_temp_derate_curr(void);

#endif

