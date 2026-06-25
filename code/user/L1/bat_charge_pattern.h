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
uint8_t bat_charge_acc_reverse_request_active(void);
uint8_t bat_charge_consume_acc_reverse_enter_event(void);
uint8_t bat_charge_reverse_timeout_lock_active(void);
void bat_charge_set_reverse_timeout_lock(uint8_t lock);
#endif



