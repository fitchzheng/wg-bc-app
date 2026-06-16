#ifndef __BAT_CHARGE_PATTERN_H
#define __BAT_CHARGE_PATTERN_H

#include "stdint.h"

#define ACC_VEER_VOUL                             10.0f
#define ACC_EXIT_VEER_VOUL                        9.0f
#define RTM_VEER_VOUL                             10.0f
#define RTM_EXIT_VEER_VOUL                        9.0f




typedef struct
{
    float OutVolt;
    float OutCurr;
    float OutCurrPower;
    float InpVolt;
    float InpCurr;
    float InpCurrPower;
    float FullCurr;
    float ChargCurr;
    float ActualOutVolt;
    float ActualOutCurr;
} ModeParameters_T;



void GetChargeState(uint16_t ChargMode);
uint16_t get_check_state_data(void);
uint8_t gpio_get_pg_en(void);
#endif



